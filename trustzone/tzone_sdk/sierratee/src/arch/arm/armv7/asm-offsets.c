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
 * asm-offsets implementation 
 */

#include <sw_types.h>
#include <cpu_task.h>
#include <cpu_data.h>
#include <tzhyp.h>
#include <global.h>
#if defined(CONFIG_NEON_SUPPORT) || defined (OTZONE_ASYNC_NOTIFY_SUPPORT)
#include "system_context.h"
#endif

#define pos(a, b) ((size_t) &((a*)0)->b)
#define GEN_SYM(name, value) \
	asm volatile("\n->" #name " %0 " #value : : "j" (value))
/**
 * @brief 
 *
 * @return 
 */
int main(void)
{
	GEN_SYM(TASK_PC_OFFSET,	    pos(struct sw_task_cpu_regs, pc));
	GEN_SYM(TASK_SPSR_OFFSET,	pos(struct sw_task_cpu_regs, spsr));
	GEN_SYM(TEMP_SWI_REGS_LR_OFFSET,	pos(struct swi_temp_regs, lr));
	GEN_SYM(TEMP_SWI_REGS_SPSR_OFFSET, pos(struct swi_temp_regs, spsr));
	GEN_SYM(TEMP_SWI_REGS_R0_OFFSET,	pos(struct swi_temp_regs, regs));
#ifdef CONFIG_NEON_SUPPORT
	GEN_SYM(NEON_OFFSET, pos(struct system_context, sysctxt_vfp));
#endif
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	GEN_SYM(PENDING_NS_NOTIFY, pos(struct system_context, pending_notify));
#endif	
	GEN_SYM(TZHYP_NS_PREEMPT_FLAG_OFFSET, (pos(struct sw_global, tzhyp_val)
			+ pos(struct tzhyp_values, ns_preempt_flag)));
	GEN_SYM(TZHYP_NS_SWITCH_FLAG_OFFSET, (pos(struct sw_global, tzhyp_val)
			+ pos(struct tzhyp_values, ns_switch_flag)));
	GEN_SYM(G_NS_NOTIFY_PENDING_OFFSET, pos(struct sw_global,
				g_ns_notify_pending));
	GEN_SYM(G_LINUX_RETURN_FLAG_OFFSET, pos(struct sw_global,
				linux_return_flag));
	return SW_OK;
}
