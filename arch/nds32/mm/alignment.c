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

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#include <asm/unaligned.h>

#ifdef CONFIG_PROC_FS
extern struct proc_dir_entry *proc_dir_cpu;
#endif

#define DEBUG(enable, tagged, ...)				\
	do{							\
		if (enable) {					\
			if (tagged)				\
			pr_warn("[ %30s() ] ", __func__);	\
			pr_warn(__VA_ARGS__);			\
		}						\
	} while (0)

#define RT(inst)	(((inst) >> 20) & 0x1FUL)
#define RA(inst)	(((inst) >> 15) & 0x1FUL)
#define RB(inst)	(((inst) >> 10) & 0x1FUL)
#define SV(inst)	(((inst) >> 8) & 0x3UL)
#define IMM(inst)	(((inst) >> 0) & 0x3FFFUL)

#define RA3(inst)	(((inst) >> 3) & 0x7UL)
#define RT3(inst)	(((inst) >> 6) & 0x7UL)
#define IMM3U(inst)	(((inst) >> 0) & 0x7UL)

#define RA5(inst)	(((inst) >> 0) & 0x1FUL)
#define RT4(inst)	(((inst) >> 5) & 0xFUL)

extern int (*do_unaligned_access)
 (unsigned long entry, unsigned long addr,
  unsigned long type, struct pt_regs * regs);
extern pte_t va_present(struct mm_struct *mm, unsigned long addr);
extern pte_t va_kernel_present(unsigned long addr);
extern int va_readable(struct pt_regs *regs, unsigned long addr);
extern int va_writable(struct pt_regs *regs, unsigned long addr);

static int mode = 0x3;
module_param(mode, int, S_IWUSR | S_IRUGO);

static inline unsigned long *idx_to_addr(struct pt_regs *regs, int idx)
{
	/* this should be consistent with ptrace.h */
	if (idx >= 0 && idx <= 25)	/* R0-R25 */
		return &regs->uregs[0] + idx;
	else if (idx >= 28 && idx <= 30)	/* FP, GP, LP */
		return &regs->fp + (idx - 28);
	else if (idx == 31)	/* SP */
		return &regs->sp;
	else
		return NULL;	/* cause a segfault */
}

static inline unsigned long get_inst(unsigned long addr)
{
	/* FIXME: consider 16-bit inst. */
	return be32_to_cpu(get_unaligned((u32 *) addr));
}

static inline unsigned long get_data(unsigned long addr, int len)
{
	if (len == 4)
		return get_unaligned((u32 *) addr);
	else
		return get_unaligned((u16 *) addr);
}

static inline void set_data(unsigned long addr, unsigned long val, int len)
{
	if (len == 4)
		put_unaligned(val, (u32 *) addr);
	else
		put_unaligned(val, (u16 *) addr);
}

static inline unsigned long sign_extend(unsigned long val, int len)
{
	unsigned long ret = 0;
	unsigned char *s, *t;
	int i = 0;

	val = cpu_to_le32(val);

	s = (void *)&val;
	t = (void *)&ret;

	while (i++ < len)
		*t++ = *s++;

	if (((*(t - 1)) & 0x80) && (i < 4)) {

		while (i++ <= 4)
			*t++ = 0xff;
	}

	return le32_to_cpu(ret);
}

