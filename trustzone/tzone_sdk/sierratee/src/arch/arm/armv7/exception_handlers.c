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
 * Excpetion handlers implementation
 */

#include <sw_types.h>
#include <sw_debug.h>
#include <cpu.h>
#include <sw_mmu_helper.h>
#include <exception_handlers.h>
#include <sw_mem_functions.h>
#include <sw_string_functions.h>
#include <page_table.h>
#include <tzhyp_global.h>
#include <sw_gic.h>
#include <task.h>
#include <global.h>
#include <cpu_data.h>
#include <monitor.h>
#include <sw_timer.h>
#include <secure_api.h>
#include <sw_syscalls_id.h>

#include <sw_board.h>
#include <secure_timer.h>
#include <otz_id.h>

/**
 * @brief Data abort fault strings
 */
static const char* dfsr_short_desc_error[] =
{
	"Invalid entry",				/* 0x0 */
	"Alignment fault",				/* 0x1 */
	"Debug event",					/* 0x2 */
	"Access flag  - First level",			/* 0x3 */
	"Fault on instruction cache maintenance",	/* 0x4 */
	"Translation fault - First level",		/* 0x5 */
	"Access flag - Second level",			/* 0x6 */
	"Translation fault - Second level",		/* 0x7 */
	"Synchronous external abort",			/* 0x8 */
	"Domain fault - First level",			/* 0x9 */
	"Invalid entry",				/* 0xa */
	"Domain fault - Second level",			/* 0xb */
	"Sync. ext. abort on translation table walk  - First level", /* 0xc */
	"Permission fault - First level",		/* 0xd */
	"Sync. ext. abort on translation table walk  - Second level", /* 0xe */
	"Permission fault - Second level",		/* 0xf */
	"TLB conflict abort",                          	/* 0x10 */
	"Invalid entry",                            	/* 0x11 */
	"Invalid entry",                            	/* 0x12 */
	"Invalid entry",                            	/* 0x13 */
	"Implementation defined Lockdown",          	/* 0x14 */
	"Invalid entry",                            	/* 0x15 */
	"Asynchronous external abort",              	/* 0x16 */
	"Invalid entry",                            	/* 0x17 */
	"Asynchronous parity error on memory access",  	/* 0x18 */
	"Synchronous parity error on memory access", 	/* 0x19 */
	"Implementation defined Coprocessor Abort", 	/* 0x1a */
	"Invalid entry",                            	/* 0x1b */
	"Sync. parity error on translation table walk  - First level",  /* 0x1c */
	"Invalid entry",                            	/* 0x1d */
	"Sync. parity error on translation table walk  - Second level", /* 0x1e */
	"Invalid entry",                            	/* 0x1f */
};

/**
 * @brief Prefetch abort fault strings
 */
static const char* ifsr_short_desc_error[] =
{
	"Invalid entry",				/* 0x0 */
	"Invalid entry",				/* 0x1 */
	"Debug event",					/* 0x2 */
	"Access flag  - First level",			/* 0x3 */
	"Invalid entry",				/* 0x4 */
	"Translation fault - First level",		/* 0x5 */
	"Access flag - Second level",			/* 0x6 */
	"Translation fault - Second level",		/* 0x7 */
	"Synchronous external abort",			/* 0x8 */
	"Domain fault - First level",			/* 0x9 */
	"Invalid entry",				/* 0xa */
	"Domain fault - Second level",			/* 0xb */
	"Sync. ext. abort on translation table walk  - First level", /* 0xc */
	"Permission fault - First level",		/* 0xd */
	"Sync. ext. abort on translation table walk  - Second level", /* 0xe */
	"Permission fault - Second level",		/* 0xf */
	"TLB conflict abort",                          	/* 0x10 */
	"Invalid entry",                            	/* 0x11 */
	"Invalid entry",                            	/* 0x12 */
	"Invalid entry",                            	/* 0x13 */
	"Implementation defined Lockdown",          	/* 0x14 */
	"Invalid entry",                            	/* 0x15 */
	"Invalid entry",              			/* 0x16 */
	"Invalid entry",                            	/* 0x17 */
	"Invalid entry",  				/* 0x18 */
	"Synchronous parity error on memory access", 	/* 0x19 */
	"Implementation defined Coprocessor Abort", 	/* 0x1a */
	"Invalid entry",                            	/* 0x1b */
	"Sync. parity error on translation table walk  - First level",  /* 0x1c */
	"Invalid entry",                            	/* 0x1d */
	"Sync. parity error on translation table walk  - Second level",  /* 0x1e */
	"Invalid entry",                            	/* 0x1f */
};


