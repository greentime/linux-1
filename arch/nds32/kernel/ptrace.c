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

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/ptrace.h>
#include <linux/user.h>
#include <linux/security.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/regset.h>
#include <linux/tracehook.h>
#include <linux/elf.h>
#include <linux/sched/task_stack.h>

#include <asm/uaccess.h>
#include <asm/pgtable.h>

enum nds32_regset {
	REGSET_GPR,
};

static int gpr_get(struct task_struct *target,
		   const struct user_regset *regset,
		   unsigned int pos, unsigned int count,
		   void *kbuf, void __user * ubuf)
{
	struct user_pt_regs *uregs = &task_pt_regs(target)->user_regs;
	return user_regset_copyout(&pos, &count, &kbuf, &ubuf, uregs, 0, -1);
}

static int gpr_set(struct task_struct *target, const struct user_regset *regset,
		   unsigned int pos, unsigned int count,
		   const void *kbuf, const void __user * ubuf)
{
	int err;
	struct user_pt_regs newregs = task_pt_regs(target)->user_regs;

	err = user_regset_copyin(&pos, &count, &kbuf, &ubuf, &newregs, 0, -1);
	if (err)
		return err;

	task_pt_regs(target)->user_regs = newregs;
	return 0;
}

static const struct user_regset nds32_regsets[] = {
	[REGSET_GPR] = {
			.core_note_type = NT_PRSTATUS,
			.n = sizeof(struct user_pt_regs) / sizeof(u32),
			.size = sizeof(u32),
			.align = sizeof(u32),
			.get = gpr_get,
			.set = gpr_set}
};

static const struct user_regset_view nds32_user_view = {
	.name = "nds32",.e_machine = EM_NDS32,
	.regsets = nds32_regsets,.n = ARRAY_SIZE(nds32_regsets)
};

const struct user_regset_view *task_user_regset_view(struct task_struct *task)
{
	return &nds32_user_view;
}

/* get_user_reg()
 *
 * This routine will get a word off of the processes privileged stack.
 * the offset is how far from the base addr as stored in the THREAD.
 * this routine assumes that all the privileged stacks are in our
 * data space.
 */
static inline unsigned int get_user_reg(struct task_struct *task, int offset)
{
	return task_pt_regs(task)->uregs[offset];
}

/* put_user_reg()
 *
 * this routine will put a word on the processes privileged stack.
 * the offset is how far from the base addr as stored in the THREAD.
 * this routine assumes that all the privileged stacks are in our
 * data space.
 */
static inline int put_user_reg(struct task_struct *task, int offset, long data)
{
	struct pt_regs newregs, *regs = task_pt_regs(task);
	int ret = -EINVAL;

	newregs = *regs;
	newregs.uregs[offset] = data;

	if (valid_user_regs(&newregs)) {
		regs->uregs[offset] = data;
		ret = 0;
	}

	return ret;
}

/*
 * Called by kernel/ptrace.c when detaching..
 *
 * Make sure the single step bit is not set.
 */
void ptrace_disable(struct task_struct *child)
{
	user_disable_single_step(child);
}

static void fill_sigtrap_info(struct task_struct *tsk,
			      struct pt_regs *regs,
			      int error_code, int si_code, struct siginfo *info)
{
	tsk->thread.trap_no = ENTRY_DEBUG_RELATED;
	tsk->thread.error_code = error_code;

	memset(info, 0, sizeof(*info));
	info->si_signo = SIGTRAP;
	info->si_code = si_code;
	info->si_addr = (void __user *)instruction_pointer(regs);
}

void user_single_step_siginfo(struct task_struct *tsk,
			      struct pt_regs *regs, struct siginfo *info)
{
	fill_sigtrap_info(tsk, regs, 0, TRAP_BRKPT, info);
}

/*
 * Handle hitting a breakpoint.
 */
void send_sigtrap(struct task_struct *tsk, struct pt_regs *regs,
		  int error_code, int si_code)
{
	struct siginfo info;

	fill_sigtrap_info(tsk, regs, error_code, TRAP_BRKPT, &info);
	/* Send us the fake SIGTRAP */
	force_sig_info(SIGTRAP, &info, tsk);
}

/* ptrace_read_user()
 *
 * Read the word at offset "off" into the "struct user".  We
 * actually access the pt_regs stored on the kernel stack.
 */
