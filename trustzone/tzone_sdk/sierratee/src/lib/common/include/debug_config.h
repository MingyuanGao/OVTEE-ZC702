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

#ifndef _SW_DEBUG_CONFIG_H
#define _SW_DEBUG_CONFIG_H

#ifdef SW_DEBUG_ENABLED
/** 
 * Enable or disable module debug flags.
 * Any module debug flag need to be added on this file.
 */

/* Enable debug flag in ACL module */
#undef DEBUG_ACL
/* Enable debug flag in Pagetable implementation */
#undef DEBUG_PAGETABLE
/* Enable debug flag in TEE based Hypervisor */
#undef SW_OTZHYP_DEBUG 

/* Enable debug flag in Exynos based eMMC driver */
#undef DEBUG_S5P_MSHC

/* Enable debug flag in Virtual Keyboard component */
#undef DEBUG_VK

/* Enable debug flag in Touch manager component */
#undef SW_DEBUG_TOUCH_MANAGER

/* Enable debug flag in Buddy allocator */
#undef DEBUG_BUDDY

/* Enable debug flag in Heap manager */
#undef DEBUG_HEAP

/* Enable debug flag in Timer component */
#undef DEBUG_TIMER 

/* Enable debug flag in Wait queue component */
#undef DEBUG_WAIT_QUEUE

/* Enable debug flag in CPU API implementation */
#undef DEBUG_CPU_API

/* Enable debug flag in Secure API implementation */
#undef DEBUG_SECURE_API

/* Enable debug flag in L2CC driver */
#undef DEBUG_L2CC

/* Enable debug flag in Secure Timer driver */
#undef DEBUG_SEC_TIMER

/* Enable debug flag in Secure Frame Buffer driver */
#undef DEBUG_FB

/* Enable debug flag in I2C driver */
#undef DEBUG_I2C
#undef DEBUG_I2C_INFO

/* Enable debug flag in Touch Screen driver */
#undef DEBUG_TOUCH_SCREEN

/* Enable debug flag in Errno */
#undef DEBUG_ERRNO

/* Enable debug flag in Memory function routines */
#undef DEBUG_MEMORY_FUNCTIONS

/* Enable debug flag in secure boot loader */
#undef SW_BL_DBG

/* Enable debug flag in Global Platform Arithmetic API */
#undef DEBUG_GP_ARITH

#endif
#endif
