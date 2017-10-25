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

#include <asm/asm-offsets.h>
#include <asm/thread_info.h>
#include <asm/errno.h>
#include <linux/linkage.h>


ENTRY(__get_user_1)
1:	lbi	$r2, [$r0]
	move	$r0, #0
	ret

ENTRY(__get_user_2)
2:	lhi	$r2, [$r0]
	move	$r0, #0
	ret

ENTRY(__get_user_4)
3:	lwi	$r2, [$r0]
	move	$r0, #0
	ret

ENTRY(__get_user_8)
4:	lwi.bi	$r2, [$r0], #4
5:	lwi	$r3, [$r0]
	move	$r0, #0
	ret

__get_user_bad_8:
	move	$r3, #0
__get_user_bad:
	move	$r2, #0
	move	$r0, #-EFAULT
	ret

.section __ex_table, "a"
	.long	1b, __get_user_bad
	.long	2b, __get_user_bad
	.long	3b, __get_user_bad
	.long	4b, __get_user_bad_8
	.long	5b, __get_user_bad_8
.previous
