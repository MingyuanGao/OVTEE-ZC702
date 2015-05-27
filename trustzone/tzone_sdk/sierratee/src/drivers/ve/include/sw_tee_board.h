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

#ifndef __SW_TEE_BOARD_H__
#define __SW_TEE_BOARD_H__

#include <sw_types.h>
#include <sw_device_io.h>

#ifndef CONFIG_BOOT_SVISOR
#define NORMAL_WORLD_RAM_START  0x80000000
#define NSK_LOAD_ADDRESS        NORMAL_WORLD_RAM_START  
#define GUEST_MEM_SIZE			0x8000000
#else   
#define NORMAL_WORLD_RAM_START  (0x88000000)
#define NSK_LOAD_ADDRESS        NORMAL_WORLD_RAM_START  
#define GUEST_MEM_SIZE 0x04000000
#endif  /* CONFIG_BOOT_SVISOR */

/* 1MB below SECURE_WORLD_RAM_START is reserved for "nsadmin" section*/
#define SECURE_WORLD_RAM_START  (VE_FRAME_BASE_PA + VE_FRAME_SIZE)
#define SECURE_WORLD_RAM_END    (0x940FFFFF)

#define SECURE_WORLD_RAM_SIZE	(SECURE_WORLD_RAM_END - SECURE_WORLD_RAM_START)


#ifdef CONFIG_MULTI_GUESTS_SUPPORT
#define LINUX_INITRD_ADDR 0x88d00000
#define LINUX_INITRD_SIZE (8192 * 1024)
#endif

/* VE RS1 IRQs numbers definitions */
#define IRQ_TIMER_PAIR0          (2 + IRQ_GIC_START)
#define IRQ_TIMER_PAIR1          (3 + IRQ_GIC_START)

#define FREE_RUNNING_TIMER_IRQ      IRQ_TIMER_PAIR1
#define TICK_TIMER_IRQ              IRQ_TIMER_PAIR1
#define KEY_PRESS_IRQ               (8 + IRQ_GIC_START)

#define GIC_ITLINES  2

#ifdef SCHEDULE_HIGH_PRIORITY_GUEST
#define HIGH_PRIORITY_GUEST 0
#define LOW_PRIORITY_GUEST_UART_IRQ	37
#endif

#define TASK_HEAP_SIZE   (1 << 16) /* 64 kb */

#define COMMON_HEAP_SIZE   (1 << 16)

#define SECURE_UART_ID 3
#define UART_ID SECURE_UART_ID

/**
 * @brief 
 *
 * @param pgd
 */
void board_map_secure_page_table(sw_uint* pgd);

/**
 * @brief Board specific eMMC init routine
 */
int board_mmc_init(void);


/**
 * @brief 
 */
void unmap_init_section(void);


/**
 * @brief 
 *      Dummy funtion to handle board smc
*/
void board_smc_handler();

#endif
