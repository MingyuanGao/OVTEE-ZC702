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
 * Task local storage for FILE I/O operations
 */

#ifndef __SW_FILE_TLS__
#define __SW_FILE_TLS__

/**
 * @brief	Task local storage for FILE I/O operations
 *	sw_tls_file structure contains refernce pointer
 *	for newlib enable/disable FILE structure.
 *	From which we can store and retrieve appropriate
 *	variables.
 *
 */
typedef struct {
#ifdef NEWLIB_SUPPORT
    void* file_lib_pointers;
#endif
#ifndef NEWLIB_SUPPORT
    void* sw_buf_io;
#endif
}sw_tls_file;

#endif

