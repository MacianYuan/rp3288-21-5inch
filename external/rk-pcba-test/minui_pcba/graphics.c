/*
 * Copyright (C) 2007 The Android Open Source Project
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
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <linux/fb.h>
#include <linux/kd.h>

#include <time.h>

#include "font_10x18.h"
#include <pixelflinger/pixelflinger.h>
#include "minui.h"
#include "graphics.h"

typedef struct {
    GRSurface* texture;
    int cwidth;
    int cheight;
} GRFont;

static GRFont* gr_font = NULL;
static minui_backend* gr_backend = NULL;

static int overscan_percent = OVERSCAN_PERCENT;
static int overscan_offset_x = 0;
static int overscan_offset_y = 0;

static int gr_vt_fd = -1;

static unsigned char gr_current_r = 255;
static unsigned char gr_current_g = 255;
static unsigned char gr_current_b = 255;
static unsigned char gr_current_a = 255;
static unsigned char rgb_555[2];
static unsigned char gr_current_r5 = 31;
static unsigned char gr_current_g5 = 63;
static unsigned char gr_current_b5 = 31;

static GRSurface* gr_draw = NULL;

static GGLContext *gr_context = 0;
GGLSurface gr_mem_surface;
static int gr_is_curr_clr_opaque = 0;

static bool outside(int x, int y)
{
    return x < 0 || x >= gr_draw->width || y < 0 || y >= gr_draw->height;
}

int gr_measure(const char *s)
{
    return gr_font->cwidth * strlen(s);
}

void gr_font_size(int *x, int *y)
{
    *x = gr_font->cwidth;
    *y = gr_font->cheight;
}

static void text_blend(unsigned char* src_p, int src_row_bytes,
                       unsigned char* dst_p, int dst_row_bytes,
                       int width, int height)
{
    int i, j;
    for (j = 0; j < height; ++j) {
        unsigned char* sx = src_p;
        unsigned char* px = dst_p;
        for (i = 0; i < width; ++i) {
            unsigned char a = *sx++;
            if (gr_current_a < 255) a = ((int)a * gr_current_a) / 255;
            if (a == 255) {
                *px++ = gr_current_r;
                *px++ = gr_current_g;
                *px++ = gr_current_b;
                px++;
            } else if (a > 0) {
                *px = (*px * (255-a) + gr_current_r * a) / 255;
                ++px;
                *px = (*px * (255-a) + gr_current_g * a) / 255;
                ++px;
                *px = (*px * (255-a) + gr_current_b * a) / 255;
                ++px;
                ++px;
            } else {
                px += 4;
            }
        }
        src_p += src_row_bytes;
        dst_p += dst_row_bytes;
    }
}


void gr_text(int x, int y, const char *s, int bold)
{
    GRFont *font = gr_font;
    unsigned off;

    if (!font->texture) return;
    if (gr_current_a == 0) return;

    bold = bold && (font->texture->height != font->cheight);

    x += overscan_offset_x;
    y += overscan_offset_y;

    while((off = *s++)) {
        off -= 32;
        if (outside(x, y) || outside(x+font->cwidth-1, y+font->cheight-1)) break;
        if (off < 96) {

            unsigned char* src_p = font->texture->data + (off * font->cwidth) +
                (bold ? font->cheight * font->texture->row_bytes : 0);
            unsigned char* dst_p = gr_draw->data + y*gr_draw->row_bytes + x*gr_draw->pixel_bytes;

            text_blend(src_p, font->texture->row_bytes,
                       dst_p, gr_draw->row_bytes,
                       font->cwidth, font->cheight);

        }
        x += font->cwidth;
    }
}

void gr_texticon(int x, int y, GRSurface* icon) {
    if (icon == NULL) return;

    if (icon->pixel_bytes != 1) {
        printf("gr_texticon: source has wrong format\n");
        return;
    }

    x += overscan_offset_x;
    y += overscan_offset_y;

    if (outside(x, y) || outside(x+icon->width-1, y+icon->height-1)) return;

    unsigned char* src_p = icon->data;
    unsigned char* dst_p = gr_draw->data + y*gr_draw->row_bytes + x*gr_draw->pixel_bytes;

    text_blend(src_p, icon->row_bytes,
               dst_p, gr_draw->row_bytes,
               icon->width, icon->height);
}

void gr_clip(int x, int y, int w, int h)
{
    GGLContext *gl = gr_context;
    gl->scissor(gl, x, y, w, h);
    gl->enable(gl, GGL_SCISSOR_TEST);
}

void gr_noclip()
{
    GGLContext *gl = gr_context;
    gl->scissor(gl, 0, 0, gr_fb_width(), gr_fb_height());
    gl->disable(gl, GGL_SCISSOR_TEST);
}

void gr_line(int x0, int y0, int x1, int y1, int width)
{
    GGLContext *gl = gr_context;

    if(gr_is_curr_clr_opaque)
        gl->disable(gl, GGL_BLEND);

    const int coords0[2] = { x0 << 4, y0 << 4 };
    const int coords1[2] = { x1 << 4, y1 << 4 };
    gl->linex(gl, coords0, coords1, width << 4);

    if(gr_is_curr_clr_opaque)
        gl->enable(gl, GGL_BLEND);
}

gr_surface gr_render_circle(int radius, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    int rx, ry;
    GGLSurface *surface;
    const int diameter = radius*2 + 1;
    const int radius_check = radius*radius + radius*0.8;
    const uint32_t px = (a << 24) | (b << 16) | (g << 8) | r;
    uint32_t *data;

    surface = (GGLSurface *)malloc(sizeof(GGLSurface));
    memset(surface, 0, sizeof(GGLSurface));

    data = (uint32_t *)malloc(diameter * diameter * 4);
    memset(data, 0, diameter * diameter * 4);

    surface->version = sizeof(surface);
    surface->width = diameter;
    surface->height = diameter;
    surface->stride = diameter;
    surface->data = (GGLubyte*)data;
    surface->format = GGL_PIXEL_FORMAT_RGBA_8888;

    for(ry = -radius; ry <= radius; ++ry)
        for(rx = -radius; rx <= radius; ++rx)
            if(rx*rx+ry*ry <= radius_check)
                *(data + diameter*(radius + ry) + (radius+rx)) = px;

    return (gr_surface)surface;
}

void gr_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    GGLContext *gl = gr_context;
    GGLint color[4];
#if defined(RECOVERY_ABGR) || defined(RECOVERY_BGRA)
    color[0] = ((b << 8) | r) + 1;
    color[1] = ((g << 8) | g) + 1;
    color[2] = ((r << 8) | b) + 1;
    color[3] = ((a << 8) | a) + 1;
#else
    color[0] = ((r << 8) | r) + 1;
    color[1] = ((g << 8) | g) + 1;
    color[2] = ((b << 8) | b) + 1;
    color[3] = ((a << 8) | a) + 1;
#endif
    gl->color4xv(gl, color);

    gr_is_curr_clr_opaque = (a == 255);
	
    gr_current_r = r;
    gr_current_g = g;
    gr_current_b = b;
    gr_current_a = a;
}

void gr_clear()
{
    if (gr_draw->pixel_bytes == 2) {
        gr_fill(0, 0, gr_fb_width(), gr_fb_height());
        return;
    }
    if (gr_current_r == gr_current_g &&
        gr_current_r == gr_current_b) {
        memset(gr_draw->data, gr_current_r, gr_draw->height * gr_draw->row_bytes);
    } else {
        int x, y;
        unsigned char* px = gr_draw->data;
        for (y = 0; y < gr_draw->height; ++y) {
            for (x = 0; x < gr_draw->width; ++x) {
                *px++ = gr_current_r;
                *px++ = gr_current_g;
                *px++ = gr_current_b;
                px++;
            }
            px += gr_draw->row_bytes - (gr_draw->width * gr_draw->pixel_bytes);
        }
    }
}

void gr_fill(int x, int y, int w, int h)
{
    GGLContext *gl = gr_context;

    //if(gr_is_curr_clr_opaque)
        gl->disable(gl, GGL_BLEND);

    gl->recti(gl, x, y, x + w, y + h);

    //if(gr_is_curr_clr_opaque)
        gl->enable(gl, GGL_BLEND);
}

void gr_blit(gr_surface source, int sx, int sy, int w, int h, int dx, int dy) {
    if (gr_context == NULL) {
        return;
    }

    GGLContext *gl = gr_context;
    GGLSurface *surface = (GGLSurface*)source;

    if(surface->format == GGL_PIXEL_FORMAT_RGBX_8888)
        gl->disable(gl, GGL_BLEND);

    gl->bindTexture(gl, surface);
    gl->texEnvi(gl, GGL_TEXTURE_ENV, GGL_TEXTURE_ENV_MODE, GGL_REPLACE);
    gl->texGeni(gl, GGL_S, GGL_TEXTURE_GEN_MODE, GGL_ONE_TO_ONE);
    gl->texGeni(gl, GGL_T, GGL_TEXTURE_GEN_MODE, GGL_ONE_TO_ONE);
    gl->enable(gl, GGL_TEXTURE_2D);
    gl->texCoord2i(gl, sx - dx, sy - dy);
    gl->recti(gl, dx, dy, dx + w, dy + h);
    gl->disable(gl, GGL_TEXTURE_2D);

    if(surface->format == GGL_PIXEL_FORMAT_RGBX_8888)
        gl->enable(gl, GGL_BLEND);
}

unsigned int gr_get_width(gr_surface surface) {
    if (surface == NULL) {
        return 0;
    }
    return ((GGLSurface*) surface)->width;
}

unsigned int gr_get_height(gr_surface surface) {
    if (surface == NULL) {
        return 0;
    }
    return ((GGLSurface*) surface)->height;
}

unsigned int gr_get_font_cwidth(void)
{
    if (gr_font->cwidth > 0)
	return gr_font->cwidth;
    else
	return font.cwidth;
}

static void gr_init_font(void)
{
    gr_font = calloc(sizeof(*gr_font), 1);

    int res = res_create_alpha_surface("font", &(gr_font->texture));
    if (res == 0) {
        // The font image should be a 96x2 array of character images.  The
        // columns are the printable ASCII characters 0x20 - 0x7f.  The
        // top row is regular text; the bottom row is bold.
        gr_font->cwidth = gr_font->texture->width / 96;
        gr_font->cheight = gr_font->texture->height / 2;
    } else {
        printf("failed to read font: res=%d\n", res);

        // fall back to the compiled-in font.
        gr_font->texture = malloc(sizeof(*gr_font->texture));
        gr_font->texture->width = font.width;
        gr_font->texture->height = font.height;
        gr_font->texture->row_bytes = font.width;
        gr_font->texture->pixel_bytes = 1;

        unsigned char* bits = malloc(font.width * font.height);
        gr_font->texture->data = (void*) bits;

        unsigned char data;
        unsigned char* in = font.rundata;
        while((data = *in++)) {
            memset(bits, (data & 0x80) ? 255 : 0, data & 0x7f);
            bits += (data & 0x7f);
        }

        gr_font->cwidth = font.cwidth;
        gr_font->cheight = font.cheight;
    }
}

#if 0
// Exercises many of the gr_*() functions; useful for testing.
static void gr_test() {
    GRSurface** images;
    int frames;
    int result = res_create_multi_surface("icon_installing", &frames, &images);
    if (result < 0) {
        printf("create surface %d\n", result);
        gr_exit();
        return;
    }

    time_t start = time(NULL);
    int x;
    for (x = 0; x <= 1200; ++x) {
        if (x < 400) {
            gr_color(0, 0, 0, 255);
        } else {
            gr_color(0, (x-400)%128, 0, 255);
        }
        gr_clear();

        gr_color(255, 0, 0, 255);
        gr_surface frame = images[x%frames];
        gr_blit(frame, 0, 0, frame->width, frame->height, x, 0);

        gr_color(255, 0, 0, 128);
        gr_fill(400, 150, 600, 350);

        gr_color(255, 255, 255, 255);
        gr_text(500, 225, "hello, world!", 0);
        gr_color(255, 255, 0, 128);
        gr_text(300+x, 275, "pack my box with five dozen liquor jugs", 1);

        gr_color(0, 0, 255, 128);
        gr_fill(gr_draw->width - 200 - x, 300, gr_draw->width - x, 500);

        gr_draw = gr_backend->flip(gr_backend);
    }
    printf("getting end time\n");
    time_t end = time(NULL);
    printf("got end time\n");
    printf("start %ld end %ld\n", (long)start, (long)end);
    if (end > start) {
        printf("%.2f fps\n", ((double)x) / (end-start));
    }
}
#endif

void gr_flip() {
    gr_draw = gr_backend->flip(gr_backend);
    // On double buffered back ends, when we flip, we need to tell
    // pixel flinger to draw to the other buffer
    gr_mem_surface.data = (GGLubyte*)gr_draw->data;
    gr_context->colorBuffer(gr_context, &gr_mem_surface);
}

static void get_memory_surface(GGLSurface* ms) {
    ms->version = sizeof(*ms);
    ms->width = gr_draw->width;
    ms->height = gr_draw->height;
    ms->stride = gr_draw->row_bytes / gr_draw->pixel_bytes;
    ms->data = (GGLubyte*)gr_draw->data;
    ms->format = gr_draw->format;
}

int gr_init(void)
{
    gr_init_font();

    gr_vt_fd = open("/dev/tty0", O_RDWR | O_SYNC);
    if (gr_vt_fd < 0) {
        // This is non-fatal; post-Cupcake kernels don't have tty0.
        perror("can't open /dev/tty0");
    } else if (ioctl(gr_vt_fd, KDSETMODE, (void*) KD_GRAPHICS)) {
        // However, if we do open tty0, we expect the ioctl to work.
        perror("failed KDSETMODE to KD_GRAPHICS on tty0");
        gr_exit();
        return -1;
    }

    gr_backend = open_adf();
    if (gr_backend) {
        gr_draw = gr_backend->init(gr_backend);
        if (!gr_draw) {
            gr_backend->exit(gr_backend);
        }
    }

    if (!gr_draw) {
        gr_backend = open_drm();
        gr_draw = gr_backend->init(gr_backend);
        if (!gr_draw) {
            gr_backend->exit(gr_backend);
        }
    }

    if (!gr_draw) {
        gr_backend = open_fbdev();
        gr_draw = gr_backend->init(gr_backend);
        if (gr_draw == NULL) {
            return -1;
        }
    }

    overscan_offset_x = gr_draw->width * overscan_percent / 100;
    overscan_offset_y = gr_draw->height * overscan_percent / 100;

    // Set up pixelflinger
    get_memory_surface(&gr_mem_surface);
    gglInit(&gr_context);
    GGLContext *gl = gr_context;
    gl->colorBuffer(gl, &gr_mem_surface);

    gl->activeTexture(gl, 0);
    gl->enable(gl, GGL_BLEND);
    gl->blendFunc(gl, GGL_SRC_ALPHA, GGL_ONE_MINUS_SRC_ALPHA);

    gr_flip();
    gr_flip();

    return 0;
}

void gr_exit(void)
{
    gr_backend->exit(gr_backend);

    ioctl(gr_vt_fd, KDSETMODE, (void*) KD_TEXT);
    close(gr_vt_fd);
    gr_vt_fd = -1;
}

int gr_fb_width(void)
{
    return gr_draw->width - 2*overscan_offset_x;
}

int gr_fb_height(void)
{
    return gr_draw->height - 2*overscan_offset_y;
}

void gr_fb_blank(bool blank)
{
    gr_backend->blank(gr_backend, blank);
}

int gr_get_surface(gr_surface* surface)
{
    GGLSurface* ms = (GGLSurface*)malloc(sizeof(GGLSurface));
    if (!ms)    return -1;

    // Allocate the data
    get_memory_surface(ms);
    ms->data = (GGLubyte*)malloc(ms->stride * ms->height * gr_draw->pixel_bytes);

    // Now, copy the data
    memcpy(ms->data, gr_mem_surface.data, gr_draw->width * gr_draw->height * gr_draw->pixel_bytes / 8);

    *surface = (gr_surface*) ms;
    return 0;
}

int gr_free_surface(gr_surface surface)
{
    if (!surface)
        return -1;

    GGLSurface* ms = (GGLSurface*) surface;
    free(ms->data);
    free(ms);
    return 0;
}

void gr_write_frame_to_file(int fd)
{
    write(fd, gr_mem_surface.data, gr_draw->width * gr_draw->height * gr_draw->pixel_bytes / 8);
}
