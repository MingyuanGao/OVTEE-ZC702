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

/*
 * sw_platform_asm functions implementation
 */

#define  NSACR_REG_VAL      0x60C00
#define  CPACR_REG_VAL      0xF00000

#define	CACHE_LEVEL1_SET	1
#define	CACHE_LEVEL2_SET	0

/*	LEVEL 1 Configuration *
 *	32KB, 
	NumSets = 256, 
	Ways=4, 
	Line Size = 5 (log2 line_len) and Line len = 32 bytes
 */
#define MAX_L1_CACHE_WAYS	3   /* Ways -1 */
#define MAX_L1_SETS			255 /* NumSets -1 */
#define MAX_L1_LINE_LEN		5

#define MAX_L2_CACHE_WAYS	0   /* Ways -1 */
#define MAX_L2_SETS			0	/* NumSets -1 */
#define MAX_L2_LINE_LEN		0
