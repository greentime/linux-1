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
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/nds32.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#ifdef CONFIG_CACHE_L2
#include <asm/l2_cache.h>
#endif
#include <nds32_intrinsic.h>

#include <asm/cache_info.h>
extern struct cache_info L1_cache_info[2];

int va_kernel_present(unsigned long addr)
{
	pmd_t *pmd;
	pte_t *ptep, pte;

	pmd = pmd_offset(pgd_offset_k(addr), addr);
	if (!pmd_none(*pmd)) {
		ptep = pte_offset_map(pmd, addr);
		pte = *ptep;
		if (pte_present(pte))
			return pte;
	}
	return 0;
}

pte_t va_present(struct mm_struct * mm, unsigned long addr)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ptep, pte;

	pgd = pgd_offset(mm, addr);
	if (!pgd_none(*pgd)) {
		pud = pud_offset(pgd, addr);
		if (!pud_none(*pud)) {
			pmd = pmd_offset(pud, addr);
			if (!pmd_none(*pmd)) {
				ptep = pte_offset_map(pmd, addr);
				pte = *ptep;
				if (pte_present(pte))
					return pte;
			}
		}
	}
	return 0;

}

int va_readable(struct pt_regs *regs, unsigned long addr)
{
	struct mm_struct *mm = current->mm;
	pte_t pte;
	int ret = 0;

	if (user_mode(regs)) {
		/* user mode */
		pte = va_present(mm, addr);
		if (!pte && pte_read(pte))
			ret = 1;
	} else {
		/* superuser mode is always readable, so we can only
		 * check it is present or not*/
		return (! !va_kernel_present(addr));
	}
	return ret;
}

int va_writable(struct pt_regs *regs, unsigned long addr)
{
	struct mm_struct *mm = current->mm;
	pte_t pte;
	int ret = 0;

	if (user_mode(regs)) {
		/* user mode */
		pte = va_present(mm, addr);
		if (!pte && pte_write(pte))
			ret = 1;
	} else {
		/* superuser mode */
		pte = va_kernel_present(addr);
		if (!pte && pte_kernel_write(pte))
			ret = 1;
	}
	return ret;
}

/*
 * All
 */
void n13_icache_inval_all(void)
{
	unsigned long end, line_size;

	line_size = L1_cache_info[ICACHE].line_size;
	end =
	    line_size * L1_cache_info[ICACHE].ways * L1_cache_info[ICACHE].sets;

	do {
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_IX_INVAL"::"r" (end));
	} while (end > 0);
}

void n13_dcache_inval_all(void)
{
	__nds32__cctl_l1d_invalall();
}

void n13_dcache_wb_all(void)
{
#ifdef CONFIG_CACHE_L2
	if (atl2c_base)
		__nds32__cctl_l1d_wball_alvl();
	else
		__nds32__cctl_l1d_wball_one_lvl();
#else
	__nds32__cctl_l1d_wball_one_lvl();
#endif
}

void n13_dcache_wbinval_all(void)
{
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	unsigned long flags;
#endif

#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	local_irq_save(flags);
#endif
#ifdef CONFIG_CACHE_L2
	if (atl2c_base)
		__nds32__cctl_l1d_wball_alvl();
	else
		__nds32__cctl_l1d_wball_one_lvl();
#else
	__nds32__cctl_l1d_wball_one_lvl();
#endif
	__nds32__cctl_l1d_invalall();
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	local_irq_restore(flags);
#endif
}

/*
 * Page
 */
void n13_icache_inval_page(unsigned long start)
{
	unsigned long line_size, end;

	line_size = L1_cache_info[ICACHE].line_size;
	end = start + PAGE_SIZE;

	do {
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_VA_INVAL"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_VA_INVAL"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_VA_INVAL"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1I_VA_INVAL"::"r" (end));
	} while (end != start);
}

