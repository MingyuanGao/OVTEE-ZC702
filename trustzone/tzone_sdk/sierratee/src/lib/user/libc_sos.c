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
 * This file contains those libc functions that may be needed by other apps 
 */

#include "libc_sos.h"
#include "sw_types.h"
#include <sw_syscall.h>
#define MIN(x,y) ((y) ^ (((x) ^ (y)) & -((x) < (y))))

/** @defgroup OS_UserApi User API for Secure OS
 *  API for Sierra OS User tasks
 *  @{
 */

/**
 * @brief 
 *
 * @param seconds
 *
 * @return 
 */
sw_int sleep(sw_uint seconds)
{
#ifndef TIMER_NOT_DEFINED
	sw_uint usecs;
	/* Mutliply by 1000000 */
	usecs = (seconds << 10) + (seconds << 3) - (seconds << 5);
	usecs = (usecs << 10) + (usecs << 3) - (usecs << 5);
	__sw_usleep(usecs);
	return 0;
#else
	return -1;
#endif	
	
}

/**
 * @brief 
 *
 * @param seconds
 *
 * @return 
 */
sw_int usleep(sw_uint seconds)
{
#ifndef TIMER_NOT_DEFINED
	__sw_usleep(seconds);
	return 0;
#else
	return -1;	
#endif

}
/** @} */ // end of OS_UserApi 
