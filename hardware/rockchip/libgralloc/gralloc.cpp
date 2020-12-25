/*
 * Copyright (C) 2010-2011 Chia-I Wu <olvaffe@gmail.com>
 * Copyright (C) 2010-2011 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#define LOG_TAG "GRALLOC-MOD"

#include <cutils/log.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <errno.h>

#include <vector>
#include "gralloc_helper.h"
#include "gralloc_drm.h"
#include "gralloc_drm_priv.h"
#include "gralloc_drm_handle.h"
#include "mali_gralloc_formats.h"

#include <inttypes.h>

#include <utils/CallStack.h>

#if RK_DRM_GRALLOC
#include <cutils/atomic.h>
#endif

#define UNUSED(...) (void)(__VA_ARGS__)

/*
 * Initialize the DRM device object
 */
static int drm_init(struct drm_module_t *dmod)
{
	int err = 0;

	pthread_mutex_lock(&dmod->mutex);
	if (!dmod->drm) {
		dmod->drm = gralloc_drm_create();
		if (!dmod->drm)
			err = -EINVAL;
	}
	pthread_mutex_unlock(&dmod->mutex);

	return err;
}

static int drm_mod_perform(const struct gralloc_module_t *mod, int op, ...)
{
	struct drm_module_t *dmod = (struct drm_module_t *) mod;
	va_list args;
	int err;

	err = drm_init(dmod);
	if (err)
		return err;

	va_start(args, op);
	switch (op) {
	case static_cast<int>(GRALLOC_MODULE_PERFORM_GET_DRM_FD):
		{
			int *fd = va_arg(args, int *);
			*fd = gralloc_drm_get_fd(dmod->drm);
			err = 0;
		}
		break;
#ifdef USE_HWC2
	case static_cast<int>(GRALLOC_MODULE_PERFORM_GET_RK_ASHMEM):
		{
			buffer_handle_t hnd = va_arg(args, buffer_handle_t);
			struct rk_ashmem_t* rk_ashmem = va_arg(args, struct rk_ashmem_t*);

			if (rk_ashmem != NULL)
				err = gralloc_drm_handle_get_rk_ashmem(hnd,rk_ashmem);
			else
				err = -EINVAL;
		}
		break;
	case static_cast<int>(GRALLOC_MODULE_PERFORM_SET_RK_ASHMEM):
		{
			buffer_handle_t hnd = va_arg(args, buffer_handle_t);
			struct rk_ashmem_t* rk_ashmem = va_arg(args, struct rk_ashmem_t*);

			if (rk_ashmem != NULL)
				err = gralloc_drm_handle_set_rk_ashmem(hnd,rk_ashmem);
			else
				err = -EINVAL;
		}
		break;
#endif
	case GRALLOC_MODULE_PERFORM_GET_HADNLE_PHY_ADDR:
		{
			buffer_handle_t hnd = va_arg(args, buffer_handle_t);
			uint32_t *phy_addr = va_arg(args, uint32_t *);

			if (phy_addr != NULL)
				err = gralloc_drm_handle_get_phy_addr(hnd,phy_addr);
			else
				err = -EINVAL;
		}
	    break;
	case GRALLOC_MODULE_PERFORM_GET_HADNLE_PRIME_FD:
		{
			buffer_handle_t hnd = va_arg(args, buffer_handle_t);
			int *fd = va_arg(args, int *);

			if (fd !=  NULL)
				err = gralloc_drm_handle_get_prime_fd(hnd,fd);
			else
				err = -EINVAL;
		}
		break;
	case GRALLOC_MODULE_PERFORM_GET_HADNLE_ATTRIBUTES:
		{
			buffer_handle_t hnd = va_arg(args, buffer_handle_t);
			std::vector<int> *attrs = va_arg(args, std::vector<int> *);

			if (attrs != NULL)
				err = gralloc_drm_handle_get_attributes(hnd, (void*)attrs);
			else
				err = -EINVAL;
		}
		break;
     case GRALLOC_MODULE_PERFORM_GET_INTERNAL_FORMAT:
        {
            buffer_handle_t hnd = va_arg(args, buffer_handle_t);
            uint64_t *internal_format = va_arg(args, uint64_t *);

            if(internal_format != NULL)
                err = gralloc_drm_handle_get_internal_format(hnd, internal_format);
            else
                err = -EINVAL;
        }
        break;
    case GRALLOC_MODULE_PERFORM_GET_HADNLE_WIDTH:
        {
            buffer_handle_t hnd = va_arg(args, buffer_handle_t);
            int *width = va_arg(args, int *);

            if(width !=  NULL)
                err = gralloc_drm_handle_get_width(hnd, width);
            else
                err = -EINVAL;
        }
        break;
    case GRALLOC_MODULE_PERFORM_GET_HADNLE_HEIGHT:
        {
            buffer_handle_t hnd = va_arg(args, buffer_handle_t);
            int *height = va_arg(args, int *);

            if(height != NULL)
                err = gralloc_drm_handle_get_height(hnd, height);
            else
                err = -EINVAL;
        }
        break;
    case GRALLOC_MODULE_PERFORM_GET_HADNLE_STRIDE:
        {
            buffer_handle_t hnd = va_arg(args, buffer_handle_t);
            int *stride = va_arg(args, int *);

            if(stride !=  NULL)
                err = gralloc_drm_handle_get_stride(hnd, stride);
            else
                err = -EINVAL;
        }
        break;
    case GRALLOC_MODULE_PERFORM_GET_HADNLE_BYTE_STRIDE:
        {
            buffer_handle_t hnd = va_arg(args, buffer_handle_t);
            int *byte_stride = va_arg(args, int *);

            if(byte_stride != NULL)
                err = gralloc_drm_handle_get_byte_stride(hnd, byte_stride);
            else
                err = -EINVAL;
        }
        break;
    case GRALLOC_MODULE_PERFORM_GET_HADNLE_FORMAT:
        {
            buffer_handle_t hnd = va_arg(args, buffer_handle_t);
            int *format = va_arg(args, int *);

            if(format != NULL)
                err = gralloc_drm_handle_get_format(hnd, format);
            else
                err = -EINVAL;
        }
        break;
    case GRALLOC_MODULE_PERFORM_GET_HADNLE_SIZE:
        {
            buffer_handle_t hnd = va_arg(args, buffer_handle_t);
            int *size = va_arg(args, int *);

            if(size != NULL)
                err = gralloc_drm_handle_get_size(hnd, size);
            else
                err = -EINVAL;
        }
        break;
    case GRALLOC_MODULE_PERFORM_GET_USAGE:
        {
            buffer_handle_t hnd = va_arg(args, buffer_handle_t);
            int *usage = va_arg(args, int *);

            if(usage != NULL)
                err = gralloc_drm_handle_get_usage(hnd, usage);
            else
                err = -EINVAL;
        }
        break;
	default:
		err = -EINVAL;
		break;
	}
	va_end(args);

	return err;
}

