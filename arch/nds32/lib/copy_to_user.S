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

#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/errno.h>

.macro 	lbi1 dst, addr, adj
lbi.bi	\dst, [\addr], \adj
.endm

.macro 	sbi1 src, addr, adj
USER( sbi.bi, \src, [\addr], \adj)
.endm

.macro	lmw1 start_reg, addr, end_reg
lmw.bim \start_reg, [\addr], \end_reg
.endm

.macro	smw1 start_reg, addr, end_reg
USER( smw.bim, \start_reg, [\addr], \end_reg)
.endm


/* Prototype: int __arch_copy_to_user(void *to, const char *from, size_t n)
 * Purpose  : copy a block to user memory from kernel memory
 * Params   : to   - user memory
 *          : from - kernel memory
 *          : n    - number of bytes to copy
 * Returns  : Number of bytes NOT copied.
 */

.text
ENTRY(__arch_copy_to_user)
	add	$r5, $r0, $r2
#include "copy_template.S"
	move	$r0, $r2
	ret
.section .fixup,"ax"
.align  2
9001:
	sub	$r0, $r5, $r0
	ret
.previous
ENDPROC(__arch_copy_to_user)
