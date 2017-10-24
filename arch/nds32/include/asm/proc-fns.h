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

#ifndef __NDS32_PROCFNS_H__
#define __NDS32_PROCFNS_H__

#define CPU_NAME n13

#ifdef __KERNEL__

#ifdef __STDC__
#define ____cpu_fn(name,fn)       name##fn
#else
#define ____cpu_fn(name,fn)       name/**/fn
#endif
#define __cpu_fn(name,fn)         ____cpu_fn(name,fn)

#define cpu_proc_init			__cpu_fn( CPU_NAME, _proc_init)
#define cpu_proc_fin			__cpu_fn( CPU_NAME, _proc_fin)
#define cpu_do_idle			__cpu_fn( CPU_NAME, _do_idle)
#define cpu_reset			__cpu_fn( CPU_NAME, _reset)
#define cpu_switch_mm			__cpu_fn( CPU_NAME, _switch_mm)

#define cpu_dcache_inval_all		__cpu_fn( CPU_NAME, _dcache_inval_all)
#define cpu_dcache_wbinval_all		__cpu_fn( CPU_NAME, _dcache_wbinval_all)
#define cpu_dcache_inval_page		__cpu_fn( CPU_NAME, _dcache_inval_page)
#define cpu_dcache_wb_page		__cpu_fn( CPU_NAME, _dcache_wb_page)
#define cpu_dcache_wbinval_page		__cpu_fn( CPU_NAME, _dcache_wbinval_page)
#define cpu_dcache_inval_range		__cpu_fn( CPU_NAME, _dcache_inval_range)
#define cpu_dcache_wb_range		__cpu_fn( CPU_NAME, _dcache_wb_range)
#define cpu_dcache_wbinval_range	__cpu_fn( CPU_NAME, _dcache_wbinval_range)

#define cpu_icache_inval_all		__cpu_fn( CPU_NAME, _icache_inval_all)
#define cpu_icache_inval_page		__cpu_fn( CPU_NAME, _icache_inval_page)
#define cpu_icache_inval_range		__cpu_fn( CPU_NAME, _icache_inval_range)

#define cpu_cache_wbinval_page		__cpu_fn( CPU_NAME, _cache_wbinval_page)
#define cpu_cache_wbinval_range		__cpu_fn( CPU_NAME, _cache_wbinval_range)
#define cpu_cache_wbinval_range_check	__cpu_fn( CPU_NAME, _cache_wbinval_range_check)

#define cpu_dma_wb_range		__cpu_fn( CPU_NAME, _dma_wb_range)
#define cpu_dma_inval_range		__cpu_fn( CPU_NAME, _dma_inval_range)
#define cpu_dma_wbinval_range		__cpu_fn( CPU_NAME, _dma_wbinval_range)

#include <asm/page.h>

struct mm_struct;
struct vm_area_struct;
extern void cpu_proc_init(void);
extern void cpu_proc_fin(void);
extern void cpu_do_idle(void);
extern void cpu_reset(unsigned long reset);
extern void cpu_switch_mm(struct mm_struct *mm);

extern void cpu_dcache_inval_all(void);
extern void cpu_dcache_wbinval_all(void);
extern void cpu_dcache_inval_page(unsigned long page);
extern void cpu_dcache_wb_page(unsigned long page);
extern void cpu_dcache_wbinval_page(unsigned long page);
extern void cpu_dcache_inval_range(unsigned long start, unsigned long end);
extern void cpu_dcache_wb_range(unsigned long start, unsigned long end);
extern void cpu_dcache_wbinval_range(unsigned long start, unsigned long end);

extern void cpu_icache_inval_all(void);
extern void cpu_icache_inval_page(unsigned long page);
extern void cpu_icache_inval_range(unsigned long start, unsigned long end);

extern void cpu_cache_wbinval_page(unsigned long page, int flushi);
extern void cpu_cache_wbinval_range(unsigned long start,
				    unsigned long end, int flushi);
extern void cpu_cache_wbinval_range_check(struct vm_area_struct *vma,
					  unsigned long start,
					  unsigned long end, bool flushi,
					  bool wbd);

extern void cpu_dma_wb_range(unsigned long start, unsigned long end);
extern void cpu_dma_inval_range(unsigned long start, unsigned long end);
extern void cpu_dma_wbinval_range(unsigned long start, unsigned long end);

#endif /* __KERNEL__ */
#endif /* __NDS32_PROCFNS_H__ */
