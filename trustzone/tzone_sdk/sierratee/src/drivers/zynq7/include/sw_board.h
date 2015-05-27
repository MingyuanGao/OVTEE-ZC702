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

#ifndef _SW_BOARD_H
#define _SW_BOARD_H

#include <sw_types.h>
#include <sw_device_io.h>

#define SECURE_TIMER_BASE       (SLCR_BASE + 0x2000) /* TTC1 */

#define SECURE_UART_ID          0

#ifdef CONFIG_SW_MULTICORE
#define MAX_CORES  2
#else
#define MAX_CORES 1
#endif

#define FREE_RUNNING_TIMER_BASE (SECURE_TIMER_BASE)
#define TICK_TIMER_BASE         (SECURE_TIMER_BASE + 0x4)

#define FREE_RUNNING_TIMER_IRQ  69
#define TICK_TIMER_IRQ          70

#define TZ_GEM              0xF2002450
#define TZ_GEM_PA               0xF8000450
#define TZ_SDIO             0xF2002454
#define TZ_SDIO_PA              0xF8000454
#define TZ_USB              0xF2002458
#define TZ_USB_PA               0xF8000458
#define TZ_FPGA_M           0xF2002484
#define TZ_FPGA_M_PA            0xF8000484
#define TZ_FPGA_AFI         0xF2002488
#define TZ_FPGA_AFI_PA          0xF8000488

/* SLCR registers */
#define SLCR_LOCK               (SLCR_BASE + 0x4)
#define SLCR_UNLOCK             (SLCR_BASE + 0x8)
#define TZ_DDR_RAM              (SLCR_BASE + 0x430)
#define DMAC_RST_CTRL           (SLCR_BASE + 0x20C)
#define TZ_DMA_NS               (SLCR_BASE + 0x440)
#define TZ_DMA_IRQ_NS           (SLCR_BASE + 0x444)
#define TZ_DMA_PERIPH_NS        (SLCR_BASE + 0x448)
/*#define TZ_FPGA_M               (SLCR_BASE + 0x484)
#define TZ_FPGA_AFI             (SLCR_BASE + 0x488)*/
#define SECURITY_FSSW_S0        (SECURITY_MOD3 + 0x1C)
#define SECURITY_FSSW_S1        (SECURITY_MOD3 + 0x20)
#define SECURITY_APB            (SECURITY_MOD3 + 0x28)
#define GIC_CPU                 (SCU_BASE + 0x100)
#define GIC_DIST                (SCU_BASE + 0x1000)

#define SECURITY2_SDIO0         (SECURITY_MOD2 + 0x8)
#define SECURITY3_SDIO1         (SECURITY_MOD2 + 0xC)
#define SECURITY4_QSPI          (SECURITY_MOD2 + 0x10)
#define SECURITY5_MIOU          (SECURITY_MOD2 + 0x14)
#define SECURITY6_APBSL         (SECURITY_MOD2 + 0x18)
#define SECURITY7_SMC           (SECURITY_MOD2 + 0x1C)

#define NORMAL_WORLD_RAM_START  0x00000000
#define NSK_LOAD_ADDRESS        0x00000000
#define SECURE_WORLD_RAM_START	0x3C000000
#define SECURE_WORLD_RAM_END    (0x40000000)

#define SECURE_WORLD_RAM_SIZE	(SECURE_WORLD_RAM_END - SECURE_WORLD_RAM_START)

#define GUEST_MEM_SIZE          0x20000000 /* this is size of first guest */

#define SLCR_LOCK_KEY           0x767B
#define SLCR_UNLOCK_KEY         0xDF0D

#define GIC_BANK_OFFSET         0x0
#define IRQ_GIC_START			32
#define GIC_NR_IRQS             128
#define GIC_MAX_NR				1

#define KEY_PRESS_IRQ			(IRQ_GIC_START + 50)

#define NO_OF_INTERRUPTS_IMPLEMENTED GIC_NR_IRQS

#define GIC_ITLINES  2

#define BOOT_ADDR_OFFSET           0xEFF0
#define BOOT_STATUS_OFFSET         0xEFF4

#define SECONDARY_BOOT_POINTER     OCM_HIGH_PHYS
#define SECBOOTP_COFFSET           BOOT_ADDR_OFFSET
#define SECBOOTP_STATUS_OFFSET     BOOT_STATUS_OFFSET


#define TIMER_PERIOD_US 18
#define TIMER_COUNT_MAX 0xFFFF
#define TASK_HEAP_SIZE (1 << 16)

#define COMMON_HEAP_SIZE   (1 << 16)

extern sw_uint sec_core_start_addr;
extern sw_uint sec_core_start_stat_addr;

/**
 * @brief 
 *
 * @param pgd
 */
void board_map_secure_page_table(sw_uint* pgd);

/**
 * @brief 
 *
 * @return 
 */
sw_phy_addr get_ram_start_addr(void);

/**
 * @brief 
 *
 * @return 
 */
sw_phy_addr get_ram_end_addr(void);

/**
 * @brief 
 */
void board_init(void);

/**
 * @brief Board specific eMMC init routine
 */
sw_uint board_mmc_init(void);

/**
 * @brief 
 */
void unmap_init_section(void);

sw_uint map_devices(void);

#endif