static int drm_mod_register_buffer(const gralloc_module_t *mod,
		buffer_handle_t handle)
{
	struct drm_module_t *dmod = (struct drm_module_t *) mod;
	int err;

	err = drm_init(dmod);
	if (err)
		return err;

	return gralloc_drm_handle_register(handle, dmod->drm);
}

static int drm_mod_unregister_buffer(const gralloc_module_t *mod,
		buffer_handle_t handle)
{
       UNUSED(mod);

	return gralloc_drm_handle_unregister(handle);
}

static int drm_mod_lock(const gralloc_module_t *mod, buffer_handle_t handle,
		int usage, int x, int y, int w, int h, void **ptr)
{
	struct gralloc_drm_bo_t *bo;
	int err;

	UNUSED(mod);

	bo = gralloc_drm_bo_from_handle(handle);
	if (!bo)
		return -EINVAL;

	err = gralloc_drm_bo_lock(bo, usage, x, y, w, h, ptr);
	gralloc_drm_bo_decref(bo);

	return err;
}

static int drm_mod_lock_ycbcr(gralloc_module_t const* module,
				buffer_handle_t handle, int usage,
				int l, int t, int w, int h, android_ycbcr *ycbcr)
{
	struct gralloc_drm_bo_t *bo;
	int ret = 0;

	GRALLOC_UN_USED(module);
	GRALLOC_UN_USED(l);
	GRALLOC_UN_USED(t);
	GRALLOC_UN_USED(w);
	GRALLOC_UN_USED(h);

	bo = gralloc_drm_bo_from_handle(handle);
	if (!bo)
		return -EINVAL;

	struct gralloc_drm_handle_t* hnd = (struct gralloc_drm_handle_t*)handle;

	if (usage & (GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK)) {
		char *cpu_addr;
		int y_stride = hnd->byte_stride;
		/* Ensure height is aligned for subsampled chroma
		 *before calculating buffer parameters
		 */
		int adjusted_height = GRALLOC_ALIGN(hnd->height, 2);
		int y_size =  y_stride * adjusted_height;

		int u_offset = 0;
		int v_offset = 0;
		int c_stride = 0;
		int step = 0;

        uint64_t base_format = (hnd->internal_format) & MALI_GRALLOC_INTFMT_FMT_MASK;

		int ret = gralloc_drm_bo_lock(bo, hnd->usage,
				0, 0, hnd->width, hnd->height,(void **)&cpu_addr);

		switch (base_format)
		{
			// case GRALLOC_ARM_HAL_FORMAT_INDEXED_NV12:
			case MALI_GRALLOC_FORMAT_INTERNAL_NV12:
            case HAL_PIXEL_FORMAT_YCrCb_NV12:
			case HAL_PIXEL_FORMAT_YCbCr_420_888:
				c_stride = y_stride;
				/* Y plane, UV plane */
				u_offset = y_size;
				v_offset = y_size + 1;
				step = 2;
				break;

			// case GRALLOC_ARM_HAL_FORMAT_INDEXED_NV21:
            case MALI_GRALLOC_FORMAT_INTERNAL_NV21:
				c_stride = y_stride;
				/* Y plane, UV plane */
				v_offset = y_size;
				u_offset = y_size + 1;
				step = 2;
				break;

			case HAL_PIXEL_FORMAT_YV12:
			// case GRALLOC_ARM_HAL_FORMAT_INDEXED_YV12:
            // case MALI_GRALLOC_FORMAT_INTERNAL_YV12:
			{
				int c_size;

				/* Stride alignment set to 16 as the SW access flags
				 *were set
				 */
				c_stride = GRALLOC_ALIGN(hnd->byte_stride / 2, 16);
				c_size = c_stride * (adjusted_height / 2);
				/* Y plane, V plane, U plane */
				v_offset = y_size;
				u_offset = y_size + c_size;
				step = 1;
				break;
			}

			default:
				ALOGE("Can't lock buffer %p: wrong format %" PRIu64 "",
								hnd, hnd->internal_format);
				ret = -EINVAL;
				break;
		}

		if (!ret) {
			ycbcr->y = cpu_addr;
			ycbcr->cb = cpu_addr + u_offset;
			ycbcr->cr = cpu_addr + v_offset;
			ycbcr->ystride = y_stride;
			ycbcr->cstride = c_stride;
			ycbcr->chroma_step = step;
		}
	}

	gralloc_drm_bo_decref(bo);
	return ret;
}



