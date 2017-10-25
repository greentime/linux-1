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

#include <asm/page.h>
#include <asm/thread_info.h>
#include <asm/cache.h>
#include <asm/memory.h>

#define LOAD_OFFSET	(PAGE_OFFSET - PHYS_OFFSET)
#include <asm-generic/vmlinux.lds.h>

OUTPUT_ARCH(nds32)
ENTRY(_stext_lma)
jiffies = jiffies_64;

SECTIONS
{
	_stext_lma = TEXTADDR - LOAD_OFFSET;
	. = TEXTADDR;
	__init_begin = .;
	HEAD_TEXT_SECTION
	INIT_TEXT_SECTION(PAGE_SIZE)
	INIT_DATA_SECTION(16)
	PERCPU_SECTION(L1_CACHE_BYTES)
	__init_end = .;

	. = ALIGN(PAGE_SIZE);
	_stext = .;
	/* Real text segment */
	.text : AT(ADDR(.text) - LOAD_OFFSET) {
		_text = .;		/* Text and read-only data	*/
		TEXT_TEXT
		SCHED_TEXT
		CPUIDLE_TEXT
		LOCK_TEXT
		KPROBES_TEXT
		IRQENTRY_TEXT
		*(.fixup)
	}

	_etext = .;			/* End of text and rodata section */

	_sdata = .;
	RO_DATA_SECTION(PAGE_SIZE)
	RW_DATA_SECTION(L1_CACHE_BYTES, PAGE_SIZE, THREAD_SIZE)
	_edata  =  .;

	EXCEPTION_TABLE(16)
	NOTES
	BSS_SECTION(4, 4, 4)
	_end = .;

	STABS_DEBUG
	DWARF_DEBUG

	DISCARDS
}
