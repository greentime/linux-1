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

#ifndef __NDS32_ASSEMBLER_H__
#define __NDS32_ASSEMBLER_H__

.macro gie_disable
	setgie.d
	dsb
.endm

.macro gie_enable
	setgie.e
	dsb
.endm

.macro gie_save oldpsw
	mfsr \oldpsw, $ir0
	setgie.d
        dsb
.endm

.macro gie_restore oldpsw
	andi \oldpsw, \oldpsw, #0x1
	beqz \oldpsw, 7001f
	setgie.e
	dsb
7001:
.endm


#define USER(insn,  reg, addr, opr)	\
9999:	insn  reg, addr, opr;		\
	.section __ex_table,"a";	\
	.align 3;			\
	.long	9999b, 9001f;		\
	.previous

#endif /* __NDS32_ASSEMBLER_H__ */
