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
 * Header for Secure Frame buffer library interface
 */

#ifndef __SW_LIB_FB_H_
#define __SW_LIB_FB_H_

#include <sw_types.h>

#define SW_FB_IOCTL_GET_INFO 0x111
#define SW_FB_IOCTL_SET_INFO 0x112
#define SW_FB_IOCTL_WRITE_PIXEL 0x113
#define SW_FB_IOCTL_WRITE_WINDOW 0x114
#define SW_FB_IOCTL_FLUSH_CACHE 0x115


/**
* @brief 
*/
struct sw_fb_info {
	sw_uint xres;
	sw_uint yres;		
	sw_uint bpp;
	sw_uint format;
	sw_uint fb_size;
	sw_uint* fb_buffer;
};

#endif
