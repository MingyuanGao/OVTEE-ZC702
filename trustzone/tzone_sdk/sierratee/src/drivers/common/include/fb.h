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
 * Header for Secure Frame buffer common declaration
 */

#ifndef __SW_COMMON_FB_H_
#define __SW_COMMON_FB_H_

#include <sw_device_id.h>
#include <sw_lib_fb.h>

/**
* @brief framebuffer open function need to be implemented for each board
*
* @return 
*/
sw_int sw_fb_open(void);

/**
* @brief framebuffer close function need to be implemented for each board
*
* @return 
*/
sw_int sw_fb_close(void);

/**
* @brief 
* @brief framebuffer ioctl function need to be implemented for each board
*
* @param ioctl_id
* @param req
* @param res
*
* @return 
*/
int sw_fb_ioctl(sw_uint ioctl_id, void* req, void* res);
#endif
