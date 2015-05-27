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
 * zynq7 board configuration
 */

#include <sw_board.h>
#include <task.h>
#include <otz_id.h>
#include <page_table.h>
#include <cpu_data.h>
#include <sw_io.h>
#include <cpu.h>
#include <sw_debug.h>
#include <smc_id.h>
#include <secure_api.h>

#ifdef CONFIG_CACHE_L2X0
#include <sw_l2_pl310.h>
sw_uint l2base = ZYNQ_L2_BASE;
static struct l2_cache_config_vars sw_l2_cache_config;
#endif

sw_uint sec_core_start_addr;
sw_uint sec_core_start_stat_addr;

#ifdef CONFIG_CACHE_L2X0
static sw_uint aux_ctrl, aux_mask;
#endif

/**
 * @brief 
 */
static const struct devmap zync7_devmap[] =  {
	{
		.dv_va = SECURE_UART_BASE,
		.dv_pa = SECURE_UART_BASE_PA,
		.dv_size = SECURE_UART_BASE_SIZE,
	},
	{
		.dv_va = SCU_BASE, /* covers scu, gic */
		.dv_pa = SCU_BASE_PA,
		.dv_size = SCU_SIZE,
	},
	{
		.dv_va = OCM_HIGH_PHYS, 
		.dv_pa = OCM_HIGH_PHYS_PA,
		.dv_size = OCM_HIGH_PHYS_SIZE,
	},
	{
		.dv_va = SLCR_BASE, 
		.dv_pa = SLCR_BASE_PA,
		.dv_size = SLCR_SIZE,
	},
	{
		.dv_va = SECURITY_MOD2, 
		.dv_pa = SECURITY_MOD2_PA,
		.dv_size = SECURITY_MOD2_SIZE,
	},
	{
		.dv_va = SECURITY_MOD3, 
		.dv_pa = SECURITY_MOD3_PA,
		.dv_size = SECURITY_MOD3_SIZE,
	},
#ifdef CONFIG_CACHE_L2X0
	{
	   .dv_va = ZYNQ_L2_BASE, 
	   .dv_pa = ZYNQ_L2_BASE_PA,
	   .dv_size = ZYNQ_L2_SIZE,
	},
#endif
	{
		.dv_va = 0x40000000,  /* PL secure access */
		.dv_pa = 0x40000000,
		.dv_size = 0x100000,
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

	sw_writel(SLCR_UNLOCK_KEY, (void *)SLCR_UNLOCK);

	/* Non secure memory
	 * Configure the top 64M as secure memory and the rest 
	 * as non secure memory. 
	 * Each bit in TZ_DDR_RAM register represents 64M
	 */
	sw_writel(0x00007fff, (void *)TZ_DDR_RAM);

	sw_writel(0x1, (void *)SECURITY2_SDIO0);
	sw_writel(0x1, (void *)SECURITY3_SDIO1);
	sw_writel(0x1, (void *)SECURITY4_QSPI);

	/* Uart1 shared by non secure as well */
	sw_writel(0x00007fff, (void *) SECURITY6_APBSL);

	sw_writel(SLCR_LOCK_KEY, (void *)SLCR_LOCK);

	/* Initialize the secondary boot pointer */
	sec_core_start_addr = SECONDARY_BOOT_POINTER + SECBOOTP_COFFSET;
	sec_core_start_stat_addr = SECONDARY_BOOT_POINTER + SECBOOTP_STATUS_OFFSET;

#ifdef CONFIG_CACHE_L2X0
	sw_init_l2_pl310_params((void*)l2base);
    sw_l2_cache_config.tag_ram_val = 0;
    sw_l2_cache_config.data_ram_val = 0;
    sw_l2_cache_config.power_val = 0;

    sw_l2_cache_config.filter_init = 0;
    sw_l2_cache_config.filter_start = 0;
    sw_l2_cache_config.filter_end = 0;
    
    sw_l2_cache_config.prefetch_val = 0;
#endif

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

	
	va = SECURE_WORLD_RAM_START;
	pa = SECURE_WORLD_RAM_START;
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
            (SECURE_UART_BASE_PA & 0xfffff000) , SECURE_UART_BASE_SIZE, PTF_PROT_KRW);

#if 0
	sw_vir_addr va;
    sw_phy_addr pa;

    /* Map kernel code */

    serial_puts("board map secure page table\n");

    /* text */
    va = get_sw_text_start();
    pa = va;
    map_secure_memory(va, pa, get_sw_text_size(), PTF_PROT_URO| PTF_EXEC);

    /*init and mod init section*/
    va = get_init_start_addr();
    pa = va;
    map_secure_memory(va, pa, get_init_size(), PTF_PROT_URO| PTF_EXEC);

    /* data */
    va = get_sw_data_start();
    pa = va;
    map_secure_memory(va, pa, get_sw_data_size(), PTF_PROT_URW);

    /* bss */
    va = get_sw_bss_start();
    pa = va;
    map_secure_memory(va, pa, get_sw_bss_size(), PTF_PROT_URW);

	/* 
     * map support data structures for heap - This is a workaround  
     * and has to be fixed in the memory manager
     */

    va = get_sw_code_end();
    pa = va;
    /*
       map_secure_memory(va, pa, 0x100000, PTF_PROT_KRW);
       */
    map_secure_memory(va, pa, TOTAL_HEAP_SIZE, PTF_PROT_URW);

#endif
}

