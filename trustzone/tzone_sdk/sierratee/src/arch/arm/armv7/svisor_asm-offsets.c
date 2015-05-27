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
#include <cpu_vcore.h>

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
	GEN_SYM(VCORE_MIDR,     pos(struct vcore, midr));
	GEN_SYM(VCORE_MPIDR,     pos(struct vcore, mpidr));
	GEN_SYM(VCORE_REGS,           pos(struct vcore, vc_regs_core));
	GEN_SYM(VCORE_USR_REGS,     pos(struct vcore, vc_regs_core));
	GEN_SYM(VCORE_SVC_REGS,     pos(struct vcore, vc_regs_core.spsr_svc));
	GEN_SYM(VCORE_ABT_REGS,     pos(struct vcore, vc_regs_core.spsr_abt));
	GEN_SYM(VCORE_UND_REGS,     pos(struct vcore, vc_regs_core.spsr_undef));
	GEN_SYM(VCORE_IRQ_REGS,     pos(struct vcore, vc_regs_core.spsr_irq));
	GEN_SYM(VCORE_FIQ_REGS,     pos(struct vcore, vc_regs_core.r8_fiq));
	GEN_SYM(VCORE_PC,       pos(struct vcore, vc_regs_core.pc));
	GEN_SYM(VCORE_CPSR,     pos(struct vcore, vc_regs_core.cpsr));
	GEN_SYM(VCORE_USR_SP,       pos(struct vcore, vc_regs_core.r13));
	GEN_SYM(VCORE_USR_LR,     pos(struct vcore, vc_regs_core.lr));
	return SW_OK;
}
