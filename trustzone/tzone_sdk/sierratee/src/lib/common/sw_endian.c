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

/* Convert endian */

#include <sw_endian.h>

/**
 * @brief 
 *
 * @param value
 *
 * @return 
 */
sw_uint change_endian(sw_uint value)
{
	sw_uint mod_val = 0;
	mod_val |= (0xFF000000 & value) >> 24;
	mod_val |= (0x00FF0000 & value) >> 8;
	mod_val |= (0x0000FF00 & value) << 8;
	mod_val |= (0x000000FF & value) << 24;
	return mod_val;
}