void n13_dcache_inval_page(unsigned long start)
{
	unsigned long line_size, end;

	line_size = L1_cache_info[DCACHE].line_size;
	end = start + PAGE_SIZE;

	do {
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL"::"r" (end));
	} while (end != start);
}

void n13_dcache_wb_page(unsigned long start)
{
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	unsigned long line_size, end;

	line_size = L1_cache_info[DCACHE].line_size;
	end = start + PAGE_SIZE;

	do {
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB"::"r" (end));
		end -= line_size;
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB"::"r" (end));
	} while (end != start);
#endif
}

void n13_dcache_wbinval_page(unsigned long start)
{
	unsigned long line_size, end;

	line_size = L1_cache_info[DCACHE].line_size;
	end = start + PAGE_SIZE;

	do {
		end -= line_size;
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB"::"r" (end));
#endif
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL"::"r" (end));
		end -= line_size;
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB"::"r" (end));
#endif
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL"::"r" (end));
		end -= line_size;
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB"::"r" (end));
#endif
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL"::"r" (end));
		end -= line_size;
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB"::"r" (end));
#endif
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL"::"r" (end));
	} while (end != start);
}

void n13_cache_wbinval_page(unsigned long page, int flushi)
{
	n13_dcache_wbinval_page(page);
	if (flushi)
		n13_icache_inval_page(page);
}

/*
 * Range
 */
void n13_icache_inval_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;

	line_size = L1_cache_info[ICACHE].line_size;

	while (end > start) {
		__asm__ volatile ("\n\tcctl %0, L1I_VA_INVAL"::"r" (start));
		start += line_size;
	}
}

void n13_dcache_inval_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;

	line_size = L1_cache_info[DCACHE].line_size;

	while (end > start) {
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL"::"r" (start));
		start += line_size;
	}
}

void n13_dcache_wb_range(unsigned long start, unsigned long end)
{
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	unsigned long line_size;

	line_size = L1_cache_info[DCACHE].line_size;

	while (end > start) {
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB"::"r" (start));
		start += line_size;
	}
#endif
}

void n13_dcache_wbinval_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;

	line_size = L1_cache_info[DCACHE].line_size;

	while (end > start) {
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
		__asm__ volatile ("\n\tcctl %0, L1D_VA_WB"::"r" (start));
#endif
		__asm__ volatile ("\n\tcctl %0, L1D_VA_INVAL"::"r" (start));
		start += line_size;
	}
}

void n13_cache_wbinval_range(unsigned long start, unsigned long end, int flushi)
{
	unsigned long line_size, align_start, align_end;

	line_size = L1_cache_info[DCACHE].line_size;
	align_start = start & ~(line_size - 1);
	align_end = (end + line_size - 1) & ~(line_size - 1);
	n13_dcache_wbinval_range(align_start, align_end);

	if (flushi) {
		line_size = L1_cache_info[ICACHE].line_size;
		align_start = start & ~(line_size - 1);
		align_end = (end + line_size - 1) & ~(line_size - 1);
		n13_icache_inval_range(align_start, align_end);
	}
}

void n13_cache_wbinval_range_check(struct vm_area_struct *vma,
				   unsigned long start, unsigned long end,
				   bool flushi, bool wbd)
{
	unsigned long line_size, t_start, t_end;

	if (!flushi && !wbd)
		return;
	line_size = L1_cache_info[DCACHE].line_size;
	start = start & ~(line_size - 1);
	end = (end + line_size - 1) & ~(line_size - 1);

	if ((end - start) > (8 * PAGE_SIZE)) {
		if (wbd)
			n13_dcache_wbinval_all();
		if (flushi)
			n13_icache_inval_all();
		return;
	}

	t_start = (start + PAGE_SIZE) & PAGE_MASK;
	t_end = ((end - 1) & PAGE_MASK);

	if ((start & PAGE_MASK) == t_end) {
		if (va_present(vma->vm_mm, start)) {
			if (wbd)
				n13_dcache_wbinval_range(start, end);
			if (flushi)
				n13_icache_inval_range(start, end);
		}
		return;
	}

	if (va_present(vma->vm_mm, start)) {
		if (wbd)
			n13_dcache_wbinval_range(start, end);
		if (flushi)
			n13_icache_inval_range(start, end);
	}

	if (va_present(vma->vm_mm, end - 1)) {
		if (wbd)
			n13_dcache_wbinval_range(start, end);
		if (flushi)
			n13_icache_inval_range(start, end);
	}

	while (t_start < t_end) {
		if (va_present(vma->vm_mm, t_start)) {
			if (wbd)
				n13_dcache_wbinval_page(t_start);
			if (flushi)
				n13_icache_inval_page(t_start);
		}
		t_start += PAGE_SIZE;
	}
}

