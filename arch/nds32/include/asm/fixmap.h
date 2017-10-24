/*
 * Copyright (C) 2005-2017 Andes Technology Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ASM_NDS32_FIXMAP_H
#define __ASM_NDS32_FIXMAP_H

#ifdef CONFIG_HIGHMEM
#include <linux/threads.h>
#include <asm/kmap_types.h>
#endif

enum fixed_addresses {
	FIX_KMAP_RESERVED,
	FIX_KMAP_BEGIN,
#ifdef CONFIG_HIGHMEM
	FIX_KMAP_END = FIX_KMAP_BEGIN + (KM_TYPE_NR * NR_CPUS),
#endif
#ifdef CONFIG_EARLY_PRINTK
	FIX_EARLY_DEBUG,
#endif
	__end_of_fixed_addresses
};
#define FIXADDR_TOP             ((unsigned long) (-(16 * PAGE_SIZE)))
#define FIXADDR_SIZE		((__end_of_fixed_addresses) << PAGE_SHIFT)
#define FIXADDR_START		(FIXADDR_TOP - FIXADDR_SIZE)

#include <asm-generic/fixmap.h>
#endif /* __ASM_NDS32_FIXMAP_H */
