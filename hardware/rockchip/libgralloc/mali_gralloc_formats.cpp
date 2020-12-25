/*
 * Copyright (C) 2016-2017 ARM Limited. All rights reserved.
 *
 * Copyright (C) 2008 The Android Open Source Project
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

#define LOG_TAG "gralloc"
// #define ENABLE_DEBUG_LOG
#include <log/custom_log.h>

#include <string.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <cutils/log.h>

#if GRALLOC_USE_GRALLOC1_API == 1
#include <hardware/gralloc1.h>
#else
#include <hardware/gralloc.h>
#endif

// #include "mali_gralloc_module.h"
// #include "gralloc_priv.h"
#include "gralloc_helper.h"
#include "mali_gralloc_formats.h"
#include "mali_gralloc_usages.h"

static int map_flex_formats(uint64_t req_format)
{
	/* Map Android flexible formats to internal base formats */
	if (req_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED || req_format == HAL_PIXEL_FORMAT_YCbCr_420_888)
	{
		req_format = MALI_GRALLOC_FORMAT_INTERNAL_NV12;
	}
	else if (req_format == HAL_PIXEL_FORMAT_YCbCr_422_888)
	{
		/* To be determined */
	}
	else if (req_format == HAL_PIXEL_FORMAT_YCbCr_444_888)
	{
		/* To be determined */
	}

	return req_format;
}

uint64_t mali_gralloc_select_format(uint64_t req_format, mali_gralloc_format_type type, uint64_t usage, int buffer_size)
{
    uint64_t internal_format;
    GRALLOC_UNUSED(type);
    GRALLOC_UNUSED(usage);
    GRALLOC_UNUSED(buffer_size);

    if ( req_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED )
    {
        if ( GRALLOC_USAGE_HW_VIDEO_ENCODER == (usage & GRALLOC_USAGE_HW_VIDEO_ENCODER) )
        {
            I("(usage & GRALLOC_USAGE_HW_VIDEO_ENCODER treat as NV12");
            internal_format = HAL_PIXEL_FORMAT_YCrCb_NV12;
        }
        else
        {
            I("treat as NV12 888");
            internal_format = HAL_PIXEL_FORMAT_RGBX_8888;
        }
    }
    else if ( req_format == HAL_PIXEL_FORMAT_YCrCb_NV12_10
        && USAGE_CONTAIN_VALUE(GRALLOC_USAGE_TO_USE_ARM_P010, GRALLOC_USAGE_ROT_MASK) )
    {
        ALOGV("rk_debug force  MALI_GRALLOC_FORMAT_INTERNAL_P010 usage=0x%" PRIx64, usage);
        internal_format = MALI_GRALLOC_FORMAT_INTERNAL_P010; // base_format of internal_format, no modifiers.
    }
    else
    {
        internal_format = map_flex_formats(req_format);
    }

    return internal_format;
}

