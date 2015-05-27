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
 * Global variables for TEE
 */

#include <sw_types.h>
#include <cpu_data.h>

/**
 * @brief Secure page table align it to 16K
 */
sw_uint tmp_page_table[PAGE_TABLE_ENTRIES]   __attribute__ ((section (".data")))
__attribute__ ((aligned (16384)));

 
sw_uint secure_page_table[PAGE_TABLE_ENTRIES]   __attribute__ ((section (".data")))
__attribute__ ((aligned (16384)));

/* .bss section variables */
/**
 * @brief User stack
 */
sw_uint tee_user_stack[MAX_CORES][STACK_SIZE/4]	__attribute__ ((aligned (4)));

/**
 * @brief Supervisor stack
 */
sw_uint tee_svc_stack[MAX_CORES][STACK_SIZE/4]	__attribute__ ((aligned (4)));

/**
 * @brief Abort stack
 */
sw_uint tee_abort_stack[MAX_CORES][STACK_SIZE/4]	
__attribute__ ((aligned (4)));

/**
 * @brief Undefined stack
 */
sw_uint tee_undefined_stack[MAX_CORES][STACK_SIZE/4]	
__attribute__ ((aligned (4)));

/**
 * @brief IRQ stack
 */
sw_uint tee_irq_stack[MAX_CORES][STACK_SIZE/4]      __attribute__ ((aligned (4)));

/**
 * @brief FIQ stack
 */
sw_uint tee_fiq_stack[MAX_CORES][STACK_SIZE/4]      __attribute__ ((aligned (4)));

/**
 * @brief Monitor stack
 */
sw_uint tee_monitor_stack[MAX_CORES][STACK_SIZE/4]  __attribute__ ((aligned (4)));

/**
 * @brief Temporary register storage
 */
sw_uint temp_regs[32]	__attribute__ ((section (".bss")));

/**
 * @brief Parameters stack which is used to SMC call parameters
 */
sw_uint params_stack[PARAM_STACK_SIZE] __attribute__ ((section (".bss"))) 
__attribute__ ((aligned (4)));

/**
 * @brief Parameters stack which is used to SMC call parameters 
 *        One-per CPU
 */
sw_uint params_smp_stack[MAX_CORES][PARAM_STACK_SIZE] __attribute__ ((section (".bss"))) 
__attribute__ ((aligned (4)));
/**
 * @brief Parameters out stack which is used store the return value of SMC call
 */
sw_uint params_out_stack[PARAM_OUT_STACK_SIZE] __attribute__ ((section (".bss"))) 
__attribute__ ((aligned (4)));

/**
 * @brief Parameters out stack which is used store the return value of SMC call
 *      One-Per CPU
 */
sw_uint params_out_smp_stack[MAX_CORES][PARAM_OUT_STACK_SIZE] __attribute__ ((section (".bss"))) 
__attribute__ ((aligned (4)));

/**
 * @brief Helps in emulating secure interrupts
 */
sw_uint secure_interrupt_set __attribute__ ((section (".bss")))
__attribute__ ((aligned (4)));


/**
 * @brief Temp registers used in SWI handler
 */
struct swi_temp_regs *temp_swi_regs;

/**
 * @brief Valid params flag 
 */
sw_uint valid_params_flag = 0;

/**
 * @brief valid_return_params_flag 
 */
sw_uint valid_return_params_flag[MAX_CORES];

/**
 * @brief multi core mode
 */
sw_uint multi_core_mode = 0;

extern sw_short_int _SW_CODE_START;
extern sw_short_int _SW_CODE_END;
extern sw_short_int _SW_TEXT_END;
extern sw_short_int _SW_BSS_START;
extern sw_short_int _SW_FS_START;
extern sw_short_int _SW_DATA_START;
extern sw_short_int	_SW_MEM_INFO_START;
extern sw_short_int	_SW_MEM_INFO_END;

extern sw_uint _text_size;
extern sw_uint _data_size;
extern sw_uint _bss_size;
extern sw_uint _init_size;

extern sw_uint _bss_size;
extern sw_uint _mem_info_size;

extern sw_short_int _MOD_INIT_SECTION_START;
extern sw_short_int _MOD_INIT_SECTION_END;
extern sw_short_int _MOD_INIT_PADDING_END;

extern sw_short_int _SW_LIBC_SECTION_START;
extern sw_short_int _SW_LIBC_SECTION_END;
extern sw_uint _libc_size;

extern sw_short_int _SW_LIBC_SYM_TBL_START;
extern sw_short_int _SW_LIBC_SYM_TBL_END;
extern sw_uint _libc_sym_tbl_size;

extern sw_short_int _SW_LIBC_STR_TBL_START;
extern sw_short_int _SW_LIBC_STR_TBL_END;
extern sw_uint _libc_str_tbl_size;

extern sw_short_int _SW_APP_SECTION_START;
extern sw_short_int _SW_APP_TEXT_SECTION_END;
extern sw_short_int _SW_APP_DATA_SECTION_END;
extern sw_short_int _SW_APP_SECTION_END;

extern sw_uint _app_section_size;
extern sw_uint _app_text_size;
extern sw_uint _app_data_size;
extern sw_uint _app_bss_size;

/**
 * @brief 
 *  Function returns the starting address of the secure world code
 *  which is given in the Linker script
 * @return 
 */
sw_uint* get_sw_code_start(void)
{
	return (sw_uint*)&_SW_CODE_START;
}

