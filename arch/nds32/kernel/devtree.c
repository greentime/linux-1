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

#include <linux/memblock.h>
#include <linux/of_fdt.h>
#include <linux/bootmem.h>

void __init early_init_dt_add_memory_arch(u64 base, u64 size)
{
	size &= PAGE_MASK;
	memblock_add_node(base, size, 0);
}

void *__init early_init_dt_alloc_memory_arch(u64 size, u64 align)
{
	return alloc_bootmem_align(size, align);
}

void __init early_init_devtree(void *params)
{
	if (!params || !early_init_dt_scan(params)) {
		pr_crit("\n"
			"Error: invalid device tree blob at (virtual address 0x%p)\n"
			"The dtb must be 8-byte aligned and must not exceed 8 KB in size\n"
			"\nPlease check your bootloader.", params);

		while (true)
			cpu_relax();
	}

	dump_stack_set_arch_desc("%s (DT)", of_flat_dt_get_machine_name());
}
