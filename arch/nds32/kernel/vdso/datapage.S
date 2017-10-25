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

#include <linux/linkage.h>
#include <asm/page.h>

ENTRY(__get_timerpage)
	sethi	$r0, hi20(. + PAGE_SIZE + 8)
	ori	$r0, $r0, lo12(. + PAGE_SIZE + 4)
	mfusr	$r1, $pc
	sub	$r0, $r1, $r0
	ret
ENDPROC(__get_timerpage)

ENTRY(__get_datapage)
	sethi	$r0, hi20(. + 2*PAGE_SIZE + 8)
	ori	$r0, $r0, lo12(. + 2*PAGE_SIZE + 4)
	mfusr	$r1, $pc
	sub	$r0, $r1, $r0
	ret
ENDPROC(__get_datapage)