static inline int do_16(unsigned long inst, struct pt_regs *regs)
{
	int imm, regular, load, len, addr_mode, idx_mode;
	unsigned long unaligned_addr, target_val, source_idx, target_idx,
	    shift = 0;
	switch ((inst >> 9) & 0x3F) {

	case 0x12:		/* LHI333    */
		imm = 1;
		regular = 1;
		load = 1;
		len = 2;
		addr_mode = 3;
		idx_mode = 3;
		break;
	case 0x10:		/* LWI333    */
		imm = 1;
		regular = 1;
		load = 1;
		len = 4;
		addr_mode = 3;
		idx_mode = 3;
		break;
	case 0x11:		/* LWI333.bi */
		imm = 1;
		regular = 0;
		load = 1;
		len = 4;
		addr_mode = 3;
		idx_mode = 3;
		break;
	case 0x1A:		/* LWI450    */
		imm = 0;
		regular = 1;
		load = 1;
		len = 4;
		addr_mode = 5;
		idx_mode = 4;
		break;
	case 0x16:		/* SHI333    */
		imm = 1;
		regular = 1;
		load = 0;
		len = 2;
		addr_mode = 3;
		idx_mode = 3;
		break;
	case 0x14:		/* SWI333    */
		imm = 1;
		regular = 1;
		load = 0;
		len = 4;
		addr_mode = 3;
		idx_mode = 3;
		break;
	case 0x15:		/* SWI333.bi */
		imm = 1;
		regular = 0;
		load = 0;
		len = 4;
		addr_mode = 3;
		idx_mode = 3;
		break;
	case 0x1B:		/* SWI450    */
		imm = 0;
		regular = 1;
		load = 0;
		len = 4;
		addr_mode = 5;
		idx_mode = 4;
		break;

	default:
		return -EFAULT;
	}

	if (addr_mode == 3) {
		unaligned_addr = *idx_to_addr(regs, RA3(inst));
		source_idx = RA3(inst);
	} else {
		unaligned_addr = *idx_to_addr(regs, RA5(inst));
		source_idx = RA5(inst);
	}

	if (idx_mode == 3)
		target_idx = RT3(inst);
	else
		target_idx = RT4(inst);

	if (imm)
		shift = IMM3U(inst) * len;

	if (regular)
		unaligned_addr += shift;
	else
		*idx_to_addr(regs, source_idx) = unaligned_addr + shift;

	if (load) {

		if (!va_readable(regs, unaligned_addr))
			return -EACCES;

		if (!access_ok(VERIFY_READ, (void *)unaligned_addr, len))
			return -EACCES;

		*idx_to_addr(regs, target_idx) = get_data(unaligned_addr, len);
	} else {

		if (!va_writable(regs, unaligned_addr))
			return -EACCES;

		if (!access_ok(VERIFY_WRITE, (void *)unaligned_addr, len))
			return -EACCES;

		target_val = *idx_to_addr(regs, target_idx);
		set_data(unaligned_addr, target_val, len);
	}

	regs->ipc += 2;

	return 0;
}

static inline int do_32(unsigned long inst, struct pt_regs *regs)
{
	int imm, regular, load, len, sign_ext;
	unsigned long unsligned_addr, target_val, shift;

	unsligned_addr = *idx_to_addr(regs, RA(inst));

	switch ((inst >> 25) << 1) {

	case 0x02:		/* LHI       */
		imm = 1;
		regular = 1;
		load = 1;
		len = 2;
		sign_ext = 0;
		break;
	case 0x0A:		/* LHI.bi    */
		imm = 1;
		regular = 0;
		load = 1;
		len = 2;
		sign_ext = 0;
		break;
	case 0x22:		/* LHSI      */
		imm = 1;
		regular = 1;
		load = 1;
		len = 2;
		sign_ext = 1;
		break;
	case 0x2A:		/* LHSI.bi   */
		imm = 1;
		regular = 0;
		load = 1;
		len = 2;
		sign_ext = 1;
		break;
	case 0x04:		/* LWI       */
		imm = 1;
		regular = 1;
		load = 1;
		len = 4;
		sign_ext = 0;
		break;
	case 0x0C:		/* LWI.bi    */
		imm = 1;
		regular = 0;
		load = 1;
		len = 4;
		sign_ext = 0;
		break;
	case 0x12:		/* SHI       */
		imm = 1;
		regular = 1;
		load = 0;
		len = 2;
		sign_ext = 0;
		break;
	case 0x1A:		/* SHI.bi    */
		imm = 1;
		regular = 0;
		load = 0;
		len = 2;
		sign_ext = 0;
		break;
	case 0x14:		/* SWI       */
		imm = 1;
		regular = 1;
		load = 0;
		len = 4;
		sign_ext = 0;
		break;
	case 0x1C:		/* SWI.bi    */
		imm = 1;
		regular = 0;
		load = 0;
		len = 4;
		sign_ext = 0;
		break;

	default:
		switch (inst & 0xff) {

		case 0x01:	/* LH        */
			imm = 0;
			regular = 1;
			load = 1;
			len = 2;
			sign_ext = 0;
			break;
		case 0x05:	/* LH.bi     */
			imm = 0;
			regular = 0;
			load = 1;
			len = 2;
			sign_ext = 0;
			break;
		case 0x11:	/* LHS       */
			imm = 0;
			regular = 1;
			load = 1;
			len = 2;
			sign_ext = 1;
			break;
		case 0x15:	/* LHS.bi    */
			imm = 0;
			regular = 0;
			load = 1;
			len = 2;
			sign_ext = 1;
			break;
		case 0x02:	/* LW        */
			imm = 0;
			regular = 1;
			load = 1;
			len = 4;
			sign_ext = 0;
			break;
		case 0x06:	/* LW.bi     */
			imm = 0;
			regular = 0;
			load = 1;
			len = 4;
			sign_ext = 0;
			break;
		case 0x09:	/* SH        */
			imm = 0;
			regular = 1;
			load = 0;
			len = 2;
			sign_ext = 0;
			break;
		case 0x0D:	/* SH.bi     */
			imm = 0;
			regular = 0;
			load = 0;
			len = 2;
			sign_ext = 0;
			break;
		case 0x0A:	/* SW        */
			imm = 0;
			regular = 1;
			load = 0;
			len = 4;
			sign_ext = 0;
			break;
		case 0x0E:	/* SW.bi     */
			imm = 0;
			regular = 0;
			load = 0;
			len = 4;
			sign_ext = 0;
			break;

		default:
			return -EFAULT;
		}
	}

	if (imm)
		shift = IMM(inst) * len;
	else
		shift = *idx_to_addr(regs, RB(inst)) << SV(inst);

	if (regular)
		unsligned_addr += shift;
	else
		*idx_to_addr(regs, RA(inst)) = unsligned_addr + shift;

	if (load) {

		if (!va_readable(regs, unsligned_addr))
			return -EACCES;

		if (!access_ok(VERIFY_READ, (void *)unsligned_addr, len))
			return -EACCES;

		if (sign_ext)
			*idx_to_addr(regs, RT(inst)) =
			    sign_extend(get_data(unsligned_addr, len), len);
		else
			*idx_to_addr(regs, RT(inst)) =
			    get_data(unsligned_addr, len);
	} else {

		if (!va_writable(regs, unsligned_addr))
			return -EACCES;

		if (!access_ok(VERIFY_WRITE, (void *)unsligned_addr, len))
			return -EACCES;

		target_val = *idx_to_addr(regs, RT(inst));
		set_data(unsligned_addr, target_val, len);
	}

	regs->ipc += 4;

	return 0;
}