static int drm_mod_unlock(const gralloc_module_t *mod, buffer_handle_t handle)
{
	struct drm_module_t *dmod = (struct drm_module_t *) mod;
	struct gralloc_drm_bo_t *bo;

	bo = gralloc_drm_bo_from_handle(handle);
	if (!bo)
		return -EINVAL;

	gralloc_drm_bo_unlock(bo);
	gralloc_drm_bo_decref(bo);

	return 0;
}

static int drm_mod_close_gpu0(struct hw_device_t *dev)
{
	struct drm_module_t *dmod = (struct drm_module_t *)dev->module;
	struct alloc_device_t *alloc = (struct alloc_device_t *) dev;

#if RK_DRM_GRALLOC
	android_atomic_dec(&dmod->refcount);

       if(!dmod->refcount && dmod->drm)

       {
              gralloc_drm_destroy(dmod->drm);
              dmod->drm = NULL;
       }
#else
       gralloc_drm_destroy(dmod->drm);
#endif
	delete alloc;

	return 0;
}

static int drm_mod_free_gpu0(alloc_device_t *dev, buffer_handle_t handle)
{
	struct drm_module_t *dmod = (struct drm_module_t *) dev->common.module;
	struct gralloc_drm_bo_t *bo;

	return gralloc_drm_free_bo_from_handle(handle);
}