/**
 * @brief Print data abort information
 */
void print_data_abort()
{
	sw_uint dfsr = get_cp15_dfsr();
	sw_uint dfar = get_cp15_dfar();
	sw_uint fault_status = (dfsr & 0xF) | ((dfsr & 0x400) >> 6);

	sw_printk("SW: Data abort Address: %08x\n", dfar);
	sw_printk("reason: ");
	sw_printk((char*)dfsr_short_desc_error[fault_status]);
	sw_printk("\n");
	sw_printk("fault status value = 0x%x\n", fault_status);
	sw_printk("Domain = 0x%x\n", ((dfsr & FSR_DOMAIN_MASK) >> 4));
	sw_printk("WnR = 0x%x\n", ((dfsr & FSR_WNR_MASK) >> 11));					
	sw_printk("ExT = 0x%x\n", ((dfsr & FSR_EXT_MASK) >> 12));
}

/**
 * @brief Print prefetch abort information
 */
void print_prefetch_abort()
{
	sw_uint ifsr = get_cp15_ifsr();
	sw_uint ifar = get_cp15_ifar();
	sw_uint fault_status = (ifsr & 0xF) | ((ifsr & 0x400) >> 6);

	sw_printk("SW: Prefetch abort Address: %08x\n", ifar);
	sw_printk("reason: ");
	sw_printk((char*)ifsr_short_desc_error[fault_status]);
	sw_printk("\n");
	sw_printk("fault status value = 0x%x\n", fault_status);
	sw_printk("ExT = 0x%x\n", ((ifsr & FSR_EXT_MASK) >> 12));
}

#ifdef CONFIG_SW_DEDICATED_TEE

/**
 * @brief 
 */
void smp_monitor_fiq_c_handler(void)
{
	sw_int cpu_id = SW_SECONDARY_CORE;
	sw_uint interrupt;
	interrupt = sw_get_current_active_irq();

	if(interrupt != 1023 && interrupt != 1022) {
		sw_gic_ack_irq(interrupt);
		if(interrupt == SEC_SGI_TO_SECONDARY_CORE) {
			valid_return_params_flag[cpu_id] = 1;
		}
	}

	asm volatile("mov r0, #0\n\t");
	asm volatile("dsb");
}
#endif

/**
 * @brief FIQ 'C' handler
 */
void fiq_c_handler(void) 
{
	sw_int cpu_id = SW_SECONDARY_CORE;
	sw_int iter;
	sw_uint interrupt;
	interrupt = sw_get_current_active_irq();

	if(interrupt != 1023 && interrupt != 1022) {
		sw_gic_ack_irq(interrupt);
#ifndef TIMER_NOT_DEFINED
		if((interrupt == FREE_RUNNING_TIMER_IRQ) ||
				(interrupt == TICK_TIMER_IRQ)) {

			secure_timer_irq_handler(interrupt);
			goto ret_int;
		}
#endif

		if(interrupt == SEC_SGI_TO_PRIMARY_CORE){

			for(iter = 0 ; iter < PARAM_STACK_SIZE; iter++) {
				params_stack[iter] = params_smp_stack[cpu_id][iter];
			}
			valid_params_flag = 1;
		}
		else{
			invoke_irq_handler(interrupt,NULL);
		}

	}
ret_int:
	asm volatile("mov r0, #0\n\t");
	asm volatile("dsb");
}

/**
 * @brief 
 */
#ifdef CONFIG_SW_DEDICATED_TEE

/**
 * @brief 
 */
void notify_smp_core(void)
{
	sw_int iter;
	sw_int cpu_id = SW_SECONDARY_CORE;
	for(iter = 0 ; iter < PARAM_OUT_STACK_SIZE; iter++) {
		params_out_smp_stack[cpu_id][iter] = params_out_stack[iter];
	}

	sw_generate_soft_int(SEC_SGI_TO_SECONDARY_CORE, SGI_TGT_CPU0);
}
#endif

/**
 * @brief Data abort handler
 *
 * @param regs: Pointer to task context
 */