static int
ptrace_read_user(struct task_struct *tsk, unsigned long off,
		 unsigned long __user * ret)
{
	unsigned long tmp = 0;

	if (off < sizeof(struct pt_regs)) {
		if (off & 3)
			return -EIO;
		tmp = get_user_reg(tsk, off >> 2);
		return put_user(tmp, ret);
	} else
		return -EIO;
}

/* ptrace_write_user()
 *
 * Write the word at offset "off" into "struct user".  We
 * actually access the pt_regs stored on the kernel stack.
 */
static int
ptrace_write_user(struct task_struct *tsk, unsigned long off, unsigned long val)
{
	if (off < sizeof(struct pt_regs)) {
		if (off & 3)
			return -EIO;
		return put_user_reg(tsk, off >> 2, val);
	} else
		return -EIO;
}

/* ptrace_getregs()
 *
 * Get all user integer registers.
 */
static int ptrace_getregs(struct task_struct *tsk, void __user * uregs)
{
	struct pt_regs *regs = task_pt_regs(tsk);

	return copy_to_user(uregs, regs, sizeof(struct pt_regs)) ? -EFAULT : 0;
}

/* ptrace_setregs()
 *
 * Set all user integer registers.
 */
static int ptrace_setregs(struct task_struct *tsk, void __user * uregs)
{
	struct pt_regs newregs;
	int ret;

	ret = -EFAULT;
	if (copy_from_user(&newregs, uregs, sizeof(struct pt_regs)) == 0) {
		struct pt_regs *regs = task_pt_regs(tsk);

		ret = -EINVAL;
		if (valid_user_regs(&newregs)) {
			*regs = newregs;
			ret = 0;
		}
	}

	return ret;
}

/* ptrace_getfpregs()
 *
 * Get the child FPU state.
 */
static int ptrace_getfpregs(struct task_struct *tsk, void __user * ufpregs)
{
	return -EFAULT;
}

/*
 * Set the child FPU state.
 */
static int ptrace_setfpregs(struct task_struct *tsk, void __user * ufpregs)
{
	return -EFAULT;
}

/* do_ptrace()
 *
 * Provide ptrace defined service.
 */
long arch_ptrace(struct task_struct *child, long request, unsigned long addr,
		 unsigned long data)
{
	int ret;

	switch (request) {
	case PTRACE_PEEKUSR:
		ret =
		    ptrace_read_user(child, addr, (unsigned long __user *)data);
		break;

	case PTRACE_POKEUSR:
		ret = ptrace_write_user(child, addr, data);
		break;

	case PTRACE_GETREGS:
		ret = ptrace_getregs(child, (void __user *)data);
		break;

	case PTRACE_SETREGS:
		ret = ptrace_setregs(child, (void __user *)data);
		break;

	case PTRACE_GETFPREGS:
		ret = ptrace_getfpregs(child, (void __user *)data);
		break;

	case PTRACE_SETFPREGS:
		ret = ptrace_setfpregs(child, (void __user *)data);
		break;

	default:
		ret = ptrace_request(child, request, addr, data);
		break;
	}

	return ret;
}

void user_enable_single_step(struct task_struct *child)
{
	struct pt_regs *regs;
	regs = task_pt_regs(child);
	regs->ipsw |= PSW_mskHSS;
	set_tsk_thread_flag(child, TIF_SINGLESTEP);
}

void user_disable_single_step(struct task_struct *child)
{
	struct pt_regs *regs;
	regs = task_pt_regs(child);
	regs->ipsw &= ~PSW_mskHSS;
	clear_tsk_thread_flag(child, TIF_SINGLESTEP);
}

/* sys_trace()
 *
 * syscall trace handler.
 */

asmlinkage int syscall_trace_enter(int syscall, struct pt_regs *regs)
{
	if (test_thread_flag(TIF_SYSCALL_TRACE)) {
		if (tracehook_report_syscall_entry(regs))
			return -1;
	}
	return syscall;
}

asmlinkage void syscall_trace_leave(struct pt_regs *regs)
{
	int step = test_thread_flag(TIF_SINGLESTEP);
	if (step || test_thread_flag(TIF_SYSCALL_TRACE))
		tracehook_report_syscall_exit(regs, step);

}
