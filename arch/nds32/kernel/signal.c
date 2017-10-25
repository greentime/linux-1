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

#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/ptrace.h>
#include <linux/personality.h>
#include <linux/freezer.h>
#include <linux/tracehook.h>
#include <linux/uaccess.h>

#include <asm/cacheflush.h>
#include <asm/ucontext.h>
#include <asm/unistd.h>

#include <asm/ptrace.h>
#include <asm/vdso.h>

struct rt_sigframe {
	struct siginfo info;
	struct ucontext uc;
};

static int restore_sigframe(struct pt_regs *regs,
			    struct rt_sigframe __user * sf)
{
	sigset_t set;
	int err;

	err = __copy_from_user(&set, &sf->uc.uc_sigmask, sizeof(set));
	if (err == 0) {
		set_current_blocked(&set);
	}

	__get_user_error(regs->uregs[0], &sf->uc.uc_mcontext.nds32_r0, err);
	__get_user_error(regs->uregs[1], &sf->uc.uc_mcontext.nds32_r1, err);
	__get_user_error(regs->uregs[2], &sf->uc.uc_mcontext.nds32_r2, err);
	__get_user_error(regs->uregs[3], &sf->uc.uc_mcontext.nds32_r3, err);
	__get_user_error(regs->uregs[4], &sf->uc.uc_mcontext.nds32_r4, err);
	__get_user_error(regs->uregs[5], &sf->uc.uc_mcontext.nds32_r5, err);
	__get_user_error(regs->uregs[6], &sf->uc.uc_mcontext.nds32_r6, err);
	__get_user_error(regs->uregs[7], &sf->uc.uc_mcontext.nds32_r7, err);
	__get_user_error(regs->uregs[8], &sf->uc.uc_mcontext.nds32_r8, err);
	__get_user_error(regs->uregs[9], &sf->uc.uc_mcontext.nds32_r9, err);
	__get_user_error(regs->uregs[10], &sf->uc.uc_mcontext.nds32_r10, err);
	__get_user_error(regs->uregs[11], &sf->uc.uc_mcontext.nds32_r11, err);
	__get_user_error(regs->uregs[12], &sf->uc.uc_mcontext.nds32_r12, err);
	__get_user_error(regs->uregs[13], &sf->uc.uc_mcontext.nds32_r13, err);
	__get_user_error(regs->uregs[14], &sf->uc.uc_mcontext.nds32_r14, err);
	__get_user_error(regs->uregs[15], &sf->uc.uc_mcontext.nds32_r15, err);
	__get_user_error(regs->uregs[16], &sf->uc.uc_mcontext.nds32_r16, err);
	__get_user_error(regs->uregs[17], &sf->uc.uc_mcontext.nds32_r17, err);
	__get_user_error(regs->uregs[18], &sf->uc.uc_mcontext.nds32_r18, err);
	__get_user_error(regs->uregs[19], &sf->uc.uc_mcontext.nds32_r19, err);
	__get_user_error(regs->uregs[20], &sf->uc.uc_mcontext.nds32_r20, err);

	__get_user_error(regs->uregs[21], &sf->uc.uc_mcontext.nds32_r21, err);
	__get_user_error(regs->uregs[22], &sf->uc.uc_mcontext.nds32_r22, err);
	__get_user_error(regs->uregs[23], &sf->uc.uc_mcontext.nds32_r23, err);
	__get_user_error(regs->uregs[24], &sf->uc.uc_mcontext.nds32_r24, err);
	__get_user_error(regs->uregs[25], &sf->uc.uc_mcontext.nds32_r25, err);
	__get_user_error(regs->fp, &sf->uc.uc_mcontext.nds32_fp, err);
	__get_user_error(regs->gp, &sf->uc.uc_mcontext.nds32_gp, err);
	__get_user_error(regs->lp, &sf->uc.uc_mcontext.nds32_lp, err);
	__get_user_error(regs->sp, &sf->uc.uc_mcontext.nds32_sp, err);
	__get_user_error(regs->ipc, &sf->uc.uc_mcontext.nds32_ipc, err);
#if defined(CONFIG_HWZOL)
	__get_user_error(regs->lc, &sf->uc.uc_mcontext.zol.nds32_lc, err);
	__get_user_error(regs->le, &sf->uc.uc_mcontext.zol.nds32_le, err);
	__get_user_error(regs->lb, &sf->uc.uc_mcontext.zol.nds32_lb, err);
#endif

	err |= !valid_user_regs(regs);

	return err;
}