void tee_data_abort_handler(struct swi_temp_regs *regs)
{
	struct sw_task *task;
	sw_uint dfar;
	sw_uint sp_residual;
	task = get_current_task();
	if(task) {
		dfar = get_cp15_dfar();
		sp_residual = ((sw_uint)task->task_sp) - PAGE_SIZE;
		
		if(dfar >= sp_residual && dfar < (sw_uint)task->task_sp) {
			sw_printk("stack over flow detected for task id 0x%x and task name %s \n", 
				task->task_id, task->name);
			task->tls->ret_val = SMC_ERROR;
			handle_task_return(task->task_id, task->tls);
			return;
		}
	}
	

	sw_printk("SW: data_abort_handler: dabt at @ pc %08x\n", regs->lr);

	print_data_abort();
	sw_uint fault_status = (get_cp15_dfsr() & 0xF) | 
				((get_cp15_dfsr() & 0x400) >> 6);
	switch(fault_status) {
		case dfs_translation_section:
		case dfs_translation_page: {
			tee_panic("Translation fault \n");
			break;
		}
		case dfs_alignment_fault:
		case dfs_debug_event:
		case dfs_access_flag_section:
		case dfs_icache_maintenance:
		case dfs_access_flag_page:
		case dfs_sync_external_abt:
		case dfs_domain_section:
		case dfs_domain_page:
		case dfs_translation_table_walk_lvl1_sync_ext_abt:
		case dfs_permission_section:
		case dfs_translation_table_walk_lvl2_sync_ext_abt:
		case dfs_permission_page:
		case dfs_imp_dep_lockdown:
		case dfs_async_external_abt:
		case dfs_mem_access_async_parity_err:
		case dfs_mem_access_async_parity_err2:
		case dfs_imp_dep_coprocessor_abort:
		case dfs_translation_table_walk_lvl1_sync_parity_err:
		case dfs_translation_table_walk_lvl2_sync_parity_err:
		default:
			tee_panic("Invalid data abort condition\n");
			break;
	}

	tee_panic("Data abort panic\n");
}


/**
 * @brief Prefetch abor handler
 */
void tee_prefetch_abort_handler(void)
{
	sw_uint ifsr = get_cp15_ifsr();
	sw_uint fault_status = (ifsr & 0xF) | ((ifsr & 0x400) >> 6);

	print_prefetch_abort();
	switch(fault_status) {
		case ifs_translation_fault_page:
		case ifs_debug_event:
		case ifs_access_flag_fault_section:
		case ifs_translation_fault_section:
		case ifs_access_flag_fault_page:
		case ifs_synchronous_external_abort:
		case ifs_domain_fault_section:
		case ifs_domain_fault_page:
		case ifs_translation_table_walk_lvl1_sync_ext_abt:
		case ifs_permission_fault_section:
		case ifs_translation_table_walk_lvl2_sync_ext_abt:
		case ifs_permission_fault_page:
		case ifs_imp_dep_lockdown:
		case ifs_memory_access_sync_parity_err:
		case ifs_imp_dep_coprocessor_abort:
		case ifs_translation_table_walk_lvl1_sync_parity_err:
		case ifs_translation_table_walk_lvl2_sync_parity_err:
		default:
			tee_panic("Invalid prefetch abort.");
	}
	tee_panic("Prefetch abort panic\n");
}

/**
 * @brief Undefined handler 
 */
void tee_undefined_handler(void)
{
	tee_panic("Undefined handler panic\n");
}


/**
 * @brief SWI handler
 *
 * 0xcccc: IPI notification 
 * 
 * @param swi_id: SWI ID
 * @param regs: Pointer to the task context
 */
void tee_swi_handler(void *reg0, void *reg1, void* reg2, void *reg3)
{
	switch((sw_uint)reg0) {
		case SW_SYSCALL_SCHEDULE:
			temp_swi_regs = (struct swi_temp_regs *)reg1;
			scheduler();
			break;
		default:
			invoke_syscall_handler((sw_uint)reg0, (struct swi_temp_regs *)reg1);
			break;    
	}
}


/**
 * @brief TZ API error handler
 *
 * @param 
 */
void smc_error_handler(void)
{
	tee_panic("SW Error: TZ API calls cannot be routed directly to second core\n");
}
