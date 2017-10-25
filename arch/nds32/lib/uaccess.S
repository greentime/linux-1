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

	.text

/* Prototype: int __arch_copy_to_user(void *to, const char *from, size_t n)
 * Purpose  : copy a block to user memory from kernel memory
 * Params   : to   - user memory
 *          : from - kernel memory
 *          : n    - number of bytes to copy
 * Returns  : Number of bytes NOT copied.
 */

ENTRY(__arch_copy_to_user)
	push	$r0
	push	$r2
	beqz	$r2, ctu_exit
	srli	$p0, $r2, #2		! $p0 = number of word to clear
	andi	$r2, $r2, #3		! Bytes less than a word to copy
	beqz	$p0, byte_ctu		! Only less than a word to copy
word_ctu:
	lmw.bim	$p1, [$r1], $p1		! Load the next word
USER(	smw.bim,$p1, [$r0], $p1)	! Store the next word
	addi	$p0, $p0, #-1		! Decrease word count
	bnez	$p0, word_ctu		! Continue looping to copy all words
	beqz	$r2, ctu_exit		! No left bytes to copy
byte_ctu:
	lbi.bi	$p1, [$r1], #1		! Load the next byte
USER(	sbi.bi,	$p1, [$r0], #1)		! Store the next byte
	addi	$r2, $r2, #-1		! Decrease byte count
	bnez	$r2, byte_ctu		! Continue looping to clear all left bytes
ctu_exit:
	move	$r0, $r2		! Set return value
	pop	$r2
	pop	$r2			! Pop saved $r0 to $r2 to not corrupt return value
	ret

	.section .fixup,"ax"
	.align	0
9001:
	pop	$p1			! Original $r2, n
	pop	$p0			! Original $r0, void *to
	sub	$r1, $r0, $p0		! Bytes copied
	sub	$r2, $p1, $r1		! Bytes left to copy
	push	$lp
	move	$r0, $p0
	bal	memzero			! Clean up the memory
	pop	$lp
	move	$r0, $r2
	ret
	.previous
ENDPROC(__arch_copy_to_user)

/* Prototype: unsigned long __arch_copy_from_user(void *to,const void *from,unsigned long n);
 * Purpose  : copy a block from user memory to kernel memory
 * Params   : to   - kernel memory
 *          : from - user memory
 *          : n    - number of bytes to copy
 * Returns  : Number of bytes NOT copied.
 */


ENTRY(__arch_copy_from_user)
	push	$r1
	push	$r2
	beqz	$r2, cfu_exit
	srli	$p0, $r2, #2		! $p0 = number of word to clear
	andi	$r2, $r2, #3		! Bytes less than a word to copy
	beqz	$p0, byte_cfu		! Only less than a word to copy
word_cfu:
USER(	lmw.bim,$p1, [$r1], $p1)	! Load the next word
	smw.bim	$p1, [$r0], $p1		! Store the next word
	addi	$p0, $p0, #-1		! Decrease word count
	bnez	$p0, word_cfu		! Continue looping to copy all words
	beqz	$r2, cfu_exit		! No left bytes to copy
byte_cfu:
USER(	lbi.bi,	$p1, [$r1], #1)		! Load the next byte
	sbi.bi	$p1, [$r0], #1		! Store the next byte
	addi	$r2, $r2, #-1		! Decrease byte count
	bnez	$r2, byte_cfu		! Continue looping to clear all left bytes
cfu_exit:
	move	$r0, $r2		! Set return value
	pop	$r2
	pop	$r1
	ret

	.section .fixup,"ax"
	.align	0
	/*
	 * We took an exception.  $r0 contains a pointer to
	 * the byte not copied.
	 */
9001:
	pop	$p1			! Original $r2, n
	pop	$p0			! Original $r0, void *to
	sub	$r1, $r1, $p0		! Bytes copied
	sub	$r2, $p1, $r1		! Bytes left to copy
	push	$lp
	bal	memzero			! Clean up the memory
	pop	$lp
	move	$r0, $r2
	ret
	.previous
ENDPROC(__arch_copy_from_user)

/* Prototype: int __arch_clear_user(void *addr, size_t sz)
 * Purpose  : clear some user memory
 * Params   : addr - user memory address to clear
 *          : sz   - number of bytes to clear
 * Returns  : number of bytes NOT cleared
 */
	.align	5
ENTRY(__arch_clear_user)
	pushm	$r0, $r1
	beqz	$r1, clear_exit
	xor	$p1, $p1, $p1		! Use $p1=0 to clear mem
	srli	$p0, $r1, #2		! $p0 = number of word to clear
	andi	$r1, $r1, #3		! Bytes less than a word to copy
	beqz	$p0, byte_clear		! Only less than a word to clear
word_clear:
USER(	smw.bim,$p1, [$r0], $p1)	! Clear the word
	addi	$p0, $p0, #-1		! Decrease word count
	bnez	$p0, word_clear		! Continue looping to clear all words
	beqz	$r1, clear_exit		! No left bytes to copy
byte_clear:
USER(	sbi.bi,	$p1, [$r0], #1)		! Clear the byte
	addi	$r1, $r1, #-1		! Decrease byte count
	bnez	$r1, byte_clear		! Continue looping to clear all left bytes
clear_exit:
	move	$r0, $r1		! Set return value
	pop	$r1
	pop	$r1			! Pop saved $r0 to $r1 to not corrupt return value
	ret

	.section .fixup,"ax"
	.align	0
9001:
	popm	$p0, $p1		! $p0 = original $r0, *addr, $p1 = original $r1, n
	sub	$p0, $r0, $p0		! Bytes copied
	sub	$r0, $p1, $p0		! Bytes left to copy
	ret
	.previous
ENDPROC(__arch_clear_user)
