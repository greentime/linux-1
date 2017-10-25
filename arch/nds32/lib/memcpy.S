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


.macro 	lbi1 dst, addr, adj
lbi.bi	\dst, [\addr], \adj
.endm

.macro 	sbi1 src, addr, adj
sbi.bi	\src, [\addr], \adj
.endm

.macro	lmw1 start_reg, addr, end_reg
lmw.bim \start_reg, [\addr], \end_reg
.endm

.macro	smw1 start_reg, addr, end_reg
smw.bim \start_reg, [\addr], \end_reg
.endm

.text
ENTRY(memcpy)
	move	$r5, $r0
#include "copy_template.S"
	move	$r0, $r5
	ret

ENDPROC(memcpy)
