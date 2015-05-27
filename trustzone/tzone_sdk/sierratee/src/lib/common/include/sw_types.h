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
 * Header for common Data types
 */

#ifndef __LIB__TYPES_H__
#define __LIB__TYPES_H__
#include <sw_common_types.h>
#include <sw_errno.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) < (b) ? (b) : (a))

#define TRUE     1
#define FALSE    0

#define true     1
#define false    0

#ifndef NULL
#define NULL    0
#endif

/*
#define OTZ_OK         		0
#define OTZ_EFAIL		-1
#define OTZ_ENOMEM           	-2
#define OTZ_ILLEGAL_ARGUMENT 	-3

#define SW_OK                	0
#define SW_EFAIL        	-1
#define SW_ENOMEM           	-2
#define SW_ILLEGAL_ARGUMENT 	-3
#define SW_EINVALID         	-5
*/
#define SW_STDIN		0
#define SW_STDOUT		1
#define SW_STDERR		2

typedef unsigned char		sw_short_int;
typedef unsigned int		sw_uint;
typedef int			sw_int;
typedef unsigned long		sw_ulong;
typedef long			sw_long;

typedef unsigned short		sw_ushort;
typedef signed short		sw_short;

typedef unsigned long long	sw_big_ulong;
typedef signed long long	sw_big_long;
typedef unsigned int 		sw_vir_addr;
typedef unsigned int 		sw_phy_addr;

typedef unsigned char   	sw_bool;

#ifndef NULL
#define NULL 0
#endif

#endif
