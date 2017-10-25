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

#ifndef __NDS32_DELAY_H__
#define __NDS32_DELAY_H__

#include <asm/param.h>

static inline void __delay(unsigned long loops)
{
	__asm__ __volatile__(".align 2\n"
			     "1:\n"
			     "\taddi\t%0, %0, -1\n"
			     "\tbgtz\t%0, 1b\n"
			     :"=r"(loops)
			     :"0"(loops));
}

static inline void __udelay(unsigned long usecs, unsigned long lpj)
{
	usecs *= (unsigned long)(((0x8000000000000000ULL / (500000 / HZ)) +
				  0x80000000ULL) >> 32);
	usecs = (unsigned long)(((unsigned long long)usecs * lpj) >> 32);
	__delay(usecs);
}

#define udelay(usecs) __udelay((usecs), loops_per_jiffy)

/* make sure "usecs *= ..." in udelay do not overflow. */
#if HZ >= 1000
#define MAX_UDELAY_MS	1
#elif HZ <= 200
#define MAX_UDELAY_MS	5
#else
#define MAX_UDELAY_MS	(1000 / HZ)
#endif

#endif
