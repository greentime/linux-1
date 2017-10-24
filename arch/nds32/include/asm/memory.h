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

#ifndef __ASM_NDS32_MEMORY_H
#define __ASM_NDS32_MEMORY_H

#include <linux/compiler.h>
#include <linux/sizes.h>

#ifndef __ASSEMBLY__
#include <asm/page.h>
#endif

#ifndef PHYS_OFFSET
#define PHYS_OFFSET     (0x0)
#endif

#ifndef __virt_to_bus
#define __virt_to_bus	__virt_to_phys
#endif

#ifndef __bus_to_virt
#define __bus_to_virt	__phys_to_virt
#endif

/*
 * TASK_SIZE - the maximum size of a user space task.
 * TASK_UNMAPPED_BASE - the lower boundary of the mmap VM area
 */
#define TASK_SIZE		((CONFIG_PAGE_OFFSET) - (SZ_32M))
#define TASK_UNMAPPED_BASE	ALIGN(TASK_SIZE / 3, SZ_32M)
#define PAGE_OFFSET		(CONFIG_PAGE_OFFSET)

/*
 * Physical vs virtual RAM address space conversion.  These are
 * private definitions which should NOT be used outside memory.h
 * files.  Use virt_to_phys/phys_to_virt/__pa/__va instead.
 */
#ifndef __virt_to_phys
#define __virt_to_phys(x)	((x) - PAGE_OFFSET + PHYS_OFFSET)
#define __phys_to_virt(x)	((x) - PHYS_OFFSET + PAGE_OFFSET)
#endif

/*
 * The module space lives between the addresses given by TASK_SIZE
 * and PAGE_OFFSET - it must be within 32MB of the kernel text.
 */
#define MODULES_END	(PAGE_OFFSET)
#define MODULES_VADDR	(MODULES_END - SZ_32M)

#if TASK_SIZE > MODULES_VADDR
#error Top of user space clashes with start of module space
#endif

#ifndef __ASSEMBLY__

/*
 * PFNs are used to describe any physical page; this means
 * PFN 0 == physical address 0.
 *
 * This is the PFN of the first RAM page in the kernel
 * direct-mapped view.  We assume this is the first page
 * of RAM in the mem_map as well.
 */
#define PHYS_PFN_OFFSET	(PHYS_OFFSET >> PAGE_SHIFT)

/*
 * Drivers should NOT use these either.
 */
#define __pa(x)			__virt_to_phys((unsigned long)(x))
#define __va(x)			((void *)__phys_to_virt((unsigned long)(x)))

/*
 * Conversion between a struct page and a physical address.
 *
 * Note: when converting an unknown physical address to a
 * struct page, the resulting pointer must be validated
 * using VALID_PAGE().  It must return an invalid struct page
 * for any physical address not corresponding to a system
 * RAM address.
 *
 *  pfn_valid(pfn)	indicates whether a PFN number is valid
 *
 *  virt_to_page(k)	convert a _valid_ virtual address to struct page *
 *  virt_addr_valid(k)	indicates whether a virtual address is valid
 */
#ifndef CONFIG_DISCONTIGMEM

#define ARCH_PFN_OFFSET		PHYS_PFN_OFFSET
#define pfn_valid(pfn)		((pfn) >= PHYS_PFN_OFFSET && (pfn) < (PHYS_PFN_OFFSET + max_mapnr))

#define virt_to_page(kaddr)	(pfn_to_page(__pa(kaddr) >> PAGE_SHIFT))
#define virt_addr_valid(kaddr)	((unsigned long)(kaddr) >= PAGE_OFFSET && (unsigned long)(kaddr) < (unsigned long)high_memory)

#else /* CONFIG_DISCONTIGMEM */
#error CONFIG_DISCONTIGMEM is not supported yet.
#endif /* !CONFIG_DISCONTIGMEM */

#define page_to_phys(page)	(page_to_pfn(page) << PAGE_SHIFT)

#endif

#include <asm-generic/memory_model.h>

#endif
