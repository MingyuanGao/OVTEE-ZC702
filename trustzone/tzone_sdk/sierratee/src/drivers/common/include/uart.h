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
 * UART declarations
 */

#ifndef __COMMON_UART_H__
#define __COMMON_UART_H__

#include <sw_types.h>
/**
 * @brief 
 *
 * @param c
 */
void serial_puts(char * c);

/**
 * @brief 
 *
 * @param c
 */
void serial_putc(char c);

/**
 * @brief 
 *
 * @param uartid
 */
void serial_init(sw_uint uartid);

/**
 * @brief 
 *
 * @param reg_offset
 * @param uartid
 *
 * @return 
 */
sw_uint read_uart(sw_uint reg_offset, sw_uint uartid);

/**
 * @brief 
 *
 * @param reg_offset
 * @param value
 * @param uartid
 */
void write_uart(sw_uint reg_offset, sw_uint value, sw_uint uartid);

#endif
