/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdio.h>

#include <sys/cdefs.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <linux/fb.h>
#include <linux/kd.h>

#include "minui.h"
#include "graphics.h"

static gr_surface fbdev_init(minui_backend*);
static gr_surface fbdev_flip(minui_backend*);
static void fbdev_blank(minui_backend*, bool);
static void fbdev_exit(minui_backend*);

static GRSurface gr_framebuffer[2];
static bool double_buffered;
static GRSurface* gr_draw = NULL;
static GRSurface* gr_draw_temp = NULL;
static GRSurface gr_draw_struct;
static int displayed_buffer;

static struct fb_var_screeninfo vi;
static int fb_fd = -1;

static minui_backend my_backend = {
    .init = fbdev_init,
    .flip = fbdev_flip,
    .blank = fbdev_blank,
    .exit = fbdev_exit,
};

minui_backend* open_fbdev() {
    return &my_backend;
}

static void fbdev_blank(minui_backend* backend __unused, bool blank)
{
    int ret;

    ret = ioctl(fb_fd, FBIOBLANK, blank ? FB_BLANK_POWERDOWN : FB_BLANK_UNBLANK);
    if (ret < 0)
        perror("ioctl(): blank");
}

static void set_displayed_framebuffer(unsigned n)
{
    if (n > 1 || !double_buffered) return;

    vi.yres_virtual = gr_framebuffer[0].height * 2;
    vi.yoffset = n * gr_framebuffer[0].height;
    vi.bits_per_pixel = gr_framebuffer[0].pixel_bytes * 8;
    if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vi) < 0) {
        perror("active fb swap failed");
    }

	if (ioctl(fb_fd, FBIOPAN_DISPLAY, &vi) < 0) {
        perror("pan display failed");
    }
	
    displayed_buffer = n;
}

static gr_surface fbdev_init(minui_backend* backend) {
    int fd;
    void *bits;

    struct fb_fix_screeninfo fi;

    fd = open("/dev/graphics/fb0", O_RDWR);
    if (fd < 0) {
        perror("cannot open fb0");
        return NULL;
    }

    if (ioctl(fd, FBIOGET_FSCREENINFO, &fi) < 0) {
        perror("failed to get fb0 info");
        close(fd);
        return NULL;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) {
        perror("failed to get fb0 info");
        close(fd);
        return NULL;
    }

    // We print this out for informational purposes only, but
    // throughout we assume that the framebuffer device uses an RGBX
    // pixel format.  This is the case for every development device I
    // have access to.  For some of those devices (eg, hammerhead aka
    // Nexus 5), FBIOGET_VSCREENINFO *reports* that it wants a
    // different format (XBGR) but actually produces the correct
    // results on the display when you write RGBX.
    //
    // If you have a device that actually *needs* another pixel format
    // (ie, BGRX, or 565), patches welcome...

    printf("fb0 reports (possibly inaccurate):\n"
           "  vi.bits_per_pixel = %d\n"
           "  vi.red.offset   = %3d   .length = %3d\n"
           "  vi.green.offset = %3d   .length = %3d\n"
           "  vi.blue.offset  = %3d   .length = %3d\n"
		   "  fi.line_length = %d\n",
           vi.bits_per_pixel,
           vi.red.offset, vi.red.length,
           vi.green.offset, vi.green.length,
           vi.blue.offset, vi.blue.length,
		   fi.line_length);
		   
	//GGL_PIXEL_FORMAT_RGBX_8888
	vi.red.offset     = 0;
    vi.red.length     = 8;
    vi.green.offset   = 8;
    vi.green.length   = 8;
    vi.blue.offset    = 16;
    vi.blue.length    = 8;
    vi.transp.offset  = 24;
    vi.transp.length  = 8;
	vi.bits_per_pixel = 32;
	vi.nonstd = 2;

    bits = mmap(0, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bits == MAP_FAILED) {
        perror("failed to mmap framebuffer");
        close(fd);
        return NULL;
    }

    memset(bits, 0, fi.smem_len);


    gr_framebuffer[0].width = vi.xres;
    gr_framebuffer[0].height = vi.yres;
    gr_framebuffer[0].row_bytes = vi.xres * 4;
    gr_framebuffer[0].pixel_bytes = vi.bits_per_pixel / 8;
    gr_framebuffer[0].data = bits;
    memset(gr_framebuffer[0].data, 0, gr_framebuffer[0].height * gr_framebuffer[0].row_bytes);

    /* check if we can use double buffering */
	//if(0) {
    if (vi.yres * fi.line_length * 2 <= fi.smem_len) {
        double_buffered = true;

        memcpy(gr_framebuffer+1, gr_framebuffer, sizeof(GRSurface));
        gr_framebuffer[1].data = gr_framebuffer[0].data +
            gr_framebuffer[0].height * gr_framebuffer[0].row_bytes;
#if !defined(RECOVERY_ROTATION_270) && !defined(RECOVERY_ROTATION_180) && !defined(RECOVERY_ROTATION_90)

        gr_draw = gr_framebuffer+1;
#else
        gr_draw = &gr_draw_struct;
		memcpy(gr_draw, gr_framebuffer, sizeof(GRSurface));
		#if defined(RECOVERY_ROTATION_270) || defined(RECOVERY_ROTATION_90)
		gr_draw->width = gr_framebuffer[0].height;
		gr_draw->height = gr_framebuffer[0].width;
		gr_draw->row_bytes = gr_draw->width  * 4;
		#endif
		
		gr_draw_temp = (GRSurface*) malloc(sizeof(GRSurface));
		memcpy(gr_draw_temp, gr_draw, sizeof(GRSurface));
		gr_draw_temp->data = (unsigned char*) malloc(gr_draw_temp->height * gr_draw_temp->row_bytes);
        if (!gr_draw_temp->data) {
            perror("failed to allocate in-memory surface");
            return NULL;
        }
		memset(gr_draw_temp->data, 0x0, gr_draw_temp->height * gr_draw_temp->row_bytes);
#endif

    } else {
        double_buffered = false;

        // Without double-buffering, we allocate RAM for a buffer to
        // draw in, and then "flipping" the buffer consists of a
        // memcpy from the buffer we allocated to the framebuffer.

        gr_draw = (GRSurface*) malloc(sizeof(GRSurface));
        memcpy(gr_draw, gr_framebuffer, sizeof(GRSurface));
	#if defined(RECOVERY_ROTATION_270) || defined(RECOVERY_ROTATION_90)
		gr_draw->width = vi.yres;
    	gr_draw->height = vi.xres;
    	gr_draw->row_bytes = vi.yres * 4;
	#endif
        gr_draw->data = (unsigned char*) malloc(gr_draw->height * gr_draw->row_bytes);
        if (!gr_draw->data) {
            perror("failed to allocate in-memory surface");
            return NULL;
        }
    }

    memset(gr_draw->data, 0, gr_draw->height * gr_draw->row_bytes);
    fb_fd = fd;
    set_displayed_framebuffer(0);

    printf("framebuffer: %d (%d x %d) double_buffer %d\n", fb_fd, gr_draw->width, gr_draw->height, double_buffered);

    fbdev_blank(backend, true);
    fbdev_blank(backend, false);

    return gr_draw;
}


