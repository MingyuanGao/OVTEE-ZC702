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
 * versatile express RS1 board configuration
 */

#include <sw_board.h>
#include <task.h>
#include <global.h>
#include <otz_id.h>
#include <secure_api.h>
#include <page_table.h>
#include <cpu_data.h>
#include <sw_io.h>
#include <cpu.h>

static const struct devmap ve_devmap[] =  {
	{
		.dv_va = SECURE_UART_BASE & 0xfffff000,
		.dv_pa = SECURE_UART_BASE_PA & 0xfffff000,
		.dv_size = SECURE_UART_BASE_SIZE,
	},
	{
		.dv_va = FREE_RUNNING_TIMER_BASE & 0xfffff000,
		.dv_pa = FREE_RUNNING_TIMER_BASE_PA & 0xfffff000,
		.dv_size = FREE_RUNNING_TIMER_BASE_SIZE,
	},
	{
		.dv_va = VE_SYSTEM_REGS & 0xfffff000,
		.dv_pa = VE_SYSTEM_REGS_PA & 0xfffff000,
		.dv_size = VE_SYSTEM_REGS_SIZE,
	},
	{
		.dv_va = VE_CLCD_BASE & 0xfffff000,
		.dv_pa = VE_CLCD_BASE_PA & 0xfffff000,
		.dv_size = VE_CLCD_BASE_SIZE,
	},
	{
		.dv_va = SYSCTL_BASE & 0xfffff000,
		.dv_pa = SYSCTL_BASE_PA & 0xfffff000,
		.dv_size = SYSCTL_BASE_SIZE,
	},
	{
		.dv_va = VE_RS1_MPIC,
		.dv_pa = VE_RS1_MPIC_PA,
		.dv_size = VE_RS1_MPIC_SIZE,

	},
	{0}
};

#ifdef CONFIG_SW_DEDICATED_TEE
extern sw_uint secondary_start_config_reg[MAX_CORES];
#endif

/**
 * @brief 
 */
void board_init(void)
{
}

#ifdef CONFIG_SW_DEDICATED_TEE

/**
 * @brief 
 *
 * @param cpu_id
 */
void start_secondary_linux(sw_uint cpu_id)
{
	switch (cpu_id) {
		case 0:
			secondary_start_config_reg[cpu_id] = KERNEL_START_ADDR;
			break;
		case 1:
			secondary_start_config_reg[cpu_id] = KERNEL_START_ADDR;
			break;
		default:
			break;
	}
	dmb();
}
#endif


/**
 * @brief 
 *
 * @param pgd
 */
void board_map_secure_page_table(sw_uint* pgd)
{
	sw_uint va, va_start;
	sw_phy_addr pa, pa_start;
	sw_uint pa_offset;

	va_start = (sw_uint)get_sw_text_start();
	pa_start = SECURE_WORLD_RAM_START;

	/* Map kernel code */

	/* text */
	va = (sw_uint)get_sw_text_start();
	pa = pa_start;

	pa_offset = va -pa;
	map_secure_memory(va, pa, get_sw_text_size(), PTF_PROT_KRW| PTF_EXEC);

	/*init and mod init section*/
	va = (sw_uint)get_init_start_addr();
	pa_offset =  va - va_start;
	pa = pa_start + pa_offset;
	map_secure_memory(va, pa, get_init_size(), PTF_PROT_KRW| PTF_EXEC);

	/* data */
	va = (sw_uint)get_sw_data_start();
	pa_offset =  va - va_start;
	pa = pa_start + pa_offset;

	map_secure_memory(va, pa, get_sw_data_size(), PTF_PROT_KRW);

	/* bss */
	va = (sw_uint)get_sw_bss_start();
	pa_offset =  va - va_start;
	pa = pa_start + pa_offset;

	map_secure_memory(va, pa, get_sw_bss_size(), PTF_PROT_KRW);

	/* bitmap */
	va = (sw_uint)get_mem_info_start();
	pa_offset =  va - va_start;
	pa = pa_start + pa_offset;

	map_secure_memory(va, pa, get_mem_info_size(), PTF_PROT_KRW);


	map_secure_memory((SECURE_UART_BASE_PA & 0xfffff000), 
			(SECURE_UART_BASE_PA & 0xfffff000) , UART0_SIZE, PTF_PROT_KRW);
}

