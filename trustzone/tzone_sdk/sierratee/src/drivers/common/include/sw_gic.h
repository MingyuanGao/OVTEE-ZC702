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

#ifndef __SW_GIC_H_
#define __SW_GIC_H_

#include <sw_types.h>
#include <system_context.h> /* tzhyp switching support */

/*! Distributor Register Offsets */
#define GIC_DIST_CTRL_OFF					0x000	
#define GIC_DIST_ICTRL_TYPE_OFF				0x004
#define GIC_DIST_INT_GRP_BASE_OFF			0x080
#define GIC_DIST_INT_SET_EN_BASE_OFF	 	0x100
#define GIC_DIST_INT_CLR_EN_BASE_OFF		0x180
#define GIC_DIST_INT_SET_PEND_BASE_OFF 		0x200
#define GIC_DIST_INT_CLR_PEND_BASE_OFF		0x280
#define GIC_DIST_INT_SET_ACTV_BASE_OFF 		0x300
#define GIC_DIST_INT_CLR_ACTV_BASE_OFF		0x380
#define GIC_DIST_INT_PRI_BASE_OFF			0x400
#define GIC_DIST_INT_PROC_TARG_BASE_OFF		0x800	
#define GIC_DIST_INT_CONF_BASE_OFF			0xC00
#define GIC_DIST_SOFT_GEN_INT_BASE_OFF		0xF00

/*! CPU Interface Register Offsets */
#define GIC_CPU_CTRL_OFF					0x000
#define GIC_CPU_PRI_MASK_OFF				0x004
#define GIC_CPU_BIN_POINT_OFF				0x008
#define GIC_CPU_INT_ACK_OFF					0x00C
#define GIC_CPU_END_OF_INT_OFF				0x010
#define GIC_CPU_RUN_PRIO_OFF				0x014
#define GIC_CPU_HIGH_PRI_PEND_INT_OFF		0x018

/*! Distributor Register Values */
#define GIC_DIST_SOFTINT_NSATT_SET     (1 << 15)
    
#define GIC_DIST_SOFTINT_TAR_CORE     (1 << 16)

#define GIC_DIST_SOFTINT_TARGET(n) ((GIC_DIST_SOFTINT_TAR_CORE) << n)

#ifdef SCHEDULE_HIGH_PRIORITY_GUEST
int is_guest_irq_active(sw_uint guest);
#endif

void sw_gic_mask_interrupt(int int_num);

void sw_gic_unmask_interrupt(int int_num);

sw_uint sw_get_current_active_irq(void);

void sw_gic_ack_irq(int int_num);

void sw_gic_distributor_init(void);

void sw_gic_cpu_interface_init(void);

void sw_generate_soft_int(sw_uint int_num, sw_uint dest_core);

void sw_gic_dist_save_regs(struct gic_context* gc_ctxt);

void sw_gic_dist_restore_regs(struct gic_context* gc_ctxt);

#endif
