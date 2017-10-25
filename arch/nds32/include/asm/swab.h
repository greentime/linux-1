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

#ifndef __NDS32_SWAB_H__
#define __NDS32_SWAB_H__

#include <linux/types.h>
#include <linux/compiler.h>

static __inline__ __attribute_const__ __u32 ___arch__swab32(__u32 x)
{
	__asm__("wsbh   %0, %0\n\t"	/* word swap byte within halfword */
		"rotri  %0, %0, #16\n"
		:"=r"(x)
		:"0"(x));
	return x;
}

static __inline__ __attribute_const__ __u16 ___arch__swab16(__u16 x)
{
	__asm__("wsbh   %0, %0\n"	/* word swap byte within halfword */
		:"=r"(x)
		:"0"(x));
	return x;
}

#define __arch_swab32(x) ___arch__swab32(x)
#define __arch_swab16(x) ___arch__swab16(x)

#if !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#define __BYTEORDER_HAS_U64__
#define __SWAB_64_THRU_32__
#endif

#endif /* __NDS32_SWAB_H__ */
