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
 * Kernel debug print routines
 */
#include "sw_debug.h"
#include "cpu.h"
#include "sw_string_functions.h"
#include "uart.h"
#include "sw_modinit.h"
#include "sw_types.h"
#include "sw_device_id.h"
#include <sw_types.h>

/** @ingroup OS_KernelApi Kernel API for Secure OS
 *  API for Sierra OS Kernel tasks
 *  @{
 */
/**
 * @brief Writes directly to device, with no permission check
 *
 * @param fmt
 * @param ...
 *
 * @return 
 */

sw_uint sw_printf(const char *fmt, ...)
{
	va_list args;
	sw_uint i;
	char print_buffer[256];
	va_start(args, fmt);

	i = sw_vsprintf(print_buffer, fmt, args);
	va_end(args);

	/* Print the string */
	serial_puts(print_buffer);
	return i;
}

/**
 * @brief Writes directly to device, with no permission check
 *
 * @param fmt
 * @param ...
 *
 * @return 
 */
sw_uint sw_printk(const char *fmt, ...)
{
	va_list args;
	sw_uint i;
	char print_buffer[256];
	va_start(args, fmt);

	i = sw_vsprintf(print_buffer, fmt, args);
	va_end(args);

	/* Print the string */
	serial_puts(print_buffer);
	return i;
}


/**
 * @brief 
 *
 * @param msg
 */
void tee_panic(char* msg)
{
#if _OTZ_NO_LIBC_BUILD
	serial_puts(msg);
	serial_puts("\r\n");
#endif
	busy_loop();
}

/** @} */ // end of OS_KernelApi
