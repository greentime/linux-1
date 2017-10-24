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

#include <linux/vmalloc.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <asm/pgtable.h>

void __iomem *__ioremap(unsigned long phys_addr, size_t size,
			unsigned long flags, unsigned long align)
{
	struct vm_struct *area;
	unsigned long addr, offset, last_addr;
	pgprot_t prot;

	/* Don't allow wraparound or zero size */
	last_addr = phys_addr + size - 1;
	if (!size || last_addr < phys_addr)
		return NULL;

	/*
	 * Mappings have to be page-aligned
	 */
	offset = phys_addr & ~PAGE_MASK;
	phys_addr &= PAGE_MASK;
	size = PAGE_ALIGN(last_addr + 1) - phys_addr;

	/*
	 * Ok, go for it..
	 */
	area = get_vm_area(size, VM_IOREMAP);
	if (!area)
		return NULL;

	area->phys_addr = phys_addr;
	addr = (unsigned long)area->addr;
	prot = __pgprot(_PAGE_V | _PAGE_M_KRW | _PAGE_D |
			_PAGE_G | _PAGE_C_DEV);
	if (ioremap_page_range(addr, addr + size, phys_addr, prot)) {
		vunmap((void *)addr);
		return NULL;
	}
	return (__force void __iomem *)(offset + (char *)addr);

}

EXPORT_SYMBOL(__ioremap);

void __iounmap(void __iomem * addr)
{
	vunmap((void *)(PAGE_MASK & (unsigned long)addr));
}

EXPORT_SYMBOL(__iounmap);
