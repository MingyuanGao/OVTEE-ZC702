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

#include <sw_debug.h>
#include "sw_string_functions.h"
#include "sw_syscall.h"
#include "sw_types.h"
#include "sw_device_id.h"

/** @ingroup OS_UserApi User API for Secure OS
 *  API for Sierra OS User tasks
 *  @{
 */

/**
 * @brief 
 *
 * @param fmt
 * @param ...
 *
 * @return 
 */
sw_uint sw_printf(const char *fmt, ...)
{
#if _OTZ_NO_LIBC_BUILD
	va_list args;
	sw_uint i;
	sw_int fd = -1;
	char print_buffer[256];
	va_start(args, fmt);

	i = sw_vsprintf(print_buffer, fmt, args);
	va_end(args);

	/* Print the string */
	fd = __sw_open(UART_DEVICE_NAME, 0, 0);
	if(fd != -1) {
		__sw_write(fd, print_buffer, i);
		__sw_close(fd);
	}
	return i;
#else
	return(0);
#endif
}

/**
 * @brief 
 *
 * @param msg
 */
void tee_panic(char* msg)
{
	while(1);
}

/** @} */ // end of OS_UserApi 
