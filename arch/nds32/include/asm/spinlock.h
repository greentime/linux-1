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

#ifndef __ASM_SPINLOCK_H
#define __ASM_SPINLOCK_H

#include <asm/processor.h>

#define arch_spin_is_locked(x)		((x)->lock != 0)

#define arch_spin_unlock_wait(lock) \
	do { while (arch_spin_is_locked(lock)) cpu_relax(); } while (0)

#define arch_spin_lock_flags(lock, flags) arch_spin_lock(lock)

static inline void arch_spin_lock(arch_spinlock_t * lock)
{
	unsigned long tmp;

	__asm__ __volatile__("1:\n"
			     "\tllw\t%0, [%1]\n"
			     "\tbnez\t%0, 1b\n"
			     "\tmovi\t%0, #0x1\n"
			     "\tscw\t%0, [%1]\n"
			     "\tbeqz\t%0, 1b\n"
			     :"=&r"(tmp)
			     :"r"(&lock->lock)
			     :"memory");
}

static inline int arch_spin_trylock(arch_spinlock_t * lock)
{
	unsigned long ret, tmp;

	__asm__ __volatile__("1:\n"
			     "\tllw\t%0, [%2]\n"
			     "\tmovi\t%1, #0x1\n"
			     "\tscw\t%1, [%2]\n"
			     "\tbeqz\t%1, 1b\n"
			     :"=&r"(ret), "=&r"(tmp)
			     :"r"(&lock->lock)
			     :"memory");

	return ret == 0;
}

static inline void arch_spin_unlock(arch_spinlock_t * lock)
{
	__asm__ __volatile__("xor\t$r15,  $r15, $r15\n"
			     "\tswi\t$r15, [%0]\n"
			     :
			     :"r"(&lock->lock)
			     :"memory");
}

static inline void arch_write_lock(arch_rwlock_t * rw)
{
	unsigned long tmp;

	__asm__ __volatile__("1:\n"
			     "\tllw\t%0, [%1]\n"
			     "\tbnez\t%0, 1b\n"
			     "\tsethi\t%0, 0x80000\n"
			     "\tscw\t%0, [%1]\n"
			     "\tbeqz\t%0, 1b\n"
			     :"=&r"(tmp)
			     :"r"(&rw->lock)
			     :"memory");
}

static inline void arch_write_unlock(arch_rwlock_t * rw)
{
	__asm__ __volatile__("xor\t$r15, $r15, $r15\n"
			     "\tswi\t$r15, [%0]\n"
			     :
			     :"r"(&rw->lock)
			     :"memory","$r15");
}

#define arch_write_can_lock(x)		((x)->lock == 0)
static inline void arch_read_lock(arch_rwlock_t * rw)
{
	int tmp;

	__asm__ __volatile__("1:\n"
			     "\tllw\t%0, [%1]\n"
			     "\tbltz\t%0, 1b\n"
			     "\taddi\t%0, %0, #1\n"
			     "\tscw\t%0, [%1]\n"
			     "\tbeqz\t%0, 1b\n"
			     :"=&r"(tmp)
			     :"r"(&rw->lock)
			     :"memory");
}

static inline void arch_read_unlock(arch_rwlock_t * rw)
{
	unsigned long tmp;

	__asm__ __volatile__("1:\n"
			     "\tllw\t%0, [%1]\n"
			     "\taddi\t%0, %0, #-1\n"
			     "\tscw\t%0, [%1]\n"
			     "\tbeqz\t%0, 1b\n"
			     :"=&r"(tmp)
			     :"r"(&rw->lock)
			     :"memory");
}

static inline int arch_read_trylock(arch_rwlock_t * rw)
{
	unsigned long ret, tmp;

	__asm__ __volatile__("\tmovi\t%0, #0x0\n"
			     "1:\n"
			     "\tllw\t%1, [%2]\n"
			     "\tbltz\t%1, 2f\n"
			     "\taddi\t%1, %1, #1\n"
			     "\tscw\t%1, [%2]\n"
			     "\tbeqz\t%1, 1b\n"
			     "\tmovi\t%0, #0x1\n"
			     "\tj\t3f\n"
			     "2:\n"
			     "\tscw\t%1, [%2]\n"
			     "3:\n"
			     :"=&r"(ret), "=&r"(tmp)
			     :"r"(&rw->lock)
			     :"memory");

	return ret;
}

static inline int arch_write_trylock(arch_rwlock_t * rw)
{
	unsigned long ret, tmp;

	__asm__ __volatile__("\tmovi\t%0, #0x0\n"
			     "1:\n"
			     "\tllw\t%1, [%2]\n"
			     "\tbnez\t%1, 2f\n"
			     "\tsethi\t%1, 0x80000\n"
			     "\tscw\t%1, [%2]\n"
			     "\tbeqz\t%1, 1b\n"
			     "\tmovi\t%0, #0x1\n"
			     "\tj\t3f\n"
			     "2:\n"
			     "\tscw\t%1, [%2]\n"
			     "3:\n"
			     :"=&r"(ret), "=&r"(tmp)
			     :"r"(&rw->lock)
			     :"memory");

	return ret;
}

#define arch_read_lock_flags(lock, flags) arch_read_lock(lock)
#define arch_write_lock_flags(lock, flags) arch_write_lock(lock)

#define arch_read_can_lock(x)	((x)->lock < 0x80000000)

#define arch_spin_relax(lock)	cpu_relax()
#define arch_read_relax(lock)	cpu_relax()
#define arch_write_relax(lock)	cpu_relax()

#endif /* __ASM_SPINLOCK_H */
