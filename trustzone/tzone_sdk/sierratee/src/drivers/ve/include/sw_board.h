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
 *versatile express RS1 board configuration
 */

#ifndef _VE_BOARD_H__
#define _VE_BOARD_H__

#include <sw_types.h>

#ifndef CONFIG_SVISOR_SUPPORT
#include <sw_tee_board.h>
#else
#include <svisor_board.h>
#endif

#define MAX_CORES 1

/*
 * Irqs
 */
#define IRQ_GIC_START           32
#define GIC_NR_IRQS         (IRQ_GIC_START + 64)
#define GIC_MAX_NR          1

#define NO_OF_INTERRUPTS_IMPLEMENTED GIC_NR_IRQS

#define NR_CPU_IRQS	IRQ_GIC_START
#define NR_SPI_IRQS	GIC_NR_IRQS - NR_CPU_IRQS

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
 * @brief 
 */
void map_devices(void);

#endif
