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

#ifndef __ASM_NDS32_PTRACE_H
#define __ASM_NDS32_PTRACE_H

#define PTRACE_GETREGS		12
#define PTRACE_SETREGS		13
#define PTRACE_GETFPREGS	14
#define PTRACE_SETFPREGS	15

#include <uapi/asm/ptrace.h>

#ifndef __ASSEMBLY__

struct pt_regs {
	union {
		struct user_pt_regs user_regs;
		struct {
			long uregs[26];
			long fp;
			long gp;
			long lp;
			long sp;
			long ipc;
#if defined(CONFIG_HWZOL)
			long lb;
			long le;
			long lc;
#else
			long dummy[3];
#endif
			long syscallno;
		};
	};
	long orig_r0;
	long ir0;
	long ipsw;
	long pipsw;
	long pipc;
	long pp0;
	long pp1;
	long fucop_ctl;
	long osp;
};

#include <asm/bitfield.h>
extern void show_regs(struct pt_regs *);
/* Avoid circular header include via sched.h */
struct task_struct;
extern void send_sigtrap(struct task_struct *tsk, struct pt_regs *regs,
			 int error_code, int si_code);

#define arch_has_single_step()		(1)
#define user_mode(regs)			(((regs)->ipsw & PSW_mskPOM) == 0)
#define interrupts_enabled(regs)	(!!((regs)->ipsw & PSW_mskGIE))
#define valid_user_regs(regs)		(user_mode(regs) && interrupts_enabled(regs))
#define regs_return_value(regs)		((regs)->uregs[0])
#define instruction_pointer(regs)	((regs)->ipc)
#define user_stack_pointer(regs)        ((regs)->sp)
#define profile_pc(regs) 		instruction_pointer(regs)

#define ARCH_HAS_USER_SINGLE_STEP_INFO

#endif /* __ASSEMBLY__ */
#endif
