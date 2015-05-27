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
 * Header for Integer Context Switch task implementation
 */

#ifndef _CONTEXT_SWITCH_TASK_H_
#define _CONTEXT_SWITCH_TASK_H_

#include <sw_types.h>
#include <tls.h>

#define TESTING_INDEX 8

/**
 * @brief Integer Context Switch task entry point
 *
 * This function is implemented to test context switching between the tasks
 *
 * @param task_id: Task identifier
 * @param tls: Pointer to task local storage
 */
void int_contxt_switch_task(int task_id, sw_tls* tls);

/**
 * @brief Test integer calculations
 *
 * This function performs interger calculations and test its correctness
 *
 * @param value : Value is incremented if the calculation is passed
 */
void num_math_calc(sw_uint value);

/**
 * @brief Test array calculations
 *
 * This function performs array calculations and test its correctness
 *
 * @param value : Value is incremented if the calculation is passed
 */
void array_math_test(sw_uint value);

#endif
