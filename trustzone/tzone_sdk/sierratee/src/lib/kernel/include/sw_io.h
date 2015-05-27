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
 * @brief header file for common io functions.
 */

#ifndef __LIB_IO_H_
#define __LIB_IO_H_

#include <sw_types.h>


/** Memory read/write legacy functions (Assumed to be Little Endian) */
/**
 * @brief 
 *
 * @param addr
 *
 * @return 
 */
static inline sw_short_int sw_readb(volatile void *addr)
{
	return *(sw_short_int*)addr;
}

/**
 * @brief 
 *
 * @param data
 * @param addr
 */
static inline void sw_writeb(sw_short_int data, volatile void *addr)
{
	*(sw_short_int*)addr = data;
}

/**
 * @brief 
 *
 * @param addr
 *
 * @return 
 */
static inline sw_ushort sw_readw(volatile void *addr)
{
	return *(sw_ushort*)addr;
}

/**
 * @brief 
 *
 * @param data
 * @param addr
 */
static inline void sw_writew(sw_ushort data, volatile void *addr)
{
	*(sw_ushort*)addr = data;
}

/**
 * @brief 
 *
 * @param addr
 *
 * @return 
 */
static inline sw_uint sw_readl(volatile void *addr)
{
	return *(sw_uint*)addr;
}

/**
 * @brief 
 *
 * @param data
 * @param addr
 */
static inline void sw_writel(sw_uint data, volatile void *addr)
{
	*(sw_uint*)addr = data;
}

#endif /* __LIB_IO_H_ */
