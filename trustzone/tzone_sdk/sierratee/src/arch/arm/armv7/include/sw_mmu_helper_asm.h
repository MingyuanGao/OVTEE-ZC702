/* 
 * OpenVirtualization: 
 * For additional details and support contact developer@sierraware.com.
 * Additional documentation can be found at www.openvirtualization.org
 * 
 * Copyright (C) 2011 SierraWare
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */


/* SW MMU HELPER HEADER TO INCLUDE IN ASM FILE */

#ifndef _SW_MMU_HELPER_ASM_H_
#define _SW_MMU_HELPER_ASM_H_

#define TTBR_FLAGS_C_BIT (0x1 << 0)
#define TTBR_FLAGS_S_BIT (0x1 << 1)
#define TTBR_FLAGS_RGN(val) ((val & 0x3) << 3)
#define TTBR_FLAGS_NOS_BIT (0x1 << 5)
#define TTBR_FLAGS_IRGN_BIT (0x1 << 6)

#ifdef CONFIG_SW_MULTICORE
/* 	
	Based on 32-bit TTBR0 format multiprocessing extensions
	Shareable, 
	outer shareable, 
	Normal memory - Outer Write-Back Write-allocate cacheable
	Normal memory - Inner Write-Back Write-Allocate Cacheable
	*/
#define TTBR_FLAGS (TTBR_FLAGS_S_BIT | 		\
		TTBR_FLAGS_RGN(0x01) | 	\
		TTBR_FLAGS_IRGN_BIT)
#else
/* 	
	Based on 32-bit TTBR0 format without multiprocessing extensions
	Normal memory - Outer Write-Back Write-allocate cacheable
	Normal memory - Inner Write-Back Write-Allocate Cacheable
	*/

#define TTBR_FLAGS (TTBR_FLAGS_IRGN_BIT | \
		TTBR_FLAGS_RGN(0x01) )
#endif

#endif
