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

/* File for math functions */

#include <sw_types.h>
#include <sw_math.h>

/** @defgroup OS_CommonApi Common API for Secure OS
 *  API for Sierra OS User and Kernel tasks
 *  @{
 */
/**
 * @brief Function to check if the given number is a power of 2
 *
 * @param x
 *
 * @return 
 */
inline int is_pow_of_2(sw_big_ulong x)
{
	return (x && !(x & (x-1)));
}

/**
 * @brief Function to find the next power of 2 of the number x
 *
 * @param x
 *
 * @return 
 */
inline sw_uint next_pow_of_2(sw_uint x) {
	if ( is_pow_of_2(x) )
		return x;
	x |= x>>1;
	x |= x>>2;
	x |= x>>4;
	x |= x>>8;
	x |= x>>16;
	return x+1;
}

/**
 * @brief Function to find the log base 2 of a number
 *
 * @param number
 *
 * @return 
 */
sw_uint log_of_2(sw_uint number)
{
	static sw_uint MultiplyDeBruijnBitPosition2[32] = 
	{
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return MultiplyDeBruijnBitPosition2[(sw_uint)(number * 0x077CB531U) >> 27];
}

/** @} */ // end of OS_CommonApi