void map_devices(){

	extern sw_uint UART_ADDR[];
	struct devmap *tmp_ve_devmap = (struct devmap *)ve_devmap;
	while(tmp_ve_devmap) {
		if(tmp_ve_devmap->dv_pa == SECURE_UART_BASE_PA)	{
			map_device(tmp_ve_devmap->dv_va, tmp_ve_devmap->dv_pa, 
					tmp_ve_devmap->dv_size);
			/* Remove one to one mapping we done for UART */
			unmap_secure_memory(SECURE_UART_BASE_PA , UART0_SIZE);

			break;
		}
		tmp_ve_devmap++;
	}

	/* Map devices */
	map_device_table(ve_devmap);

	/* Change UART_ADDR to point to newly mapped addr */
	UART_ADDR[SECURE_UART_ID] = SECURE_UART_BASE;
	/*Mapping frame buffer memory*/
	map_secure_memory(VE_FRAME_BASE, VE_FRAME_BASE_PA, 
			VE_FRAME_SIZE, PTF_PROT_URW);	
}

/**
 * @brief 
 *      This functions unmaps init and mod init from secure ram
 */
void unmap_init_section(void){
	sw_vir_addr va;
	va = (sw_vir_addr)get_init_start_addr();
	unmap_secure_memory(va, get_init_size());
}

/**
 * @brief 
 *      This functions returns the starting address of the 
 *      Secure RAM
 * @return 
 *      Starting address of the Secure RAM
 */
sw_phy_addr get_ram_start_addr(void)
{   
	return SECURE_WORLD_RAM_START;
}

/**
 * @brief 
 *      This function returns the End address of the Secure RAM
 * @return 
 *      End address of the Secure RAM
 */
sw_phy_addr get_ram_end_addr(void)
{
	return SECURE_WORLD_RAM_END;
}

/**
 * @brief 
 *      Dummy funtion to handle board smc
 */
void board_smc_handler(){
}

/**
 * @brief Create entry_point,open_session and start the task
 *          of Dispatcher,Shell and Linux by their corresponding
 *          sa_config_t instance and service_id
 */
void run_init_tasks(void) {
	sa_config_t sa_config,sa_config_linux;
	int dispatch_task_id, ret_val;
#ifdef CONFIG_SHELL
	sa_config_t sa_config_shell;
	int shell_task_id;
#endif
	/* Dispatcher should be the first task to start */
	ret_val = sa_create_entry_point(OTZ_SVC_GLOBAL, &sa_config);
	if(ret_val == SW_OK)
	{
		if(sa_open_session(&sa_config, (void*)&dispatch_task_id)
				== SW_OK) {
			start_task(dispatch_task_id, NULL);
		}
	}

#ifdef CONFIG_SHELL
	ret_val = sa_create_entry_point(OTZ_SVC_SHELL, &sa_config_shell);
	if(ret_val == SW_OK)
	{
		if(sa_open_session(&sa_config_shell, (void*)&shell_task_id)
				== SW_OK) {
			start_task(shell_task_id, NULL);
		}
	}
#endif

	ret_val = sa_create_entry_point(OTZ_SVC_LINUX, &sa_config_linux);
	if(ret_val == SW_OK)
	{
		if(sa_open_session(&sa_config_linux, (void*)&global_val.linux_task_id)
				== SW_OK) {
#ifndef CONFIG_SHELL
			start_task(global_val.linux_task_id, NULL);
#endif
		}
	}
	
	/* Testing the task context switching,heap_buddy_allocator */
	run_init_test_tasks();

}

