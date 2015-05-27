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
 * Header for sw_board_Asm configuration
 */

#ifndef _SW_BOARD_ASM_H_
#define _SW_BOARD_ASM_H_

#define SCU_BASE                   0xF8F00000
#define SCU_SIZE                   0x2000
#define SLCR_BASE                  0xF8000000
#define SLCR_SIZE                  0x8000
#define SECURITY_MOD2              0xE0200000
#define SECURITY_MOD2_SIZE         0x2000
#define SECURITY_MOD3              0xF8900000
#define SECURITY_MOD3_SIZE         0x2000
#define SECURE_UART_BASE           0xE0001000 /* shared by non secure as well */
#define SECURE_UART_BASE_SIZE      0x1000

#define ZYNQ_L2_BASE              0xF8F02000
#define ZYNQ_L2_SIZE              0x1000

#define OCM_HIGH_PHYS              0xFFFF1000
#define OCM_HIGH_PHYS_SIZE         0xF000

#define NSADMIN_LOAD            0x38000000

#define SECBOOT_STAGE_1    		0xA
#define SECBOOT_STAGE_2         0x1

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
#define NS_SGI_NOTIFY_INTERRUPT 0x7
#endif


#endif
