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

#include <asm/errno.h>
#include <linux/linkage.h>

	.text

ENTRY(__put_user_1)
1:	sb	$r2, [$r0]
	move	$r0, #0
	ret

ENTRY(__put_user_2)
2:	shi	$r2, [$r0]	! Store input halfword
	move	$r0, #0
	ret

ENTRY(__put_user_4)
3:	sw	$r2, [$r0]
	move	$r0, #0
	ret

ENTRY(__put_user_8)
4:	swi.bi	$r2, [$r0], #4
5:	sw	$r3, [$r0]
	move	$r0, #0
	ret

__put_user_bad:
	move	$r0, #-EFAULT
	ret

.section __ex_table, "a"
	.long	1b, __put_user_bad
	.long	2b, __put_user_bad
	.long	3b, __put_user_bad
	.long	4b, __put_user_bad
	.long	5b, __put_user_bad
.previous