static int drm_mod_alloc_gpu0(alloc_device_t *dev,
		int w, int h, int format, int usage,
		buffer_handle_t *handle, int *stride)
{
	struct drm_module_t *dmod = (struct drm_module_t *) dev->common.module;
	struct gralloc_drm_bo_t *bo;
	int size, bpp, err;

#if RK_DRM_GRALLOC
    if(format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        if(usage  & GRALLOC_USAGE_HW_VIDEO_ENCODER)
            bpp = 1;    //HAL_PIXEL_FORMAT_YCrCb_NV12
        else
            bpp = 4;    //HAL_PIXEL_FORMAT_RGBX_8888
    }
    else
#endif
    {
	    bpp = gralloc_drm_get_bpp(format);
    }

	if (!bpp)
	{
#if RK_DRM_GRALLOC
		ALOGE("Cann't get valid bpp for format(0x%x)", format);
#endif
		return -EINVAL;
	}

	bo = gralloc_drm_bo_create(dmod->drm, w, h, format, usage);
	if (!bo)
		return -ENOMEM;

	*handle = gralloc_drm_bo_get_handle(bo, stride);
	//It is no need this operation since stide is already in pixels.
	/* in pixels */
	*stride /= bpp;

	return 0;
}

static int drm_mod_open_gpu0(struct drm_module_t *dmod, hw_device_t **dev)
{
	struct alloc_device_t *alloc;
	int err;

#if RK_DRM_GRALLOC
	android_atomic_inc(&dmod->refcount);
#endif

	err = drm_init(dmod);
	if (err)
		return err;

	alloc = new alloc_device_t;
	if (!alloc)
		return -EINVAL;

	alloc->common.tag = HARDWARE_DEVICE_TAG;
	alloc->common.version = 0;
	alloc->common.module = &dmod->base.common;
	alloc->common.close = drm_mod_close_gpu0;

	alloc->alloc = drm_mod_alloc_gpu0;
	alloc->free = drm_mod_free_gpu0;
	alloc->dump = NULL;

	*dev = &alloc->common;

	return 0;
}

static int drm_mod_open(const struct hw_module_t *mod,
		const char *name, struct hw_device_t **dev)
{
	struct drm_module_t *dmod = (struct drm_module_t *) mod;
	int err;

	if (strcmp(name, GRALLOC_HARDWARE_GPU0) == 0)
		err = drm_mod_open_gpu0(dmod, dev);
	else
		err = -EINVAL;

	return err;
}

static struct hw_module_methods_t drm_mod_methods = {
	.open = drm_mod_open
};

drm_module_t::drm_module_t()
{
    base.common.tag = HARDWARE_MODULE_TAG;
    base.common.version_major = 1;
    base.common.version_minor = 0;
    base.common.id = GRALLOC_HARDWARE_MODULE_ID;
    base.common.name = "DRM Memory Allocator";
    base.common.author = "Chia-I Wu";
    base.common.methods = &drm_mod_methods;

    base.registerBuffer = drm_mod_register_buffer;
    base.unregisterBuffer = drm_mod_unregister_buffer;
    base.lock = drm_mod_lock;
    base.lock_ycbcr = drm_mod_lock_ycbcr;
    base.unlock = drm_mod_unlock;
    base.perform = drm_mod_perform;

    mutex = PTHREAD_MUTEX_INITIALIZER;
    drm = NULL;

#if RK_DRM_GRALLOC
    refcount = 0;
#endif
}

struct drm_module_t HAL_MODULE_INFO_SYM;