void rk_rotate_surface_270(GRSurface* surface_to, GRSurface* surface_from) {
    int width = surface_to->width;
    int height = surface_to->height;
#if (defined RECOVERY_RGBX || defined RECOVERY_BGRA)
	int byt = 4;
#else
    int byt = 2; // 2 byte for RGB_565 
#endif
	//printf("%s FROM width=%d height=%d byte=%d \n", __FUNCTION__, surface_from->width,surface_from->height, byt);
	//printf("%s TO width=%d height=%d byte=%d \n", __FUNCTION__, surface_to->width,surface_to->height, byt);
    int x = 0,y = 0;
	for(y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
#if (defined RECOVERY_RGBX || defined RECOVERY_BGRA)
			surface_to->data[width*y*byt + x*byt] = surface_from->data[height*x*byt + (height-y)*byt];
			surface_to->data[width*y*byt + x*byt + 1] = surface_from->data[height*x*byt + (height-y)*byt + 1];
			surface_to->data[width*y*byt + x*byt + 2] = surface_from->data[height*x*byt + (height-y)*byt + 2];
			surface_to->data[width*y*byt + x*byt + 3] = surface_from->data[height*x*byt + (height-y)*byt + 3];
#else
			surface_to->data[width*y*byt + x*byt] = surface_from->data[height*x*byt + (height-y)*byt];
			surface_to->data[width*y*byt + x*byt + 1] = surface_from->data[height*x*byt + (height-y)*byt + 1];
#endif
		}
	}
}



void rk_rotate_surface_180(GRSurface* surface_to, GRSurface* surface_from) {
    int width = surface_to->width;
    int height = surface_to->height;
	int length = width * height;
#if (defined RECOVERY_RGBX || defined RECOVERY_BGRA)
	int byt = 4;
#else
    int byt = 2; // 2 byte for RGB_565 
#endif

	int i = 0;
    for (i=0; i<length; i++) {
#if (defined RECOVERY_RGBX || defined RECOVERY_BGRA)
        surface_to->data[i*byt] = surface_from->data[(length-i-1)*byt];
        surface_to->data[i*byt+1] = surface_from->data[(length-i-1)*byt+1];
        surface_to->data[i*byt+2] = surface_from->data[(length-i-1)*byt+2];
        surface_to->data[i*byt+3] = surface_from->data[(length-i-1)*byt+3];
#else
		surface_to->data[i*byt] = surface_from->data[(length-i-1)*byt];
        surface_to->data[i*byt+1] = surface_from->data[(length-i-1)*byt+1];
#endif
    }
}




