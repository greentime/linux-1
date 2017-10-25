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

#ifndef __UAPI_ASM_NDS32_PTRACE_H
#define __UAPI_ASM_NDS32_PTRACE_H

#ifndef __ASSEMBLY__

/*
 * User structures for general purpose register.
 */
struct user_pt_regs {
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
#endif
#endif
