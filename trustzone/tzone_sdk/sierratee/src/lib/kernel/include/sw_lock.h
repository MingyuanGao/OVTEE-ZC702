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

#ifndef _SW_LOCK_H_
#define _SW_LOCK_H_

#include <cpu.h>

#define RESET_BUSY_LOCK(s) ((s)->lock = 0)

/**
 * @brief 
 */
struct lock_shared{
	sw_uint lock;
};

/**
 * @brief 
 *
 * @param lock
 * @param cpsr
 */
static inline void lock_irq_shared_resource(struct lock_shared* lock, 
		sw_uint* cpsr)
{
	*cpsr = cpu_save_irq_state();
	/* If TEE runs on SMP, we need to lock the shared variable.
	 * otherwise, we need to disable IRQ and save the current program
	 * status.
	 */

}

/**
 * @brief 
 *
 * @param slock
 * @param cpsr
 */
static inline void unlock_irq_shared_resource(struct lock_shared* lock, 
		sw_uint cpsr)
{
	/* If TEE runs on SMP, we need to release the lock on the shared variable.
	 * otherwise, we need to restore the current program
	 * status.
	 */

	cpu_restore_irq_state(cpsr);
}
#endif