void rk_rotate_surface_90(GRSurface* surface_to, GRSurface* surface_from) {
    int width = surface_to->width;
    int height = surface_to->height;
#if (defined RECOVERY_RGBX || defined RECOVERY_BGRA)
	int byt = 4;
#else
    int byt = 2; // 2 byte for RGB_565 
#endif

    int x = 0,y = 0;
	for(y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
#if (defined RECOVERY_RGBX || defined RECOVERY_BGRA)
			surface_to->data[width*y*byt + x*byt] = surface_from->data[height*(width-x)*byt + y*byt];
			surface_to->data[width*y*byt + x*byt + 1] = surface_from->data[height*(width-x)*byt + y*byt + 1];
			surface_to->data[width*y*byt + x*byt + 2] = surface_from->data[height*(width-x)*byt + y*byt + 2];
			surface_to->data[width*y*byt + x*byt + 3] = surface_from->data[height*(width-x)*byt + y*byt + 3];
#else
			surface_to->data[width*y*byt + x*byt] = surface_from->data[height*(width-x)*byt + y*byt];
			surface_to->data[width*y*byt + x*byt + 1] = surface_from->data[height*(width-x)*byt + y*byt + 1];
#endif
		}
	}
}


static gr_surface fbdev_flip(minui_backend* backend __unused) {
	//printf("fbdev_flip, double_buffered= %d, displayed_buffer= %d\n",double_buffered, displayed_buffer);
    if (double_buffered) {
#if defined(RECOVERY_BGRA)
        // In case of BGRA, do some byte swapping
        unsigned int idx;
        unsigned char tmp;
        unsigned char* ucfb_vaddr = (unsigned char*)gr_draw->data;
        for (idx = 0 ; idx < (gr_draw->height * gr_draw->row_bytes);
                idx += 4) {
            tmp = ucfb_vaddr[idx];
            ucfb_vaddr[idx    ] = ucfb_vaddr[idx + 2];
            ucfb_vaddr[idx + 2] = tmp;
        }
#endif
        // Change gr_draw to point to the buffer currently displayed,
        // then flip the driver so we're displaying the other buffer
        // instead.
#if !defined(RECOVERY_ROTATION_270) && !defined(RECOVERY_ROTATION_180) && !defined(RECOVERY_ROTATION_90)

        gr_draw = gr_framebuffer + displayed_buffer;
#else
		gr_draw->data = gr_framebuffer[displayed_buffer].data;
		//memset(gr_draw->data, 0x0, gr_draw->height * gr_draw->row_bytes);
		memcpy(gr_draw_temp->data, gr_framebuffer[1-displayed_buffer].data,gr_framebuffer[1-displayed_buffer].height * gr_framebuffer[1-displayed_buffer].row_bytes);
		#if defined(RECOVERY_ROTATION_270)
		rk_rotate_surface_270(&gr_framebuffer[1-displayed_buffer], gr_draw_temp);
		#elif defined(RECOVERY_ROTATION_180)
		rk_rotate_surface_180(&gr_framebuffer[1-displayed_buffer], gr_draw_temp);
		#elif defined(RECOVERY_ROTATION_90)
		rk_rotate_surface_90(&gr_framebuffer[1-displayed_buffer], gr_draw_temp);
		#endif
		
#endif
        set_displayed_framebuffer(1-displayed_buffer);
#if defined(RECOVERY_ROTATION_270) || defined(RECOVERY_ROTATION_180) || defined(RECOVERY_ROTATION_90)

		memset(gr_draw->data, 0x0, gr_draw->height * gr_draw->row_bytes);
#endif
    } else {
        // Copy from the in-memory surface to the framebuffer.

#if defined(RECOVERY_BGRA)
        unsigned int idx;
        unsigned char* ucfb_vaddr = (unsigned char*)gr_framebuffer[0].data;
        unsigned char* ucbuffer_vaddr = (unsigned char*)gr_draw->data;
        for (idx = 0 ; idx < (gr_draw->height * gr_draw->row_bytes); idx += 4) {
            ucfb_vaddr[idx    ] = ucbuffer_vaddr[idx + 2];
            ucfb_vaddr[idx + 1] = ucbuffer_vaddr[idx + 1];
            ucfb_vaddr[idx + 2] = ucbuffer_vaddr[idx    ];
            ucfb_vaddr[idx + 3] = ucbuffer_vaddr[idx + 3];
        }
#else
	#if defined(RECOVERY_ROTATION_270)
		rk_rotate_surface_270(&gr_framebuffer[0], gr_draw);
	#elif defined(RECOVERY_ROTATION_180)
		rk_rotate_surface_180(&gr_framebuffer[0], gr_draw);
	#elif defined(RECOVERY_ROTATION_90)
		rk_rotate_surface_90(&gr_framebuffer[0], gr_draw);
	#else
        memcpy(gr_framebuffer[0].data, gr_draw->data,
               gr_draw->height * gr_draw->row_bytes);
	#endif
#endif
    }
    return gr_draw;
}

static void fbdev_exit(minui_backend* backend __unused) {
    close(fb_fd);
    fb_fd = -1;

    if (!double_buffered && gr_draw) {
        free(gr_draw->data);
        free(gr_draw);
    }
    gr_draw = NULL;
#if defined(RECOVERY_ROTATION_270) || defined(RECOVERY_ROTATION_180) || defined(RECOVERY_ROTATION_90)

	if(double_buffered && gr_draw_temp)
	{
		free(gr_draw_temp->data);
        free(gr_draw_temp);
	}
	gr_draw_temp = NULL;
#endif
}
