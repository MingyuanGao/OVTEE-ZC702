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

/* TEE 'c' entry function */
#include <sw_types.h>
#include <monitor.h>
#include <cpu.h>
#include <sw_mmu_helper.h>
#include <page_table.h>
#include <uart.h>
#include <sw_gic.h>
#ifdef CONFIG_SCU
#include <scu.h>
#endif
#include <sw_mem_functions.h>
#include <sw_string_functions.h>
#include <sw_debug.h>
#include <mem_mng.h>
#include <sw_board.h>
#include <global.h>
#include <cpu_data.h>
#include <task.h>

#include <dispatcher_task.h>

#include <otz_id.h>
#include <secure_api.h>
#include <sw_modinit.h>

#include <sw_timer.h>
#include <cache.h>

#include <tzhyp.h>

#include <version.h>
#ifdef CONFIG_NEON_SUPPORT
#include "neon_app.h"
#endif

extern sw_uint kernel;
/**
 * @brief 
 *
 * @return 
 */
int secure_main(void)
{

	serial_init(SECURE_UART_ID);
	serial_puts("\r\nSW: Entering Secure Main \r\n");
	sw_printk("Sierra product: %s\n",SIERRA_PRODUCT_NAME);
	sw_printk("Sierra release version: %s.%s.%s (%s)\n",SIERRA_MAJOR_VERSION, \
			SIERRA_MINOR_VERSION,SIERRA_PATCH_LEVEL,SIERRA_BUILD_NO);
	sw_printk("cross_compiler version: %d.%d.%d\n",__GNUC__,__GNUC_MINOR__, \
			__GNUC_PATCHLEVEL__);


	global_val.heap_init_done = 0;
	global_val.pagetable_addr = sw_meminfo_init(&global_val.sw_mem_info);
	if(global_val.pagetable_addr == NULL) {
		tee_panic("Null secure page table pointer\n");
	}
	
    cpu_mmu_enable();
	branch_predictor_inv_all();
	enable_branch_predictor();

	global_init();
			
	map_devices();
	
	board_init();


#ifdef CONFIG_SCU
	scu_init();
#endif

	sw_gic_distributor_init();
	sw_gic_cpu_interface_init();

	modules_init();
	unmap_init_section();

	tee_irq_enable();
	init_sw_timer();
	timer_init();

	enable_timer(); 
	
	/* Load 'C' library to memory */
	load_libc_to_memory();
	load_user_app_to_memory();
	
#ifdef CONFIG_NEON_SUPPORT
	test_neon_app();
#endif

#ifdef CONFIG_SW_MULTICORE
	flush_icache_and_dcache();
	invoke_dsb();
	invoke_isb();
	start_secondary_core();
#endif   

	tzhyp_init();

	run_init_tasks();

	jump_to_sys_mode();
	schedule();

	while(1);

	return SW_OK;
}

