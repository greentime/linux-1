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

#ifndef _ASMNDS32_SHMPARAM_H
#define _ASMNDS32_SHMPARAM_H

/*
 * This should be the size of the virtually indexed cache/ways,
 * whichever is greater since the cache aliases every size/ways
 * bytes.
 */
#define	SHMLBA	(4 * PAGE_SIZE)	/* attach addr a multiple of this */
#define	REALSHMLBA	SHMLBA

/*
 * Enforce SHMLBA in shmat
 */
#define __ARCH_FORCE_SHMLBA

#endif /* _ASMNDS32_SHMPARAM_H */
