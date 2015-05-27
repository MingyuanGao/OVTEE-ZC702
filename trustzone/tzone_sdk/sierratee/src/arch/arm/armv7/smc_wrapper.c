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
 * SMC wrapper functions implementation
 */


#include <sw_config.h>
#include <sw_types.h>
#include <cpu_asm.h>
#include <cpu_data.h>
#include <system_context.h>

/** @defgroup sec_normal_ipi Secure/Non-Secure Communication API
 *  APIs for communicating between Secure and normal world
 *  @{
 */

/**
 * @brief smc system call implementation
 *
 * @param smc_arg: smc system call parameter
 */
void __execute_smc(sw_uint smc_arg)
{
	register sw_uint r0 asm("r0") = smc_arg;
	asm volatile(
#if USE_ARCH_EXTENSION_SEC
			".arch_extension sec\n\t"
#endif
			"smc    #0  @ switch to Non secure world\n"
				: "=r" (r0)
				: "r" (r0));
	return;
}

/**
 * @brief 
 */
void invoke_ns_kernel(void)
{
	register sw_uint r0 asm("r0") = INVOKE_NON_SECURE_KERNEL;
	do {

		/* Execute SMC and go to non-secure world*/
		asm volatile(
#if USE_ARCH_EXTENSION_SEC
				".arch_extension sec\n\t"
#endif
				"smc    #0  @ switch to Non secure world\n" 
				: "=r" (r0)
				: "r" (r0));

	} while (0);

	return;
}

/**
 * @brief 
 *
 * @param retval
 */
void return_secure_api(sw_uint retval)
{
#ifdef CONFIG_FLUSH_SMC_RET
        flush_icache_and_dcache();
#ifdef CONFIG_CACHE_L2X0
        sw_flush_l2cache_all();
#endif
#endif
	register sw_uint r0 asm("r0") = RET_FROM_SECURE_API;
	params_out_stack[0] = retval;
	do {

		/* Execute SMC and go to non-secure world*/
		asm volatile(
#if USE_ARCH_EXTENSION_SEC
				".arch_extension sec\n\t"
#endif
				"smc    #0  @ switch to secure world\n"
				: "=r" (r0)
				: "r" (r0));
	} while (0);

	return;
}

/**
 * @brief 
 */
void smc_nscpu_context_init(void)
{
	register  sw_uint r0 asm ("r0") = TZHYP_NSCPU_CTXT_INIT;
	do {

		/* Execute SMC and go to non-secure world*/
		asm volatile(
#if USE_ARCH_EXTENSION_SEC
				".arch_extension sec\n\t"
#endif
				/*__asmeq("%0", "r0")*/
				/*__asmeq("%1", "r0")*/
				"smc    #0  @ switch to secure world\n"
				: "=r" (r0)
				: "r" (r0));
	} while (0);

	return;
}

void return_non_secure_kernel(void)
{
	register sw_uint r0 asm("r0") = RET_TO_NON_SECURE_KERNEL;
	do {
	/* Execute SMC and return to the non secure path */
	asm volatile(
#if USE_ARCH_EXTENSION_SEC
		".arch_extension sec\n\t"
#endif
		"smc    #0  @ switch to secure world\n"
				: "=r" (r0)
				: "r" (r0));
   	} while (0) ;
	
	return;
}
/** @} */ //sec_normal_ipi 
