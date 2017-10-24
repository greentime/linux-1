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

#ifndef _ASM_NDS32_CURRENT_H
#define _ASM_NDS32_CURRENT_H

#ifndef __ASSEMBLY__
register struct task_struct *current asm("$r25");
#endif /* __ASSEMBLY__ */
#define tsk $r25

#endif /* _ASM_NDS32_CURRENT_H */
