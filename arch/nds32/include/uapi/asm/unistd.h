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

#define __ARCH_WANT_SYNC_FILE_RANGE2

/* Use the standard ABI for syscalls */
#include <asm-generic/unistd.h>

/* Additional NDS32 specific syscalls. */
#define __NR_cacheflush		(__NR_arch_specific_syscall)
#define __NR__llseek             __NR_llseek
__SYSCALL(__NR_cacheflush, sys_cacheflush)
