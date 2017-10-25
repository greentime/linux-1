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

#include <linux/syscalls.h>

#include <asm/uaccess.h>
#include <asm/cachectl.h>
#include <asm/proc-fns.h>

long sys_mmap2(unsigned long addr, unsigned long len,
	       unsigned long prot, unsigned long flags,
	       unsigned long fd, unsigned long pgoff)
{
	if (pgoff & (~PAGE_MASK >> 12))
		return -EINVAL;

	return sys_mmap_pgoff(addr, len, prot, flags, fd,
			      pgoff >> (PAGE_SHIFT - 12));
}

asmlinkage long sys_fadvise64_64_wrapper(int fd, int advice, loff_t offset,
					 loff_t len)
{
	return sys_fadvise64_64(fd, offset, len, advice);
}

int sys_cacheflush(unsigned int start, unsigned int end, int cache)
{
	struct vm_area_struct *vma;
	bool flushi = true, wbd = true;

	vma = find_vma(current->mm, start);
	if (!vma)
		return -EFAULT;
	switch (cache) {
	case ICACHE:
		wbd = false;
		break;
	case DCACHE:
		flushi = false;
		break;
	case BCACHE:
		break;
	default:
		return -EINVAL;
	}
	cpu_cache_wbinval_range_check(vma, start, end, flushi, wbd);

	return 0;
}
