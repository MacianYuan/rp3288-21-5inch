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

#ifndef _PCI_ID_DRIVER_MAP_H_
#define _PCI_ID_DRIVER_MAP_H_

#include <stddef.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#if !defined(DRIVER_MAP_DRI2_ONLY) && !defined(DRIVER_MAP_GALLIUM_ONLY)
static const int i810_chip_ids[] = {
#define CHIPSET(chip, desc, misc) chip,
#include "pci_ids/i810_pci_ids.h"
#undef CHIPSET
};
#endif

static const int i915_chip_ids[] = {
#define CHIPSET(chip, desc, misc) chip,
#include "pci_ids/i915_pci_ids.h"
#undef CHIPSET
};

static const int i965_chip_ids[] = {
#define CHIPSET(chip, desc, misc) chip,
#include "pci_ids/i965_pci_ids.h"
#undef CHIPSET
};

#ifndef DRIVER_MAP_GALLIUM_ONLY
static const int r100_chip_ids[] = {
#define CHIPSET(chip, name, family) chip,
#include "pci_ids/radeon_pci_ids.h"
#undef CHIPSET
};

static const int r200_chip_ids[] = {
#define CHIPSET(chip, name, family) chip,
#include "pci_ids/r200_pci_ids.h"
#undef CHIPSET
};
#endif

static const int r300_chip_ids[] = {
#define CHIPSET(chip, name, family) chip,
#include "pci_ids/r300_pci_ids.h"
#undef CHIPSET
};

static const int r600_chip_ids[] = {
#define CHIPSET(chip, name, family) chip,
#include "pci_ids/r600_pci_ids.h"
#undef CHIPSET
};

static const int vmwgfx_chip_ids[] = {
#define CHIPSET(chip, name, family) chip,
#include "pci_ids/vmwgfx_pci_ids.h"
#undef CHIPSET
};

static const struct {
   int vendor_id;
   const char *driver;
   const int *chip_ids;
   int num_chips_ids;
} driver_map[] = {
#if !defined(DRIVER_MAP_DRI2_ONLY) && !defined(DRIVER_MAP_GALLIUM_ONLY)
   { 0x8086, "i810", i810_chip_ids, ARRAY_SIZE(i810_chip_ids) },
#endif
   { 0x8086, "i915", i915_chip_ids, ARRAY_SIZE(i915_chip_ids) },
   { 0x8086, "i965", i965_chip_ids, ARRAY_SIZE(i965_chip_ids) },
#ifndef DRIVER_MAP_GALLIUM_ONLY
   { 0x1002, "radeon", r100_chip_ids, ARRAY_SIZE(r100_chip_ids) },
   { 0x1002, "r200", r200_chip_ids, ARRAY_SIZE(r200_chip_ids) },
#endif
   { 0x1002, "r300", r300_chip_ids, ARRAY_SIZE(r300_chip_ids) },
   { 0x1002, "r600", r600_chip_ids, ARRAY_SIZE(r600_chip_ids) },
   { 0x10de, "nouveau", NULL, -1 },
   { 0x15ad, "vmwgfx", vmwgfx_chip_ids, ARRAY_SIZE(vmwgfx_chip_ids) },
   { 0x0000, NULL, NULL, 0 },
};

#endif /* _PCI_ID_DRIVER_MAP_H_ */