/*
 * DMA
 */
void n13_dma_wb_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;
	unsigned long flags;
	line_size = L1_cache_info[DCACHE].line_size;
	start = start & (~(line_size - 1));
	end = (end + line_size - 1) & (~(line_size - 1));
	if (unlikely(start == end))
		return;

	local_irq_save(flags);
	n13_dcache_wb_range(start, end);

#ifdef CONFIG_CACHE_L2
	if (atl2c_base) {
		unsigned long p_start = __pa(start);
		unsigned long p_end = __pa(end);
		unsigned long cmd;
		/* TODO Can Use PAGE Mode to optimize if range large than PAGE_SIZE */
		line_size = L2_CACHE_LINE_SIZE();
		p_start = p_start & (~(line_size - 1));
		p_end = (p_end + line_size - 1) & (~(line_size - 1));
		cmd =
		    (p_start & ~(line_size - 1)) | CCTL_CMD_L2_PA_WB |
		    CCTL_SINGLE_CMD;
		do {
			L2_CMD_RDY();
			L2C_W_REG(L2_CCTL_CMD_OFF, cmd);
			cmd += line_size;
			p_start += line_size;
		} while (p_end > p_start);
		cmd = CCTL_CMD_L2_SYNC;
		L2_CMD_RDY();
		L2C_W_REG(L2_CCTL_CMD_OFF, cmd);
		L2_CMD_RDY();

	}
#endif
	local_irq_restore(flags);
}

#ifdef CONFIG_CACHE_L2
void n13_l2dcache_wbinval_range(unsigned long start, unsigned long end)
{
	unsigned long p_start;
	unsigned long p_end;
	unsigned long cmd;
	unsigned long line_size;

	p_start = __pa(start);
	p_end = __pa(end);
	/* TODO Can Use PAGE Mode to optimize if range large than PAGE_SIZE */
	line_size = L2_CACHE_LINE_SIZE();
	p_start = p_start & (~(line_size - 1));
	p_end = (p_end + line_size - 1) & (~(line_size - 1));
	cmd =
	    (p_start & ~(line_size - 1)) | CCTL_CMD_L2_PA_WBINVAL |
	    CCTL_SINGLE_CMD;
	do {
		L2_CMD_RDY();
		L2C_W_REG(L2_CCTL_CMD_OFF, cmd);
		cmd += line_size;
		p_start += line_size;
	} while (p_end > p_start);
	cmd = CCTL_CMD_L2_SYNC;
	L2_CMD_RDY();
	L2C_W_REG(L2_CCTL_CMD_OFF, cmd);
	L2_CMD_RDY();

}
#endif

void n13_dma_inval_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;
	unsigned long old_start = start;
	unsigned long old_end = end;
	unsigned long flags;
	line_size = L1_cache_info[DCACHE].line_size;
	start = start & (~(line_size - 1));
	end = (end + line_size - 1) & (~(line_size - 1));
	if (unlikely(start == end))
		return;
	local_irq_save(flags);
	if (start != old_start) {
		n13_dcache_wbinval_range(start, start + line_size);
#ifdef CONFIG_CACHE_L2
		if (atl2c_base)
			n13_l2dcache_wbinval_range(start, start + line_size);
#endif
	}
	if (end != old_end) {
		n13_dcache_wbinval_range(end - line_size, end);
#ifdef CONFIG_CACHE_L2
		if (atl2c_base)
			n13_l2dcache_wbinval_range(end - line_size, end);
#endif
	}
	n13_dcache_inval_range(start, end);
