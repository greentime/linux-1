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

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/swap.h>
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/mman.h>
#include <linux/nodemask.h>
#include <linux/initrd.h>
#include <linux/highmem.h>
#include <linux/memblock.h>

#include <asm/sections.h>
#include <asm/setup.h>
#include <asm/tlb.h>
#include <asm/page.h>

DEFINE_PER_CPU(struct mmu_gather, mmu_gathers);
DEFINE_SPINLOCK(anon_alias_lock);
extern pgd_t swapper_pg_dir[PTRS_PER_PGD];
extern unsigned long phys_initrd_start;
extern unsigned long phys_initrd_size;

/*
 * empty_zero_page is a special page that is used for
 * zero-initialized data and COW.
 */
struct page *empty_zero_page;
#ifdef CONFIG_EARLY_PRINTK
#include <asm/fixmap.h>

/*
 * Using tlbop to create an early I/O mapping
 */
void __iomem *__init early_io_map(phys_addr_t pa)
{
	unsigned long va;
	pa &= PAGE_MASK;
	pa += pgprot_val(PAGE_DEVICE);
	va = __fix_to_virt(FIX_EARLY_DEBUG);
	/* insert and lock this page to tlb entry directly */

	__nds32__mtsr_dsb(va, NDS32_SR_TLB_VPN);
	__nds32__tlbop_rwlk(pa);
	__nds32__isb();
	return (void __iomem *)va;
}

int __init early_io_unmap(void)
{
	unsigned long va;
	va = __fix_to_virt(FIX_EARLY_DEBUG);
	__nds32__tlbop_unlk(va);
	__nds32__tlbop_inv(va);
	__nds32__isb();
	return 0;
}

late_initcall(early_io_unmap);
#endif

static void __init zone_sizes_init(void)
{
	unsigned long zones_size[MAX_NR_ZONES];

	/* Clear the zone sizes */
	memset(zones_size, 0, sizeof(zones_size));

	zones_size[ZONE_NORMAL] = max_low_pfn;
#ifdef CONFIG_HIGHMEM
	zones_size[ZONE_HIGHMEM] = max_pfn;
#endif
	free_area_init_nodes(zones_size);

}

/*
 * Map all physical memory under high_memory into kernel's address space.
 *
 * This is explicitly coded for two-level page tables, so if you need
 * something else then this needs to change.
 */
static void __init map_ram(void)
{
	unsigned long v, p, e;
	pgd_t *pge;
	pud_t *pue;
	pmd_t *pme;
	pte_t *pte;
	/* These mark extents of read-only kernel pages...
	 * ...from vmlinux.lds.S
	 */

	p = (u32) memblock_start_of_DRAM() & PAGE_MASK;
	e = min((u32) memblock_end_of_DRAM(), (u32) __pa(high_memory));

	v = (u32) __va(p);
	pge = pgd_offset_k(v);

	while (p < e) {
		int j;
		pue = pud_offset(pge, v);
		pme = pmd_offset(pue, v);

		if ((u32) pue != (u32) pge || (u32) pme != (u32) pge) {
			panic("%s: Kernel hardcoded for "
			      "two-level page tables", __func__);
		}

		/* Alloc one page for holding PTE's... */
		pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
		set_pmd(pme, __pmd(__pa(pte) + _PAGE_KERNEL_TABLE));

		/* Fill the newly allocated page with PTE'S */
		for (j = 0; p < e && j < PTRS_PER_PTE;
		     v += PAGE_SIZE, p += PAGE_SIZE, j++, pte++) {
			/* Create mapping between p and v. */
			/* TODO: more fine grant for page access permission */
			set_pte(pte, __pte(p + pgprot_val(PAGE_KERNEL)));
		}

		pge++;
	}
}

static void __init fixedrange_init(void)
{
	unsigned long vaddr;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	/*
	 * Fixed mappings:
	 */
	vaddr = __fix_to_virt(__end_of_fixed_addresses - 1);
	pgd = swapper_pg_dir + pgd_index(vaddr);
	pud = pud_offset(pgd, vaddr);
	pmd = pmd_offset(pud, vaddr);
	pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
	set_pmd(pmd, __pmd(__pa(pte) + _PAGE_KERNEL_TABLE));

#ifdef CONFIG_HIGHMEM
	/*
	 * Permanent kmaps:
	 */
	vaddr = PKMAP_BASE;

	pgd = swapper_pg_dir + pgd_index(vaddr);
	pud = pud_offset(pgd, vaddr);
	pmd = pmd_offset(pud, vaddr);
	pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
	set_pmd(pmd, __pmd(__pa(pte) + _PAGE_KERNEL_TABLE));
	pkmap_page_table = pte;
#endif /* CONFIG_HIGHMEM */
}

/*
 * paging_init() sets up the page tables, initialises the zone memory
 * maps, and sets up the zero page, bad page and bad page tables.
 */
void __init paging_init(void)
{
	void *zero_page;
	int i;

	pr_info("Setting up paging and PTEs.\n");
	/* clear out the init_mm.pgd that will contain the kernel's mappings */
	for (i = 0; i < PTRS_PER_PGD; i++)
		swapper_pg_dir[i] = __pgd(1);

	map_ram();

	fixedrange_init();

	/* allocate space for empty_zero_page */
	zero_page = alloc_bootmem_low_pages(PAGE_SIZE);
	memset(zero_page, 0, PAGE_SIZE);

	zone_sizes_init();

	empty_zero_page = virt_to_page(zero_page);
	flush_dcache_page(empty_zero_page);
}