/**
 * @brief 
 *  Function returns the End address of the secure world code
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_sw_code_end(void)
{
	return (sw_uint*)&_SW_CODE_END;
}

/**
 * @brief 
 *  Function returns the starting address of the text section
 *  which is given in the Linker script
 * @return 
 */
sw_uint* get_sw_text_start(void)
{
	return (sw_uint*)&_SW_CODE_START;
}

/**
 * @brief 
 *  Function returns the size of text section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_sw_text_size(void)
{
	return (int)&_text_size;
}

/**
 * @brief 
 *  Function returns the starting address of the data section
 *  which is given in the Linker script
 * @return 
 */
sw_uint* get_sw_data_start(void)
{
	return (sw_uint*)&_SW_DATA_START;
}

/**
 * @brief 
 *  Function returns the size of data section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_sw_data_size(void)
{
	return (int)&_data_size;
}

/**
 * @brief 
 *  Function returns the starting address of the bss section
 *  which is given in the Linker script
 * @return 
 */
sw_uint* get_sw_bss_start(void)
{
	return (sw_uint*)&_SW_BSS_START;
}

/**
 * @brief 
 *  Function returns the size of bss section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_sw_bss_size(void)
{
	return (int)&_bss_size;
}

/**
 * @brief 
 *      This function returns the start address of the region
 *      which has the module initialization codes
 * @return 
 */

sw_uint* get_mod_init_start_addr(void)
{
	return (sw_uint*)&_MOD_INIT_SECTION_START;
}

/**
 * @brief 
 *      This function returns the start address of the region
 *      which has the initialization codes
 * @return 
 */

sw_uint* get_init_start_addr(void)
{
	return (sw_uint*)&_SW_TEXT_END;
}

/**
 * @brief 
 *      This function returns the end address of the region
 *      which has the module initialization codes
 *
 * @return 
 */
sw_uint* get_mod_init_end_addr(void)
{
	return (sw_uint*)&_MOD_INIT_SECTION_END;
}

/**
 * @brief 
 *      This function returns the size of the region
 *      which has the initialization codes and module initialization lies
 * @return 
 */

int get_init_size(void)
{
	return (int)&_init_size;
}

/**
 * @brief 
 *  Function returns the memory allocator metadata of the secure world code
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_mem_info_start(void)
{
	return (sw_uint*)&_SW_MEM_INFO_START;
}

/**
 * @brief 
 *  Function returns the memory allocator metadata of the secure world code
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_mem_info_end(void)
{
	return (sw_uint*)&_SW_MEM_INFO_END;
}

/**
 * @brief 
 *  Function returns the size of alloc section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_mem_info_size(void)
{
	return (int)&_mem_info_size;
}



/**
 * @brief 
 *  Function returns the libc section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_libc_section_start(void)
{
	return (sw_uint*)&_SW_LIBC_SECTION_START;
}

/**
 * @brief 
 *  Function returns the libc section end address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_libc_section_end(void)
{
	return (sw_uint*)&_SW_LIBC_SECTION_END;
}

/**
 * @brief 
 *  Function returns the size of libc section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_libc_size(void)
{
	return (int)&_libc_size;
}


/**
 * @brief 
 *  Function returns the libc section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_libc_sym_start(void)
{
	return (sw_uint*)&_SW_LIBC_SYM_TBL_START;
}

/**
 * @brief 
 *  Function returns the libc section end address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_libc_sym_end(void)
{
	return (sw_uint*)&_SW_LIBC_SYM_TBL_END;
}

/**
 * @brief 
 *  Function returns the size of libc section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_libc_sym_size(void)
{
	return (int)&_libc_sym_tbl_size;
}

/**
 * @brief 
 *  Function returns the libc section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_libc_str_start(void)
{
	return (sw_uint*)&_SW_LIBC_STR_TBL_START;
}

/**
 * @brief 
 *  Function returns the libc section end address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_libc_str_end(void)
{
	return (sw_uint*)&_SW_LIBC_STR_TBL_END;
}

/**
 * @brief 
 *  Function returns the size of libc section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_libc_str_size(void)
{
	return (int)&_libc_str_tbl_size;
}

/**
 * @brief 
 *  Function returns the user application section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_app_section_start(void)
{
	return (sw_uint*)&_SW_APP_SECTION_START;
}

/**
 * @brief 
 *  Function returns the user application section end address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_app_section_end(void)
{
	return (sw_uint*)&_SW_APP_SECTION_END;
}

/**
 * @brief 
 *  Function returns the size of user application section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_app_section_size(void)
{
	return (int)&_app_section_size;
}


/**
 * @brief 
 *  Function returns the user application text section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_app_text_start(void)
{
	return (sw_uint*)&_SW_APP_SECTION_START;
}

/**
 * @brief 
 *  Function returns the size of user application text section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_app_text_size(void)
{
	return (int)&_app_text_size;
}


/**
 * @brief 
 *  Function returns the user application data section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_app_data_start(void)
{
	return (sw_uint*)&_SW_APP_TEXT_SECTION_END;
}

/**
 * @brief 
 *  Function returns the size of user application data section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_app_data_size(void)
{
	return (int)&_app_data_size;
}


/**
 * @brief 
 *  Function returns the user application bss section start address
 *  which is given in the Linker script
 *
 * @return 
 */
sw_uint* get_app_bss_start(void)
{
	return (sw_uint*)&_SW_APP_DATA_SECTION_END;
}

/**
 * @brief 
 *  Function returns the size of user application bss section
 *  which is given in the Linker script
 *
 * @return 
 */
int get_app_bss_size(void)
{
	return (int)&_app_bss_size;
}
