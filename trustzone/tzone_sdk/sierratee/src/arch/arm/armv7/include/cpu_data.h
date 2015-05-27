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
 * TEE Global variables declaration
 */

#ifndef __OTZ_CPU_DATA_H__
#define __OTZ_CPU_DATA_H__

#include <page_table.h>
#include <cpu_asm.h>
#include <sw_board.h>
#include <sw_mcore.h>

#define TASK_STACK_SIZE 4096
#define SW_PRIMARY_CORE     0x1
#define SW_SECONDARY_CORE   0x0

/**
 * @brief Structure to store register in SWI handler
 */
struct swi_temp_regs {
	/*! spsr */
	sw_uint spsr;
	/*! regs r0 - r12 */
	sw_uint regs[13];   
	/*! link register */
	sw_uint lr;         
};


extern sw_uint _SW_KSYMTAB;
extern sw_uint _SW_KSYMTAB_END;

/**
 * @brief Secure page table
 */
extern sw_uint secure_page_table[PAGE_TABLE_ENTRIES];

extern sw_uint tmp_page_table[PAGE_TABLE_ENTRIES];


/**
 * @brief User stack
 */
extern sw_uint tee_user_stack[MAX_CORES][STACK_SIZE/4];

/**
 * @brief Supervisor stack
 */
extern sw_uint tee_svc_stack[MAX_CORES][STACK_SIZE/4];

/**
 * @brief Abort stack
 */
extern sw_uint tee_abort_stack[MAX_CORES][STACK_SIZE/4];

/**
 * @brief Undefined stack
 */
extern sw_uint tee_undefined_stack[MAX_CORES][STACK_SIZE/4];

/**
 * @brief IRQ stack
 */
extern sw_uint tee_irq_stack[MAX_CORES][STACK_SIZE/4];

/**
 * @brief FIQ stack
 */
extern sw_uint tee_fiq_stack[MAX_CORES][STACK_SIZE/4];

/**
 * @brief Monitor stack
 */
extern sw_uint tee_monitor_stack[MAX_CORES][STACK_SIZE/4];

/**
 * @brief Parameters stack which is used to SMC call parameters
 */
extern sw_uint params_stack[PARAM_STACK_SIZE];

/**
 * @brief 
 */
extern sw_uint params_smp_stack[MAX_CORES][PARAM_STACK_SIZE];

/**
 * @brief Helps in emulating secure interrupts
 */
extern sw_uint secure_interrupt_set;

/**
 * @brief Temporary register storage
 */
extern sw_uint temp_regs[32];

/**
 * @brief Parameters out stack which is used store the return value of SMC call
 */
extern sw_uint params_out_stack[PARAM_OUT_STACK_SIZE];

/**
 * @brief Parameters out stack which is used store the return value of SMC call
 * One Per CPU
 */
extern sw_uint params_out_smp_stack[MAX_CORES][PARAM_OUT_STACK_SIZE];

/**
 * @brief Temp registers used in SWI handler
 */
extern struct swi_temp_regs *temp_swi_regs;


/**
 * @brief Valid params flag
 */
extern sw_uint valid_params_flag;

/**
 * @brief Valid return params flag
 */
extern sw_uint valid_return_params_flag[MAX_CORES];

/**
 * @brief multi core mode
 */
extern sw_uint multi_core_mode;

/**
 * @brief 
 *  This function returns the start of the virtual address
 *  of the secure world from the linker script
 * @return 
 */
sw_uint* get_sw_code_start(void);

/**
 * @brief 
 *  This function returns the end address(virtual)
 *  of the secure world code with the help of the linker script
 *
 * @return 
 */
sw_uint* get_sw_code_end(void);

/**
 * @brief 
 *  Function returns the starting address of the text section
 *  which is given in the Linker script
 * @return 
 */
sw_uint* get_sw_text_start(void);

/**
 * @brief 
 *  Function returns the size of text section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_sw_text_size(void);

/**
 * @brief 
 *   Function returns the start of the file system
 *   which is given in the Linker script
 * @return 
 */
sw_uint* get_sw_fs_start(void);

/**
 * @brief 
 *  Function returns the starting address of the data section
 *  which is given in the Linker script
 * @return 
 */
sw_uint* get_sw_data_start(void);

/**
 * @brief 
 *  Function returns the size of data section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_sw_data_size(void);

/**
 * @brief 
 *  Function returns the starting address of the bss section
 *  which is given in the Linker script
 * @return 
 */
sw_uint* get_sw_bss_start(void);

/**
 * @brief 
 *  Function returns the size of bss section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_sw_bss_size(void);

/**
 * @brief 
 *      This function returns the start address of the region
 *      which has the module initialization codes
 * @return 
 */

sw_uint* get_mod_init_start_addr(void);

/**
 * @brief 
 *      This function returns the start address of the region
 *      which has the initialization codes
 * @return 
 */
sw_vir_addr* get_init_start_addr(void);

/**
 * @brief 
 *      This function returns the end address of the region
 *      which has the module initialization codes
 *
 * @return 
 */
sw_uint* get_mod_init_end_addr(void);


/**
 * @brief 
 *      This function returns the size of the region
 *      which has the initialization codes and module initialization lies
 * @return 
 */
int get_init_size(void);

/**
 * @brief 
 *  Function returns the memory allocator metadata of the secure world code
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_mem_info_start(void);

/**
 * @brief 
 *  Function returns the memory allocator metadata of the secure world code
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_mem_info_end(void);

/**
 * @brief 
 *  Function returns the size of alloc section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_mem_info_size(void);

/**
 * @brief 
 *  Function returns the libc section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_libc_section_start(void);

/**
 * @brief 
 *  Function returns the libc section end address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_libc_section_end(void);

/**
 * @brief 
 *  Function returns the size of libc section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_libc_size(void);

/**
 * @brief 
 *  Function returns the user application text section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_app_text_start(void);

/**
 * @brief 
 *  Function returns the size of user application text section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_app_text_size(void);


/**
 * @brief 
 *  Function returns the user application data section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_app_data_start(void);
/**
 * @brief 
 *  Function returns the size of user application data section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_app_data_size(void);

/**
 * @brief 
 *  Function returns the user application bss section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_app_bss_start(void);
/**
 * @brief 
 *  Function returns the size of user application bss section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_app_bss_size(void);

#endif 
