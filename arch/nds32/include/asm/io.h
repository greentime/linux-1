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

#ifndef __ASM_NDS32_IO_H
#define __ASM_NDS32_IO_H

#ifdef __KERNEL__

#include <asm-generic/iomap.h>
extern void __iomem *__ioremap(unsigned long, size_t, unsigned long,
			       unsigned long);
extern void __iounmap(void __iomem * addr);

#define ioremap(cookie,size)		__ioremap(cookie,size,0,1)
#define ioremap_nocache(cookie,size)	__ioremap(cookie,size,0,1)
#define iounmap(cookie)			__iounmap(cookie)
#include <asm-generic/io.h>

#endif /* __KERNEL__ */
#endif /* __ASM_NDS32_IO_H */
