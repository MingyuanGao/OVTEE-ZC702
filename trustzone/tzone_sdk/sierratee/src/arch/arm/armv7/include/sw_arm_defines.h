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
 * Header for common ARM specific declarations
 */

#ifndef __ARMV7_SW_ARM_DEFINES_H__
#define __ARMV7_SW_ARM_DEFINES_H__

/* assembler definitions */
#define  ARCH_USR_MODE	0x10
#define  ARCH_FIQ_MODE	0x11
#define  ARCH_IRQ_MODE	0x12
#define  ARCH_SVC_MODE	0x13
#define  ARCH_MON_MODE	0x16
#define  ARCH_ABT_MODE   0x17
#define  ARCH_HYP_MODE   0x1A
#define  ARCH_UNDEF_MODE	0x1B
#define  ARCH_SYS_MODE	0x1F


#define	IRQ_BIT          0x80    
#define	FIQ_BIT          0x40    

#define	SCTLR_MMU_BIT			0x1
#define SCTLR_DCACHE_BIT		(1 << 2)
#define SCTLR_BRANCH_PRED_BIT	(1 << 11)
#define SCTLR_ICACHE_BIT		(1 << 12)


#define  SCR_NS_BIT     	0x1
#define  SCR_FIQ_BIT        0x4
#define  SCR_IRQ_BIT        0x2
#define  SCR_HCR_BIT		0x100

#define CNTFRQ_VAL  0x5f5e100   /* 100 MHz */
#endif
