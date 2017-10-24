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

#ifndef ASMNDS32_DMA_MAPPING_H
#define ASMNDS32_DMA_MAPPING_H

extern struct dma_map_ops nds32_dma_ops;

static inline struct dma_map_ops *get_arch_dma_ops(struct bus_type *bus)
{
	return &nds32_dma_ops;
}

#endif