asmlinkage int sys_rt_sigreturn(struct pt_regs *regs)
{
	struct rt_sigframe __user *frame;

	/* Always make any pending restarted system calls return -EINTR */
	current->restart_block.fn = do_no_restart_syscall;

	/*
	 * Since we stacked the signal on a 64-bit boundary,
	 * then 'sp' should be two-word aligned here.  If it's
	 * not, then the user is trying to mess with us.
	 */
	if (regs->sp & 7)
		goto badframe;

	frame = (struct rt_sigframe __user *)regs->sp;

	if (!access_ok(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;

	if (restore_sigframe(regs, frame))
		goto badframe;

	if (restore_altstack(&frame->uc.uc_stack))
		goto badframe;

	return regs->uregs[0];

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}

static int
setup_sigframe(struct rt_sigframe __user * sf, struct pt_regs *regs,
	       sigset_t * set)
{
	int err = 0;


	__put_user_error(regs->uregs[0], &sf->uc.uc_mcontext.nds32_r0, err);
	__put_user_error(regs->uregs[1], &sf->uc.uc_mcontext.nds32_r1, err);
	__put_user_error(regs->uregs[2], &sf->uc.uc_mcontext.nds32_r2, err);
	__put_user_error(regs->uregs[3], &sf->uc.uc_mcontext.nds32_r3, err);
	__put_user_error(regs->uregs[4], &sf->uc.uc_mcontext.nds32_r4, err);
	__put_user_error(regs->uregs[5], &sf->uc.uc_mcontext.nds32_r5, err);
	__put_user_error(regs->uregs[6], &sf->uc.uc_mcontext.nds32_r6, err);
	__put_user_error(regs->uregs[7], &sf->uc.uc_mcontext.nds32_r7, err);
	__put_user_error(regs->uregs[8], &sf->uc.uc_mcontext.nds32_r8, err);
	__put_user_error(regs->uregs[9], &sf->uc.uc_mcontext.nds32_r9, err);
	__put_user_error(regs->uregs[10], &sf->uc.uc_mcontext.nds32_r10, err);
	__put_user_error(regs->uregs[11], &sf->uc.uc_mcontext.nds32_r11, err);
	__put_user_error(regs->uregs[12], &sf->uc.uc_mcontext.nds32_r12, err);
	__put_user_error(regs->uregs[13], &sf->uc.uc_mcontext.nds32_r13, err);
	__put_user_error(regs->uregs[14], &sf->uc.uc_mcontext.nds32_r14, err);
	__put_user_error(regs->uregs[15], &sf->uc.uc_mcontext.nds32_r15, err);
	__put_user_error(regs->uregs[16], &sf->uc.uc_mcontext.nds32_r16, err);
	__put_user_error(regs->uregs[17], &sf->uc.uc_mcontext.nds32_r17, err);
	__put_user_error(regs->uregs[18], &sf->uc.uc_mcontext.nds32_r18, err);
	__put_user_error(regs->uregs[19], &sf->uc.uc_mcontext.nds32_r19, err);
	__put_user_error(regs->uregs[20], &sf->uc.uc_mcontext.nds32_r20, err);

	__put_user_error(regs->uregs[21], &sf->uc.uc_mcontext.nds32_r21, err);
	__put_user_error(regs->uregs[22], &sf->uc.uc_mcontext.nds32_r22, err);
	__put_user_error(regs->uregs[23], &sf->uc.uc_mcontext.nds32_r23, err);
	__put_user_error(regs->uregs[24], &sf->uc.uc_mcontext.nds32_r24, err);
	__put_user_error(regs->uregs[25], &sf->uc.uc_mcontext.nds32_r25, err);
	__put_user_error(regs->fp, &sf->uc.uc_mcontext.nds32_fp, err);
	__put_user_error(regs->gp, &sf->uc.uc_mcontext.nds32_gp, err);
	__put_user_error(regs->lp, &sf->uc.uc_mcontext.nds32_lp, err);
	__put_user_error(regs->sp, &sf->uc.uc_mcontext.nds32_sp, err);
	__put_user_error(regs->ipc, &sf->uc.uc_mcontext.nds32_ipc, err);
#if defined(CONFIG_HWZOL)
	__put_user_error(regs->lc, &sf->uc.uc_mcontext.zol.nds32_lc, err);
	__put_user_error(regs->le, &sf->uc.uc_mcontext.zol.nds32_le, err);
	__put_user_error(regs->lb, &sf->uc.uc_mcontext.zol.nds32_lb, err);
#endif

	__put_user_error(current->thread.trap_no, &sf->uc.uc_mcontext.trap_no,
			 err);
	__put_user_error(current->thread.error_code,
			 &sf->uc.uc_mcontext.error_code, err);
	__put_user_error(current->thread.address,
			 &sf->uc.uc_mcontext.fault_address, err);
	__put_user_error(set->sig[0], &sf->uc.uc_mcontext.oldmask, err);

	err |= __copy_to_user(&sf->uc.uc_sigmask, set, sizeof(*set));

	return err;
}

static inline void __user *get_sigframe(struct ksignal *ksig,
					struct pt_regs *regs, int framesize)
{
	unsigned long sp = sigsp(regs->sp, ksig);
	void __user *frame;

	/*
	 * ATPCS B01 mandates 8-byte alignment
	 */
	frame = (void __user *)((sp - framesize) & ~7);

	/*
	 * Check that we can actually write to the signal frame.
	 */
	if (!access_ok(VERIFY_WRITE, frame, framesize))
		frame = NULL;

	return frame;
}

static int
setup_return(struct pt_regs *regs, struct ksignal *ksig, void __user * frame)
{
	unsigned long handler = (unsigned long)ksig->ka.sa.sa_handler;
	unsigned long retcode;

	/*
	 * Maybe we need to deliver a 32-bit signal to a 26-bit task.
	 */
	if (ksig->ka.sa.sa_flags & SA_RESTORER)
		retcode = (unsigned long)ksig->ka.sa.sa_restorer;
	else
		retcode = VDSO_SYMBOL(current->mm->context.vdso, rt_sigtramp);

	regs->uregs[0] = ksig->sig;
	regs->sp = (unsigned long)frame;
	regs->lp = retcode;
	regs->ipc = handler;
	/* Also store handler address in r15 for updating GP in the handler. */
	regs->uregs[15] = handler;

	return 0;
}

static int
setup_rt_frame(struct ksignal *ksig, sigset_t * set, struct pt_regs *regs)
{
	struct rt_sigframe __user *frame =
	    get_sigframe(ksig, regs, sizeof(*frame));
	int err = 0;

	if (!frame)
		return 1;

	err |= copy_siginfo_to_user(&frame->info, &ksig->info);

	__put_user_error(0, &frame->uc.uc_flags, err);
	__put_user_error(NULL, &frame->uc.uc_link, err);

	err |= __save_altstack(&frame->uc.uc_stack, regs->sp);
	err |= setup_sigframe(frame, regs, set);
	if (err == 0) {
		setup_return(regs, ksig, frame);

		if (err == 0) {
			/*
			 * For realtime signals we must also set the second and third
			 * arguments for the signal handler.
			 *   -- Peter Maydell <pmaydell@chiark.greenend.org.uk> 2000-12-06
			 */
			regs->uregs[1] = (unsigned long)&frame->info;
			regs->uregs[2] = (unsigned long)&frame->uc;
		}
	}
	return err;
}

/*
 * OK, we're invoking a handler
 */
static void handle_signal(struct ksignal *ksig, struct pt_regs *regs)
{
	int ret;
	sigset_t *oldset = sigmask_to_save();

	/*
	 * Set up the stack frame
	 */
	ret = setup_rt_frame(ksig, oldset, regs);

	/*
	 * Check that the resulting registers are actually sane.
	 */
	ret |= !valid_user_regs(regs);

	signal_setup_done(ret, ksig, 0);
}

/*
 * Note that 'init' is a special process: it doesn't get signals it doesn't
 * want to handle. Thus you cannot kill init even with a SIGKILL even by
 * mistake.
 *
 * Note that we go through the signals twice: once to check the signals that
 * the kernel can handle, and then we build all the user-level signal handling
 * stack-frames in one go after that.
 */
static int do_signal(struct pt_regs *regs, int syscall)
{
	unsigned int retval = 0, continue_addr = 0, restart_addr = 0;
	struct ksignal ksig;
	int restart = 0;

	/*
	 * We want the common case to go fast, which
	 * is why we may in certain cases get here from
	 * kernel mode. Just return without doing anything
	 * if so.
	 */
	if (!user_mode(regs))
		return 0;

	/*
	 * If we were from a system call, check for system call restarting...
	 */
	if (syscall) {
		continue_addr = regs->ipc;
		restart_addr = continue_addr - 4;
		retval = regs->uregs[0];

		/*
		 * Prepare for system call restart.  We do this here so that a
		 * debugger will see the already changed PSW.
		 */
		switch (retval) {
		case -ERESTART_RESTARTBLOCK:
			restart -= 2;
		case -ERESTARTNOHAND:
		case -ERESTARTSYS:
		case -ERESTARTNOINTR:
			restart++;
			regs->uregs[0] = regs->orig_r0;
			regs->ipc = restart_addr;
			break;
		}
	}

	/*
	 * Get the signal to deliver.  When running under ptrace, at this
	 * point the debugger may change all our registers ...
	 */
	/*
	 * Depending on the signal settings we may need to revert the
	 * decision to restart the system call.  But skip this if a
	 * debugger has chosen to restart at a different PC.
	 */
	if (get_signal(&ksig)) {
		if (unlikely(restart) && regs->ipc == restart_addr) {
			if (retval == -ERESTARTNOHAND ||
			    retval == -ERESTART_RESTARTBLOCK
			    || (retval == -ERESTARTSYS
				&& !(ksig.ka.sa.sa_flags & SA_RESTART))) {
				regs->uregs[0] = -EINTR;
				regs->ipc = continue_addr;
			}
		}
		handle_signal(&ksig, regs);
	} else {
		restore_saved_sigmask();
		if (unlikely(restart) && regs->ipc == restart_addr) {
			regs->ipc = continue_addr;
			return restart;
		}
	}
	return 0;
}

asmlinkage int
do_notify_resume(struct pt_regs *regs, unsigned int thread_flags, int syscall)
{
	if (thread_flags & _TIF_SIGPENDING)
		return do_signal(regs, syscall);

	if (thread_flags & _TIF_NOTIFY_RESUME) {
		clear_thread_flag(TIF_NOTIFY_RESUME);
		tracehook_notify_resume(regs);
	}
	return 0;
}
