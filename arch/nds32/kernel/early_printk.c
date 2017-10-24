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
#include <linux/console.h>
#include <linux/init.h>
#include <linux/io.h>

#include <linux/serial_reg.h>

extern void __iomem *early_io_map(phys_addr_t phys);
static void __iomem *early_base;
static void (*printch) (char ch);

/*
 * 8250/16550 (8-bit aligned registers) single character TX.
 */
static void uart8250_8bit_printch(char ch)
{
	while (!(readb(early_base + UART_LSR) & UART_LSR_THRE)) ;
	writeb(ch, early_base + UART_TX);
}

/*
 * 8250/16550 (32-bit aligned registers) single character TX.
 */
static void uart8250_32bit_printch(char ch)
{
	while (!(readl(early_base + (UART_LSR << 2)) & UART_LSR_THRE)) ;
	writel(ch, early_base + (UART_TX << 2));
}

struct earlycon_match {
	const char *name;
	void (*printch) (char ch);
};

static const struct earlycon_match earlycon_match[] __initconst = {
	{.name = "uart8250-8bit",.printch = uart8250_8bit_printch,},
	{.name = "uart8250-32bit",.printch = uart8250_32bit_printch,},
	{}
};

static void early_write(struct console *con, const char *s, unsigned n)
{
	while (n-- > 0) {
		if (*s == '\n')
			printch('\r');
		printch(*s);
		s++;
	}
}

static struct console early_console_dev = {
	.name = "earlycon",
	.write = early_write,
	.flags = CON_PRINTBUFFER | CON_BOOT,
	.index = -1,
};

/*
 * Parse earlyprintk=... parameter in the format:
 *
 *   <name>[,<addr>][,<options>]
 *
 * and register the early console. It is assumed that the UART has been
 * initialised by the bootloader already.
 */
static int __init setup_early_printk(char *buf)
{
	const struct earlycon_match *match = earlycon_match;
	phys_addr_t paddr = 0;

	if (!buf) {
		pr_warning("No earlyprintk arguments passed.\n");
		return 0;
	}

	while (match->name) {
		size_t len = strlen(match->name);
		if (!strncmp(buf, match->name, len)) {
			buf += len;
			break;
		}
		match++;
	}
	if (!match->name) {
		pr_warning("Unknown earlyprintk arguments: %s\n", buf);
		return 0;
	}

	/* I/O address */
	if (!strncmp(buf, ",0x", 3)) {
		char *e;
		paddr = simple_strtoul(buf + 1, &e, 16);
		buf = e;
	}

	if (paddr)
		early_base = early_io_map(paddr);

	if (!strcmp(CONFIG_NDS32_BUILTIN_DTB, "ae3xx"))
		early_base += 32;

	printch = match->printch;
	register_console(&early_console_dev);

	return 0;
}

early_param("earlyprintk", setup_early_printk);
