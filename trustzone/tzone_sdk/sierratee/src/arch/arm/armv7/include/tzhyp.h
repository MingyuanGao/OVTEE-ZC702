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
 * Header for trustzone based hypervisor implementation
 */

#ifndef TZHYP_H
#define TZHYP_H

#include <cpu_asm.h>


#define GET_CORE_BOUNDARY(world)					\
	((struct system_context *)GET_CORE_CONTEXT(world) + GUESTS_NO)

#define GET_CORE_CONTEXT(context)					\
	((char *)context + (get_cpu_id() * sizeof(struct system_context)\
		* GUESTS_NO))

#define GET_CORE_CONTEXT_BYID(context, id)				\
	((char *)context + (id * sizeof(struct system_context) \
		* GUESTS_NO))

#define SET_CORE_CONTEXT(new, context)	\
	(context = (struct system_context *)((char *)new - (get_cpu_id()			\
														* sizeof(struct system_context) * GUESTS_NO)))

/**
 * @brief Trustzone Hypervisor global variables structure
 */
struct tzhyp_values {
	/**
	 * @brief Non secure preempt flag
	 */
	sw_uint ns_preempt_flag;

	/**
	 * @brief NS switch flag to indicate TLB flush 
	 */
	sw_uint ns_switch_flag;
	
	/**
	 * @brief Non secure contexts
	 */
	struct system_context *ns_world;

	/**
	 * @brief Common Secure context;  no secure context for secondary cores 
	 */
	struct system_context *s_world;
	};

/**
 * @brief 
 *
 * @return 
 */
extern int tzhyp_init(void);

/**
 * @brief 
 */
extern void tzhyp_boot_ack_event(void);

#endif