static inline int free_area(unsigned long addr, unsigned long end, char *s)
{
	unsigned int size = (end - addr) >> 10;
	int pages = 0;

	for (; addr < end; addr += PAGE_SIZE) {
		struct page *page = virt_to_page(addr);
		ClearPageReserved(page);
		init_page_count(page);
		free_page(addr);
		totalram_pages++;
		pages++;
	}

	if (size && s)
		pr_info("free_area: Freeing %s memory: %dK\n", s, size);

	return pages;
}

static inline void __init free_highmem(void)
{
#ifdef CONFIG_HIGHMEM
	unsigned long pfn;
	for (pfn = PFN_UP(__pa(high_memory)); pfn < max_pfn; pfn++) {
		phys_addr_t paddr = (phys_addr_t) pfn << PAGE_SHIFT;
		if (!memblock_is_reserved(paddr))
			free_highmem_page(pfn_to_page(pfn));
	}
#endif
}

static void __init set_max_mapnr_init(void)
{
	max_mapnr = max_pfn;
}

/*
 * mem_init() marks the free areas in the mem_map and tells us how much
 * memory is free.  This is done after various parts of the system have
 * claimed their memory after the kernel image.
 */
void __init mem_init(void)
{
	phys_addr_t memory_start = memblock_start_of_DRAM();
	BUG_ON(!mem_map);
	set_max_mapnr_init();

	free_highmem();

	/* this will put all low memory onto the freelists */
	free_all_bootmem();

	pr_info("virtual kernel memory layout:\n"
		"    fixmap  : 0x%08lx - 0x%08lx   (%4ld kB)\n"
#ifdef CONFIG_HIGHMEM
		"    pkmap   : 0x%08lx - 0x%08lx   (%4ld kB)\n"
#endif
		"    consist : 0x%08lx - 0x%08lx   (%4ld MB)\n"
		"    vmalloc : 0x%08lx - 0x%08lx   (%4ld MB)\n"
		"    lowmem  : 0x%08lx - 0x%08lx   (%4ld MB)\n"
		"      .init : 0x%08lx - 0x%08lx   (%4ld kB)\n"
		"      .data : 0x%08lx - 0x%08lx   (%4ld kB)\n"
		"      .text : 0x%08lx - 0x%08lx   (%4ld kB)\n",
		FIXADDR_START, FIXADDR_TOP, (FIXADDR_TOP - FIXADDR_START) >> 10,
#ifdef CONFIG_HIGHMEM
		PKMAP_BASE, PKMAP_BASE + LAST_PKMAP * PAGE_SIZE,
		(LAST_PKMAP * PAGE_SIZE) >> 10,
#endif
		CONSISTENT_BASE, CONSISTENT_END,
		((CONSISTENT_END) - (CONSISTENT_BASE)) >> 20, VMALLOC_START,
		(unsigned long)VMALLOC_END, (VMALLOC_END - VMALLOC_START) >> 20,
		(unsigned long)__va(memory_start), (unsigned long)high_memory,
		((unsigned long)high_memory -
		 (unsigned long)__va(memory_start)) >> 20,
		(unsigned long)&__init_begin, (unsigned long)&__init_end,
		((unsigned long)&__init_end -
		 (unsigned long)&__init_begin) >> 10, (unsigned long)&_etext,
		(unsigned long)&_edata,
		((unsigned long)&_edata - (unsigned long)&_etext) >> 10,
		(unsigned long)&_text, (unsigned long)&_etext,
		((unsigned long)&_etext - (unsigned long)&_text) >> 10);

	/*
	 * Check boundaries twice: Some fundamental inconsistencies can
	 * be detected at build time already.
	 */
#ifdef CONFIG_HIGHMEM
	BUILD_BUG_ON(PKMAP_BASE + LAST_PKMAP * PAGE_SIZE > FIXADDR_START);
	BUILD_BUG_ON((CONSISTENT_END) > PKMAP_BASE);
#endif
	BUILD_BUG_ON(VMALLOC_END > CONSISTENT_BASE);
	BUILD_BUG_ON(VMALLOC_START >= VMALLOC_END);

#ifdef CONFIG_HIGHMEM
	BUG_ON(PKMAP_BASE + LAST_PKMAP * PAGE_SIZE > FIXADDR_START);
	BUG_ON(CONSISTENT_END > PKMAP_BASE);
#endif
	BUG_ON(VMALLOC_END > CONSISTENT_BASE);
	BUG_ON(VMALLOC_START >= VMALLOC_END);
	BUG_ON((unsigned long)high_memory > VMALLOC_START);

	return;
}

void free_initmem(void)
{
	free_initmem_default(-1);
}

#ifdef CONFIG_BLK_DEV_INITRD
static int keep_initrd;

void free_initrd_mem(unsigned long start, unsigned long end)
{
	if (!keep_initrd)
		free_area(start, end, "initrd");
}

static int __init keepinitrd_setup(char *__unused)
{
	keep_initrd = 1;
	return 1;
}

__setup("keepinitrd", keepinitrd_setup);
#endif
