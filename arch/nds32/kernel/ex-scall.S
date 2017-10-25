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
#include <asm/unistd.h>
#include <asm/assembler.h>
#include <asm/nds32.h>
#include <asm/asm-offsets.h>
#include <asm/thread_info.h>
#include <asm/current.h>

/*
 * $r0 = previous task_struct,
 * $r1 = next task_struct,
 * previous and next are guaranteed not to be the same.
 */

ENTRY(__switch_to)

	la	$p0, __entry_task
	sw	$r1, [$p0]
	move	$p1, $r0
	addi	$p1, $p1, #THREAD_CPU_CONTEXT
	smw.bi 	$r6, [$p1], $r14, #0xb		! push r6~r14, fp, lp, sp
	move	$r25, $r1
	addi	$r1, $r1, #THREAD_CPU_CONTEXT
	lmw.bi 	$r6, [$r1], $r14, #0xb		! pop r6~r14, fp, lp, sp
	ret

/*
 * These are the registers used in the syscall handler, and allow us
 * to have in theory up to 6 arguments to a function - $r0 to $r5.
 *
 * We must set at least "tsk" and "tbl" when calling ret_with_reschedule.
 */
#define tbl	$r8            //  syscall table pointer

/*
 * $r7 will be writen as syscall nr
 * by retrieving from $ITYPE 'SWID' bitfiled
 */
	.macro	get_scno
	mfsr	$r7, $ITYPE
	srli	$r7, $r7, #ITYPE_offSWID
	swi	$r7, [$sp + SYSCALLNO_OFFSET]
	.endm

	.macro	updateipc
	addi	$r17, $r13, #4				! $r13 is $IPC
	swi	$r17, [$sp + IPC_OFFSET]
	.endm

ENTRY(eh_syscall)
	updateipc

	get_scno
	gie_enable

ENTRY(eh_syscall_phase_2)
	lwi	$p0, [tsk+#TSK_TI_FLAGS]		! check for syscall tracing

	andi	$p1, $p0, #_TIF_WORK_SYSCALL_ENTRY	! are we tracing syscalls?
	bnez	$p1, __sys_trace

	la	$lp, ret_fast_syscall		! return address
jmp_systbl:
	addi	$p1, $r7, #-__NR_syscalls	! syscall number of syscall instruction is guarded by addembler
	bgez	$p1, _SCNO_EXCEED		! call sys_* routine
	la	tbl, sys_call_table		! load syscall table pointer
	slli	$p1, $r7, #2
	add	$p1, tbl, $p1
	lwi	$p1, [$p1]
	jr	$p1				! no return

_SCNO_EXCEED:
	ori	$r0, $r7, #0
        ori	$r1, $sp, #0
	b	bad_syscall

/*
 * This is the really slow path.  We're going to be doing
 * context switches, and waiting for our parent to respond.
 */
__sys_trace:
	move	$r1, $sp
	move	$r0, $r7			! trace entry [IP = 0]
	bal	syscall_trace_enter
	move	$r7, $r0
	la	$lp, __sys_trace_return		! return address

	addi    $p1, $r7, #1
	beqz    $p1, ret_slow_syscall		! fatal signal is pending

	addi	$p1, $sp, #R0_OFFSET		! pointer to regs
	lmw.bi	$r0, [$p1], $r5			! have to reload $r0 - $r5
	b	jmp_systbl

__sys_trace_return:
	swi	$r0, [$sp+#R0_OFFSET]		! T: save returned $r0
	move	$r0, $sp			! set pt_regs for syscall_trace_leave
	bal	syscall_trace_leave
	b	ret_slow_syscall

/*
 * Special system call wrappers
 *
 * $r0 = syscall number
 * $r8 = syscall table
 */
       .type   sys_syscall, #function
ENTRY(sys_syscall)
	addi	$p1, $r0, #-__NR_syscalls
	bgtz	$p1, 3f
	move	$p1, $r0
	move	$r0, $r1
	move	$r1, $r2
	move	$r2, $r3
	move	$r3, $r4
	move	$r4, $r5
! add for syscall 6 args
	lwi	$r5, [$sp + #SP_OFFSET ]
	lwi	$r5, [$r5]
! ~add for syscall 6 args

	lw	$p1, [tbl+$p1<<2]
	jr	$p1
3:	b	sys_ni_syscall
ENDPROC(sys_syscall)

ENTRY(sys_rt_sigreturn_wrapper)
	addi	$r0, $sp, #0
	b	sys_rt_sigreturn
ENDPROC(sys_rt_sigreturn_wrapper)
