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

#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/kbuild.h>
#include <asm/thread_info.h>
#include <asm/ptrace.h>

int main(void)
{
	DEFINE(TSK_TI_FLAGS, offsetof(struct task_struct, thread_info.flags));
	DEFINE(TSK_TI_PREEMPT,
	       offsetof(struct task_struct, thread_info.preempt_count));
	DEFINE(THREAD_CPU_CONTEXT,
	       offsetof(struct task_struct, thread.cpu_context));
	DEFINE(OSP_OFFSET, offsetof(struct pt_regs, osp));
	DEFINE(SP_OFFSET, offsetof(struct pt_regs, sp));
	DEFINE(FUCOP_CTL_OFFSET, offsetof(struct pt_regs, fucop_ctl));
	DEFINE(IPSW_OFFSET, offsetof(struct pt_regs, ipsw));
	DEFINE(SYSCALLNO_OFFSET, offsetof(struct pt_regs, syscallno));
	DEFINE(IPC_OFFSET, offsetof(struct pt_regs, ipc));
	DEFINE(R0_OFFSET, offsetof(struct pt_regs, uregs[0]));
	DEFINE(CLOCK_REALTIME_RES, MONOTONIC_RES_NSEC);
	DEFINE(CLOCK_COARSE_RES, LOW_RES_NSEC);
	return 0;
}
