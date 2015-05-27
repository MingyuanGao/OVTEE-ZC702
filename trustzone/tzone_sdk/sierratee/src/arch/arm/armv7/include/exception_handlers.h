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
 * Header for exception handlers methods implementation
 */

#ifndef __OTZ_EXCEPTION_HANDLERS_H__
#define __OTZ_EXCEPTION_HANDLERS_H__

#include <sw_types.h>
#include <cpu_data.h>

/**
 * @brief 
 * gets & prints the dfsr & dfar 
 */
void print_data_abort(void); 

/**
 * @brief 
 * gets & prints the ifsr & ifar 
 */
void print_prefetch_abort(void); 

/**
 * @brief FIQ 'C' handler
 */
void tee_fiq_c_handler(void);

/**
 * @brief Prefetch abor handler
 */
void tee_prefetch_abort_handler(void);

/**
 * @brief Data abort handler
 *
 * @param regs: Pointer to the task context
 */
void tee_data_abort_handler(struct swi_temp_regs *regs);

/**
 * @brief Undefined handler 
 */
void tee_undefined_handler(void);

/**
 * @brief SWI handler
 *
 * @param swi_id: SWI ID
 * @param regs: Pointer to the task context
 */
void tee_swi_handler(void *reg0, void *reg1, void* reg2, void* reg3);

/**
* @brief System Call Implementation
*
* @param swi_id: System call identifier
* @param regs: context registers
*/
void invoke_syscall_handler(sw_int swi_id, struct swi_temp_regs *regs);
#endif
