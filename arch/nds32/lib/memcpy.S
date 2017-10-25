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

	.text

ENTRY(memcpy)
	move	$r5, $r0
	beq	$r0, $r1, quit_memcpy
	beqz	$r2, quit_memcpy
	srli    $r3, $r2, #5	 ! check if len < cache-line size 32
	beqz	$r3, word_copy_entry
	andi	$r4, $r0, #0x3	! check byte-align
	beqz	$r4, unalign_word_copy_entry

	addi	$r4, $r4,#-4
	abs	$r4, $r4		! check how many un-align byte to copy
	sub	$r2, $r2, $r4	! update $R2

unalign_byte_copy:
	lbi.bi	$r3, [$r1], #1
	addi	$r4, $r4, #-1
	sbi.bi	$r3, [$r0], #1
	bnez	$r4, unalign_byte_copy
	beqz	$r2, quit_memcpy

unalign_word_copy_entry:
	andi	$r3, $r0, 0x1f	! check cache-line unaligncount
	beqz	$r3, cache_copy

	addi	$r3, $r3, #-32
	abs	$r3, $r3
	sub	$r2, $r2, $r3	! update $R2

unalign_word_copy:
	lmw.bim	$r4, [$r1], $r4
	addi	$r3, $r3, #-4
	smw.bim	$r4, [$r0], $r4
	bnez	$r3, unalign_word_copy
	beqz	$r2, quit_memcpy

	addi	$r3, $r2, #-32	! to check $r2< cache_line , than go to word_copy
	bltz	$r3, word_copy_entry
cache_copy:
	srli	$r3, $r2, #5
	beqz	$r3, word_copy_entry
	pushm	$r6, $r13
3:
	lmw.bim	$r6, [$r1], $r13
	addi	$r3, $r3, #-1
	smw.bim	$r6, [$r0], $r13
	bnez	$r3, 3b
	popm	$r6, $r13

word_copy_entry:
	andi	$r2, $r2, #31

	beqz	$r2, quit_memcpy
5:
	srli	$r3, $r2, #2
	beqz	$r3, byte_copy
word_copy:
	lmw.bim	$r4, [$r1], $r4
	addi	$r3, $r3, #-1
	smw.bim	$r4, [$r0], $r4
	bnez	$r3, word_copy
	andi	$r2, $r2, #3
	beqz	$r2, quit_memcpy
byte_copy:
	lbi.bi	$r3, [$r1], #1
	addi	$r2, $r2, #-1

	sbi.bi	$r3, [$r0], #1
	bnez	$r2, byte_copy
quit_memcpy:
	move	$r0, $r5
	ret

ENDPROC(memcpy)
