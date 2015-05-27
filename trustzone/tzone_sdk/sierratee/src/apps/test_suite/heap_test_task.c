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
 * Test heap invoke functions
 */

#include <sw_mem_functions.h>
#include <sw_types.h>
#include <sw_debug.h>
#include <heap_test_task.h>
#include <sw_user_app_api.h>

static int tmp_val[NO_OF_BINS];

/**
* @brief 
*/
void heap_test_task(int task_id,sw_tls *tls) {

	task_init(task_id, tls);
	sw_uint *alloc_addr,alloc_val=NO_OF_BINS,free_val=NO_OF_BINS,index=1;
    sw_printf("TESTING starts ........\n");
    sw_printf("######### sw_malloc #########\n");
    while(alloc_val--) {
        alloc_addr=(sw_int*)sw_malloc(250);
        sw_printf("%d.Allocated address %x\n",index,(sw_int*)alloc_addr);
        tmp_val[index]=(sw_uint)alloc_addr;
        alloc_addr+=30;
        index++;
    }
    index=1;
    sw_printf("\n");
    sw_printf("######### sw_free #########\n");
    while(free_val) {
        sw_free((void*)tmp_val[index]);
		sw_printf("%d.Freed address %x\n",index,(sw_int*)tmp_val[index]);
        if(--free_val)
            index++;
    }
    sw_printf("TESTING ends ........\n");

	task_exit(task_id, tls);
}
