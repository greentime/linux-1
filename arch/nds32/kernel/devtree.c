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

#include <linux/bug.h>
#include <linux/printk.h>
#include <linux/of_fdt.h>

void __init early_init_devtree(void *params)
{
	if (!params || !early_init_dt_scan(params)) {
		pr_crit("\n"
			"Error: invalid device tree blob at (virtual address 0x%p)\n"
			"\nPlease check your bootloader.", params);

		BUG_ON(1);
	}

	dump_stack_set_arch_desc("%s (DT)", of_flat_dt_get_machine_name());
}
