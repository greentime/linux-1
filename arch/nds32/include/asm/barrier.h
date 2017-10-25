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

#ifndef __NDS32_ASM_BARRIER_H
#define __NDS32_ASM_BARRIER_H

#ifndef __ASSEMBLY__
#define mb()		asm volatile("msync all":::"memory")
#define rmb()		asm volatile("msync all":::"memory")
#define wmb()		asm volatile("msync store":::"memory")
#include <asm-generic/barrier.h>

#endif	/* __ASSEMBLY__ */

#endif	/* __NDS32_ASM_BARRIER_H */
