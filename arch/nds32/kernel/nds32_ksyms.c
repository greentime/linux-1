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

#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/in6.h>
#include <linux/syscalls.h>

#include <asm/checksum.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/ftrace.h>
#include <asm/proc-fns.h>

/* mem functions */
EXPORT_SYMBOL(memset);
EXPORT_SYMBOL(memcpy);
EXPORT_SYMBOL(memmove);
EXPORT_SYMBOL(memzero);

/* user mem (segment) */
EXPORT_SYMBOL(__arch_copy_from_user);
EXPORT_SYMBOL(__arch_copy_to_user);
EXPORT_SYMBOL(__arch_clear_user);

/* cache handling */
EXPORT_SYMBOL(cpu_icache_inval_all);
EXPORT_SYMBOL(cpu_dcache_wbinval_all);
EXPORT_SYMBOL(cpu_dma_inval_range);
EXPORT_SYMBOL(cpu_dma_wb_range);
