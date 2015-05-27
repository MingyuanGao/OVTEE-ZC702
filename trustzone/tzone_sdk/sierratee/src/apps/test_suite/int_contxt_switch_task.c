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
 * Testing task implementation
 */

#include <int_contxt_switch_task.h>
#include <sw_debug.h>
#include <sw_errno.h>
#include <sw_mem_functions.h>
#include <sw_string_functions.h>
#include <sw_buddy.h>

static sw_uint contxt_val[TESTING_INDEX] = { 0 };

/**
 * @brief Integer Context Switch task entry point
 *
 * This function is implemented to test context switching between the tasks
 *
 * @param task_id: Task identifier
 * @param tls: Pointer to task local storage
 */
void int_contxt_switch_task(int task_id, sw_tls* tls){

	task_init(task_id, tls);
	tls->ret_val = SW_OK;
	static sw_uint index = 0;	
	num_math_calc(contxt_val[index]);
	if(++index >= TESTING_INDEX)
		index = 0;
	task_exit(task_id, tls);
	return;
}

/**
 * @brief Test integer calculations
 *
 * This function performs interger calculations and test its correctness
 *
 * @param value : Value is incremented if the calculation is passed
 */
void num_math_calc(sw_uint value)
{
	sw_long n1, n2, n3, n4, n5;
	const sw_long total = (1235 * -7643) + (93742 * 3054);
	while(1) {
		n1 = 1235;
		n2 = -7643;
		n3 = 93742;
		n4 = 3054;
		n5 = (n1 * n2) + (n3 * n4);
		
		if(n5 == total) {
			value++;
		}
		else {
			sw_seterrno(SW_EINVAL);
			sw_printk("Integer calculation failed\r\n");
		}
		array_math_test(value);
		schedule();
	}
	return;
}

/**
 * @brief Test array calculations
 *
 * This function performs array calculations and test its correctness
 *
 * @param value : Value is incremented if the calculation is passed
 */
void array_math_test(sw_uint value)
{
	sw_uint i, pos = 256;
	sw_long *array_math, total1 = 0, total2 = 0;
	array_math = (sw_long *)sw_malloc(256 * sizeof(sw_long));
	if(!array_math) {
		sw_seterrno(SW_ENOMEM);
		goto array_math_ret;
	}
	for(i = 0; i < pos; i++) {
		array_math[pos] = (23452 * 894);
		total1 += (23452 * 894);
	}
	schedule();
	for(i = 0; i < pos; i++) {
		total2 += array_math[pos];
	}
	if(total1 == total2) {
		value++;
	}
	else {
		sw_seterrno(SW_EINVAL);
		sw_printf("Array calculation failed\r\n");
	}
array_math_ret:
	if(array_math)
		sw_free(array_math);
	return;
}
