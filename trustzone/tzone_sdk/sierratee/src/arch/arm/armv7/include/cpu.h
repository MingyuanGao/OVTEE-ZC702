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
 * Header for cpu info implementation
 */

#ifndef __CPU_ARCH__CPU_H__
#define __CPU_ARCH__CPU_H__

#include <sw_types.h>
#include <cpu_asm.h>

#define CACHELINE_SIZE 32

#define MPIDR_LEVEL_BITS 8
#define MPIDR_LEVEL_MASK ((1 << MPIDR_LEVEL_BITS) - 1)

#define ID_PFR1_GT_MASK  0x000F0000  /* Generic Timer interface support */
#define ID_PFR1_GT_v1    0x00010000

#define be_32_to_le32(x) \
	((sw_uint)( \
		(((sw_uint)(x) & (sw_uint)0x000000ffUL) << 24) | \
		(((sw_uint)(x) & (sw_uint)0x0000ff00UL) <<  8) | \
		(((sw_uint)(x) & (sw_uint)0x00ff0000UL) >>  8) | \
		(((sw_uint)(x) & (sw_uint)0xff000000UL) >> 24) ))

/**
* @brief 
*/
static inline void invoke_dsb(void)
{
	asm volatile("dsb");
}

/**
* @brief 
*/
static inline void invoke_dmb(void)
{
	asm volatile("dmb");
}

/**
* @brief 
*/
static inline void invoke_isb(void)
{
	asm volatile("isb");
}

/*
 * @brief
 */
void jump_to_sys_mode(void);

/**
* @brief 
*
* @return 
*/
sw_uint get_cp15_mpid(void);

/**
* @brief 
*
* @return 
*/
sw_uint get_cp15_sctlr(void);

/**
* @brief 
*
* @param val
*/
void set_cp15_sctlr(sw_uint val);

/**
* @brief 
*
* @return 
*/
sw_uint get_cp15_dfsr(void);

/**
* @brief 
*
* @return 
*/
sw_uint get_cp15_dfar(void);

/**
* @brief 
*
* @return 
*/
sw_uint get_cp15_ifsr(void);

/**
* @brief 
*
* @return 
*/
sw_uint get_cp15_ifar(void);

/**
* @brief 
*
* @param val
*/
void set_cp15_ttbcr(sw_uint val);

/**
* @brief 
*
* @param val
*/
void set_cp15_asid(sw_uint asid);

/**
* @brief 
*
* @param val
*/
sw_uint get_cp15_ttbcr(void);

/**
* @brief 
*
* @return 
*/
sw_uint get_cp15_dacr(void);

/**
* @brief 
*
* @param val
*/
void set_cp15_dacr(sw_uint val);

/**
* @brief 
*
* @return 
*/
sw_uint get_cp15_ttbr0(void);

/**
* @brief 
*
* @param val
*/
void set_cp15_ttbr0(sw_uint val);

/**
* @brief 
*
* @return 
*/
sw_uint get_cp15_ttbr1(void);

/**
* @brief 
*
* @param val
*/
void set_cp15_ttbr1(sw_uint val);


/**
* @brief 
*/
void branch_predictor_inv_all(void);

/**
* @brief 
*/
void icache_inv_all(void);


void busy_loop(void);

/**
 * @brief 
 */
void enable_l1_cache(void);

/**
 * @brief 
 *
 * @return 
 */
sw_uint cpu_save_irq_state(void);

/**
 * @brief 
 *
 * @param cpsr_irq
 */
void cpu_restore_irq_state(sw_uint cpsr_irq);


/**
 * @brief 
 */
void clear_data_cache(void);

/**
 * @brief 
 *
 * @return 
 */
sw_uint get_cpu_id(void);


/**
 * @brief 
 *
 * @return 
 */
void init_cpu_state(void);

/**
 * @brief 
 */
void enable_branch_predictor(void);

/**
 * @brief 
 */
void tee_irq_enable(void);

/**
 * @brief 
 */
void tee_irq_disable(void);

/**
 * @brief 
 */
void start_secondary_core(void);

#ifdef CONFIG_SVISOR_SUPPORT
/**
 * @brief 
 *
 * @return 
 */
sw_uint get_cp15_hvbar(void);

/**
 * @brief 
 *
 * @param sw_uint
 */
void set_cp15_hvbar(sw_uint);
/**
 * @brief 
 *
 * @return 
 */
sw_uint get_cp15_hcr(void);

/**
 * @brief 
 *
 * @param sw_uint
 */
void set_cp15_hcr(sw_uint);

/**
 * @brief 
 */
void svisor_irq_disable(void);

/**
 * @brief 
 */
void svisor_irq_enable(void);
#endif

#endif 