static int _do_unaligned_access(unsigned long entry, unsigned long addr,
				unsigned long type, struct pt_regs *regs)
{
	unsigned long inst;
	int ret = -EFAULT;

	if (user_mode(regs)) {
		/* user mode */
		if (!va_present(current->mm, addr))
			return ret;
	} else {
		/* kernel mode */
		if (!va_kernel_present(addr))
			return ret;
	}

	inst = get_inst(regs->ipc);

	DEBUG(mode & 0x04, 1,
	      "Faulting Addr: 0x%08lx, PC: 0x%08lx [ 0x%08lx ]\n", addr,
	      regs->ipc, inst);

	if ((user_mode(regs) && (mode & 0x01))
	    || (!user_mode(regs) && (mode & 0x02))) {

		mm_segment_t seg = get_fs();

		set_fs(KERNEL_DS);

		if (inst & 0x80000000)
			ret = do_16((inst >> 16) & 0xffff, regs);
		else
			ret = do_32(inst, regs);

		set_fs(seg);
	}

	return ret;
}

#ifdef CONFIG_PROC_FS

static int proc_alignment_read(struct file *file, char __user * ubuf,
			       size_t count, loff_t * ppos)
{
	char p[80];
	int ret;

	ret = sprintf(p, "(0x01) User Mode: %s\n"
		      "(0x02) Kernel Mode: %s\n"
		      "(0x04) Warning: %s\n",
		      mode & 0x01 ? "on" : "off",
		      mode & 0x02 ? "on" : "off", mode & 0x04 ? "on" : "off");

	return simple_read_from_buffer(ubuf, count, ppos, p, ret);
}

#define INPUTLEN 12		/* '0' + 'x' + 8digit + '\n' + '\0' */

static ssize_t proc_alignment_write(struct file *file,
				    const char __user * ubuf, size_t count,
				    loff_t * ppos)
{
	unsigned long en;
	char *endp;
	char inbuf[INPUTLEN];

	if (count > INPUTLEN - 1)
		return -EFAULT;

	if (copy_from_user(inbuf, ubuf, count))
		return -EFAULT;

	inbuf[count - 1] = '\0';

	en = simple_strtoul(inbuf, &endp, 0);
	if (en > 0x07)
		return -EFAULT;

	mode = en & 0x7;

	return count;
}

static const struct file_operations fops = {
	.open = simple_open,
	.read = proc_alignment_read,
	.write = proc_alignment_write,
};
#endif /* CONFIG_PROC_FS */

static int __init unaligned_access_init(void)
{
#ifdef CONFIG_PROC_FS
	static struct proc_dir_entry *res_alignment;

	if (!proc_dir_cpu)
		if (!(proc_dir_cpu = proc_mkdir("cpu", NULL)))
			return -ENOMEM;

	if (!
	    (res_alignment =
	     proc_create("alignment", S_IWUSR | S_IRUGO, proc_dir_cpu, &fops)))
		return -ENOMEM;

#endif
	do_unaligned_access = _do_unaligned_access;

	return 0;
}

static void __exit unaligned_access_exit(void)
{
#ifdef CONFIG_PROC_FS
	remove_proc_entry("alignment", proc_dir_cpu);
#endif
	do_unaligned_access = NULL;
}

MODULE_DESCRIPTION("Unaligned Access Handler");
MODULE_LICENSE("GPL");

module_init(unaligned_access_init);
module_exit(unaligned_access_exit);