/*
 * Map devices tree
 */
sw_uint map_devices(void){
	extern sw_uint secure_uart_base;
	/* Map devices */
	struct devmap *tmp_ve_devmap = (struct devmap*)zync7_devmap;
	while(*(sw_uint*)tmp_ve_devmap) {
		if(tmp_ve_devmap->dv_pa == SECURE_UART_BASE_PA)	{
			map_device(tmp_ve_devmap->dv_va, tmp_ve_devmap->dv_pa, 
									tmp_ve_devmap->dv_size);
			/* Remove one to one mapping we done for UART */
			unmap_secure_memory((SECURE_UART_BASE_PA & 0xfff00000) , 0x100000);
			
			break;
		}
		tmp_ve_devmap++;
	}	
	map_device_table(zync7_devmap);
	secure_uart_base = SECURE_UART_BASE;
	return SW_OK;
}

/** 
 * @brief 
 *      This functions unmaps init and mod init from secure ram
 */ 
void unmap_init_section(void){
	sw_vir_addr va;
	va = (sw_uint)get_init_start_addr();
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
 * @brief device_sw_phy_addro_va
 *
 *
 * @param dt: pointer to array of device mappings
 *            Array has to terminated with zero.
 *
 * @return sw_vir_addr:
 */

sw_vir_addr device_sw_phy_addro_va(sw_phy_addr pa)
{
	struct devmap *dt = (struct devmap*)zync7_devmap;
	while (*(sw_uint *)dt != 0) {
		if((dt->dv_pa & ~0xFFFFF) == (pa & ~0xFFFFF))
				return (dt->dv_va + (pa - dt->dv_pa));
		dt++;
	};
	return 0x0;
}

void start_sec_kernel(sw_uint smc_id, sw_uint cpu, void *boot_address)
{
	sw_uint p_bootaddr, p_bootstataddr, val;
	static sw_uint wakeup_flag[MAX_CORES];

	if (wakeup_flag[cpu])
		return;

	p_bootaddr = sec_core_start_addr;
	p_bootstataddr = sec_core_start_stat_addr;

	/* update the secondary_start_address */
	sw_printk("start kernel on secondary core\n");

	sw_writel(0, (volatile void*)p_bootstataddr);
	sw_writel((sw_uint)boot_address, (volatile void *)p_bootaddr);

	invoke_dsb();
	invoke_isb();

	asm volatile( "sev     @ send event");

	sw_printk("wait for secondary core to wakeup...");

	while (1) {
		val = sw_readl((volatile void*)p_bootstataddr);
		invoke_dsb();
		invoke_isb();
		if (val == SECBOOT_STAGE_2)
			break;
	}

	wakeup_flag[cpu] = 1;
	invoke_dsb();
	sw_printk("done\n");
}

void sw_init_l2_cache(void)
{
#ifdef CONFIG_CACHE_L2X0
       /* Initialization code does not enable L2 cache */
       sw_l2_pl310_init(aux_ctrl, aux_mask, &sw_l2_cache_config);

       sw_l2_pl310_enable();
#endif
}

/**
 * @brief 
 */
sw_uint board_smc_handler(sw_uint arg0, sw_uint arg1, sw_uint arg2, sw_uint arg3)
{
	switch(arg0) {
		case SMC_CMD_SECURE_READ:
			arg0 = sw_readl((volatile void*)device_sw_phy_addro_va(arg1));
			break;
		case SMC_CMD_SECURE_WRITE:
			sw_writel(arg2,(volatile void*)device_sw_phy_addro_va(arg1));
			break;
		case SMC_CMD_CPU1BOOT:
			start_sec_kernel(arg0, arg1, (void*)arg2);
			break;
#ifdef CONFIG_CACHE_L2X0
       case SMC_CMD_L2X0FLTR_SETUP: {
            sw_l2_cache_config.filter_start = arg1;
            sw_l2_cache_config.filter_end = arg2;
            sw_l2_cache_config.filter_init = 1;
            break;
        }
       case SMC_CMD_L2X0CTRL: {
           if(arg1 == 1)
               sw_l2_pl310_enable();
           else if(arg1 == 0)
               sw_l2_pl310_disable();
           break;
       }
       case SMC_CMD_L2X0SETUP1: {
           sw_l2_cache_config.tag_ram_val = arg1;
           sw_l2_cache_config.data_ram_val = arg2;
           sw_l2_cache_config.prefetch_val = arg3;
           break;
       }
       case SMC_CMD_L2X0SETUP2: {
           sw_l2_cache_config.power_val = arg1;
           aux_ctrl = arg2;
           aux_mask = arg3;
           sw_init_l2_cache();
           break;
       }
       case SMC_CMD_L2X0INVALL: {
           sw_invalidate_l2cache_all();
           break;
       }
       case SMC_CMD_L2X0CLEANALL: {
           sw_clean_l2cache_all();
           break;
       }
       case SMC_CMD_L2X0INVRANGE: {
           sw_invalidate_l2cache_multi_line(arg1, arg2);
           break;
       }
       case SMC_CMD_L2X0INVLINE: {
           sw_invalidate_l2cache_line(arg1);
           break;
       }
       case SMC_CMD_L2X0CLEANRANGE: {
           sw_clean_l2cache_multi_line(arg1, arg2);
           break;
       }
       case SMC_CMD_L2X0CLEANLINE: {
           sw_clean_l2cache_line(arg1);
           break;
       }
       case SMC_CMD_L2X0FLUSHLINE: {
           sw_flush_l2cache_line(arg1);
           break;
       }
       case SMC_CMD_L2X0FLUSHRANGE: {
           sw_flush_l2cache_multi_line(arg1, arg2);
           break;
       }
       case SMC_CMD_L2X0FLUSHALL: {
           sw_flush_l2cache_all();
           break;
       }
#endif
	}
	return arg0;
}


void start_secondary_core(void)
{
	sw_uint p_bootaddr, p_bootstataddr, val;
	sw_uint core;
	core = 1;

	sw_printk("starting secondary core %x .. \n", core);

	p_bootaddr = sec_core_start_addr;
	p_bootstataddr = sec_core_start_stat_addr;

	sw_writel(SECURE_WORLD_RAM_START, (volatile void*)p_bootaddr);

	invoke_dsb();
	invoke_isb();

	asm volatile("sev     @ send event");

	sw_printk("wait for secondary core to wakeup...\n");

	while (1) {
		val = sw_readl((volatile void*)p_bootstataddr);
		invoke_dsb();
		invoke_isb();
		if (val == SECBOOT_STAGE_1)
			break;
	}

	sw_printk("done\n");
}

/**
 * @brief Create entry_point,open_session and start the task
 *          of Dispatcher,Shell and Linux by their corresponding
 *          sa_config_t instance and service_id
 */
void run_init_tasks(void) 
{
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
#if defined(CONFIG_TEST_TASKS ) || defined(CONFIG_TEST_SUITE)
	/* Testing the task context switching,heap_buddy_allocator */
			run_init_test_tasks();
#endif
        }
    }
}
