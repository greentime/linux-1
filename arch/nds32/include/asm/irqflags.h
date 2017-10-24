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

#include <asm/nds32.h>
#include <nds32_intrinsic.h>

#define arch_local_irq_disable()	\
	GIE_DISABLE();

#define arch_local_irq_enable()	\
	GIE_ENABLE();
static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags;
	flags = __nds32__mfsr(NDS32_SR_PSW) & PSW_mskGIE;
	GIE_DISABLE();
	return flags;
}

static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;
	flags = __nds32__mfsr(NDS32_SR_PSW) & PSW_mskGIE;
	return flags;
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	if(flags)
		GIE_ENABLE();
}

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return !flags;
}
