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
 * Header for sw_board_asm implementation
 */

#ifndef __SW_BOARD_ASM_H__
#define __SW_BOARD_ASM_H__

#define SCU_BASE    (0x2c000000)
#define VE_RS1_MPIC (0x2c000000)
#define VE_RS1_MPIC_SIZE 0x3000

#ifdef CONFIG_SW_DEDICATED_TEE
#define KERNEL_START_ADDR 0x80000000
#endif

#ifndef CONFIG_CORTEX_A15
#define GIC_CPU     (VE_RS1_MPIC + 0x0100)
#define GIC_DIST    (VE_RS1_MPIC + 0x1000)
#else /* Cortex A15 */
#define GIC_CPU     (VE_RS1_MPIC + 0x2000)
#define GIC_DIST    (VE_RS1_MPIC + 0x1000)
#define GIC_HYP     (VE_RS1_MPIC + 0x4000)
#define GIC_VCPU    (VE_RS1_MPIC + 0x6000)
#endif

#define GIC_DIST_SIZE 0x1000

#define GIC_BANK_OFFSET 0x0

#define NSADMIN_LOAD            0x90000000

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
#define NS_SGI_NOTIFY_INTERRUPT 0x7
#endif

//From board.h


#define VE_FRAME_BASE   0x90100000
#define VE_FRAME_SIZE   (1048576 * 3)

#define VE_RS1_L2CC (0x2c100000)
#define VE_RS1_L2CC_SIZE 0x10000

#define VE_RS1_SCU  (VE_RS1_MPIC + 0x0000)
#define VE_RS1_MPCORE_TWD   (VE_RS1_MPIC + 0x0600)

#define VE_SYSTEM_REGS  0x1C010000
#define VE_SYSTEM_REGS_SIZE 0x10000
#define VE_CLCD_BASE    0x1C1F0000
#define VE_CLCD_BASE_SIZE 0x10000
#define VE_SYS_FLAGSSET_ADDR (VE_SYSTEM_REGS + 0x30)
#define VE_SYS_FLAGSCLR_ADDR (VE_SYSTEM_REGS + 0x34)

/*
 * Peripheral addresses
 */
#define SYSCTL_BASE         0x1c020000
#define SYSCTL_BASE_SIZE    0x10000

#define TIMER0_BASE         0x1c110000
#define TIMER1_BASE         0x1c110020
#define TIMER2_BASE         0x1c120000
#define TIMER3_BASE         0x1c120020
#define TIMER_COUNT_MAX	    0xFFFFFFFF
#define TIMER0_SIZE         0x00010000
#define TIMER2_SIZE         0x00010000

#define UART0_ADDR      0x1C090000
#define UART0_SIZE      0x00010000
#define UART1_ADDR      0x1C0A0000
#define UART1_SIZE      0x00010000
#define UART2_ADDR      0x1C0B0000
#define UART2_SIZE      0x00010000
#define UART3_ADDR      0x1C0C0000
#define UART3_SIZE      0x00010000


#define FREE_RUNNING_TIMER_BASE     TIMER2_BASE
#define TICK_TIMER_BASE             TIMER3_BASE

#define SECURE_UART_BASE    UART3_ADDR
#define SECURE_UART_BASE_SIZE UART3_SIZE

#endif