#ifdef CONFIG_CACHE_L2
	if (atl2c_base) {
		unsigned long p_start = __pa(start);
		unsigned long p_end = __pa(end);
		unsigned long cmd;
		/* TODO Can Use PAGE Mode to optimize if range large than PAGE_SIZE */
		line_size = L2_CACHE_LINE_SIZE();
		p_start = p_start & (~(line_size - 1));
		p_end = (p_end + line_size - 1) & (~(line_size - 1));
		cmd =
		    (p_start & ~(line_size - 1)) | CCTL_CMD_L2_PA_INVAL |
		    CCTL_SINGLE_CMD;
		do {
			L2_CMD_RDY();
			L2C_W_REG(L2_CCTL_CMD_OFF, cmd);
			cmd += line_size;
			p_start += line_size;
		} while (p_end > p_start);
		cmd = CCTL_CMD_L2_SYNC;
		L2_CMD_RDY();
		L2C_W_REG(L2_CCTL_CMD_OFF, cmd);
		L2_CMD_RDY();
	}
#endif
	local_irq_restore(flags);

}

void n13_dma_wbinval_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;
	unsigned long flags;
	line_size = L1_cache_info[DCACHE].line_size;
	start = start & (~(line_size - 1));
	end = (end + line_size - 1) & (~(line_size - 1));
	if (unlikely(start == end))
		return;

	local_irq_save(flags);
	n13_dcache_wbinval_range(start, end);
#ifdef CONFIG_CACHE_L2
	if (atl2c_base) {
		unsigned long p_start = __pa(start);
		unsigned long p_end = __pa(end);
		unsigned long cmd;

		/* TODO Can Use PAGE Mode to optimize if range large than PAGE_SIZE */
		line_size = L2_CACHE_LINE_SIZE();
		p_start = p_start & (~(line_size - 1));
		p_end = (p_end + line_size - 1) & (~(line_size - 1));
		cmd =
		    (p_start & ~(line_size - 1)) | CCTL_CMD_L2_PA_WBINVAL |
		    CCTL_SINGLE_CMD;
		do {
			L2_CMD_RDY();
			L2C_W_REG(L2_CCTL_CMD_OFF, cmd);
			cmd += line_size;
			p_start += line_size;
		} while (p_end > p_start);
		cmd = CCTL_CMD_L2_SYNC;
		L2_CMD_RDY();
		L2C_W_REG(L2_CCTL_CMD_OFF, cmd);
		L2_CMD_RDY();

	}
#endif
	local_irq_restore(flags);
}

void n13_proc_init(void)
{
}

void n13_proc_fin(void)
{
}

void n13_do_idle(void)
{
	__nds32__standby_no_wake_grant();
}

void n13_reset(unsigned long reset)
{
	u32 tmp;
	GIE_DISABLE();
	tmp = __nds32__mfsr(NDS32_SR_CACHE_CTL);
	tmp &= ~(CACHE_CTL_mskIC_EN | CACHE_CTL_mskDC_EN);
	__nds32__mtsr_isb(tmp, NDS32_SR_CACHE_CTL);
	n13_dcache_wbinval_all();
	n13_icache_inval_all();

	__asm__ __volatile__("jr.toff %0\n\t"::"r"(reset));
}

void n13_switch_mm(struct mm_struct *mm)
{
	unsigned long cid;
	cid = __nds32__mfsr(NDS32_SR_TLB_MISC);
	cid = (cid & ~TLB_MISC_mskCID) | mm->context.id;
	__nds32__mtsr_dsb(cid, NDS32_SR_TLB_MISC);
	__nds32__mtsr_isb(__pa(mm->pgd), NDS32_SR_L1_PPTB);
}
