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


#include <sw_gic.h>
#include <sw_board.h>
#include <sw_io.h>
#include <sw_debug.h>

/**
* @brief
*  Structure Holding the information about CPU 
*/
struct gic_info {
	sw_uint base_irq;
	sw_uint max_irq_supported;
	sw_uint dist_regs_base_addr;
	sw_uint cpu_intface_base_addr[MAX_CORES];;
};

static struct gic_info sw_gic_info;

/**
 * @brief 
 *		Function to write to the GIC distributor register
 * @param value
 *  	The value to be written to the gic distribution register
 * @param offset
 *		The offset of the register from the base address of the 
 * GIC Distributor  
 */
static inline void sw_write_to_gic_dist(sw_uint value, sw_uint offset)
{
	sw_writel(value, 
			(volatile void*)(sw_gic_info.dist_regs_base_addr + offset));
}

/**
 * @brief 
 *		Function to read from the GIC distributor register
 * @param offset
 *		The offset of the register from the base address of the 
 * GIC Distributor  
 */
static inline sw_uint sw_read_frm_gic_dist(sw_uint offset)
{
	return sw_readl((volatile void*)(sw_gic_info.dist_regs_base_addr + offset));
}

/**
 * @brief 
 *		Function to write to the CPU interface register of the GIC
 * @param value
 *  	The value to be written to the register
 * @param offset
 *		The offset of the register from the base address of the 
 * CPU interface registers
 */
static inline void sw_write_to_gic_cpu_if(sw_uint value, sw_uint offset)
{
	sw_uint cpu_id = get_cpu_id();
	sw_writel(value, 
	(volatile void*)(sw_gic_info.cpu_intface_base_addr[cpu_id] + offset));

}

/**
 * @brief 
 *		Function to read from the CPU interface register of the GIC
 * @param offset
 *		The offset of the register from the base address of the 
 * CPU interface registers
 */
static inline sw_uint sw_read_frm_gic_cpu_if(sw_uint offset)
{
	return sw_readl(
	(volatile void*)(sw_gic_info.cpu_intface_base_addr[get_cpu_id()] + offset));
}

#ifdef SCHEDULE_HIGH_PRIORITY_GUEST
/**
 * @brief 
 *
 * @param guest
 *
 * @return 
 */
int is_guest_irq_active(sw_uint guest)
{
    int  gic_nr = 0;
    sw_uint guest_irq;
    sw_uint val;

    if(guest == HIGH_PRIORITY_GUEST) {
        guest_irq = LOW_PRIORITY_GUEST_UART_IRQ;

		val = sw_read_frm_gic_dist(GIC_DIST_INT_SET_PEND_BASE_OFF
							+ 4 * (guest_irq/32));
		return ((val)? 0 : 1);
    }
    return 0;
}
#endif

/**
* @brief 
*	This function is called to mask a particular Interrupt in the Distributor
* @param int_num
*   The number of the interrupt to be masked
*/
void sw_gic_mask_interrupt(int int_num)
{
	sw_uint int_mask_bit;
	int_mask_bit = 1 << (int_num % 32);

	if(int_num < sw_gic_info.base_irq){
		sw_printk("SW:WARNING - Inavlid int num 0x%x\n",int_num);
		sw_printk("Cannot mask this interrupt\n");
        return;
	}

	int_num -=  sw_gic_info.base_irq;

	sw_write_to_gic_dist(int_mask_bit,
		GIC_DIST_INT_CLR_EN_BASE_OFF + (4 * (int_num / 32)));
	return;
}

/**
* @brief 
*	To Unmask a particular Interrupt in the GIC Distributor
* @param int_num
*	The number of the interrupt to be masked
*/
void sw_gic_unmask_interrupt(int int_num)
{
	sw_uint int_mask_bit;
	int_mask_bit = 1 << (int_num % 32);

	if(int_num < sw_gic_info.base_irq){
		sw_printk("SW:WARNING - Inavlid int num 0x%x\n",int_num);
		sw_printk("Cannot Unmask this interrupt\n");
		return;
	}

	int_num -=  sw_gic_info.base_irq;

	sw_write_to_gic_dist(int_mask_bit,
		GIC_DIST_INT_SET_EN_BASE_OFF + (4 * (int_num / 32)));
	return;
}


/**
* @brief 
*	To get the interrupt number of current active IRQ from the
* GIC CPU Interface
* @return 
*	The interrupt number
*/
sw_uint sw_get_current_active_irq(void)
{
	sw_uint int_num;

	int_num = sw_read_frm_gic_cpu_if(GIC_CPU_INT_ACK_OFF);
	int_num &= 0x3FF;
	int_num += sw_gic_info.base_irq;

	return int_num; 
}

/**
* @brief 
*	To Acknowledge the interrupt by writing to the 
* CPU interface register
* @param int_num
*	The interrupt number to be acknowledged
*/
void sw_gic_ack_irq(int int_num)
{
	if(int_num < sw_gic_info.base_irq){
		sw_printk("SW:WARNING - Inavlid int num 0x%x\n",int_num);
		sw_printk("Cannot acknowledge this interrupt\n");
		return;
	}

	int_num -= sw_gic_info.base_irq;
	sw_write_to_gic_cpu_if(int_num, GIC_CPU_END_OF_INT_OFF);
	return;
}


/**
 * @brief
 * 	Configuring Free running timer and Tick Interrupts
 */
static void sw_config_sec_timer_interrupts(void)
{
	sw_uint int_num, reg_num , reg_val;

	/* Configure Free running Interrupt */
	int_num = FREE_RUNNING_TIMER_IRQ - sw_gic_info.base_irq;
	reg_num = (int_num / 32);
	reg_val = sw_read_frm_gic_dist(
			GIC_DIST_INT_GRP_BASE_OFF + (4 * reg_num));
	reg_val &= (~(1 <<(FREE_RUNNING_TIMER_IRQ % 32)) & 0xFFFFFFFF);
	sw_write_to_gic_dist(reg_val,
		GIC_DIST_INT_GRP_BASE_OFF + (4 * reg_num));

	/* Configure Tick Timer */
	int_num = TICK_TIMER_IRQ - sw_gic_info.base_irq;
	reg_num = (int_num / 32);
	reg_val = sw_read_frm_gic_dist(
			GIC_DIST_INT_GRP_BASE_OFF + (4 * reg_num));
	reg_val &= (~(1 <<(TICK_TIMER_IRQ % 32)) & 0xFFFFFFFF);
	sw_write_to_gic_dist(reg_val,
		GIC_DIST_INT_GRP_BASE_OFF + (4 * reg_num));
}

#ifdef CONFIG_SHELL
/**
 * @brief
 * Configuring Shell Interrupt as Secure Interrupt
 *  The interrupt of the UART used the secure shell is configured as
 * 	a secure Interrupt
 */
static void sw_config_sec_shell_interrupt(void)
{
	sw_uint int_num, reg_num , reg_val;

	/* Configure Shell Interrupt */
	int_num = KEY_PRESS_IRQ - sw_gic_info.base_irq;
	reg_num = (int_num / 32);
	reg_val = sw_read_frm_gic_dist(
			GIC_DIST_INT_GRP_BASE_OFF + (4 * reg_num));
	reg_val &= (~(1 <<(KEY_PRESS_IRQ % 32)) & 0xFFFFFFFF);
	sw_write_to_gic_dist(reg_val,
		GIC_DIST_INT_GRP_BASE_OFF + (4 * reg_num));

}
#endif

/**
 * @brief
 * 	Configuring the nature of Interrupts:
 * 	(i) Edge Triggered or Level Triggered
 */
static void sw_config_trig_level_info(void)
{
	sw_uint iter;
	/* All Interrupts are configured as Level triggered Interrupts */
    for (iter = 32; iter < sw_gic_info.max_irq_supported; iter += 16){
		sw_write_to_gic_dist(0, GIC_DIST_INT_CONF_BASE_OFF + iter * 4 / 16);
	}
}

/**
 * @brief
 *  The Processors to which a particular global interrupt should be 
 * routed is configured here
 * 
 */
static void sw_set_target_cpus_for_glob_ints(void)
{
	sw_uint iter;
	sw_uint cpu_mask;

	cpu_mask = 1 << get_cpu_id();
	cpu_mask |= cpu_mask << 8;
	cpu_mask |= cpu_mask << 16;

	/* All Global interrupts are configures to the current cpu only */
    for (iter = 32; iter < sw_gic_info.max_irq_supported; iter += 4) {
		sw_write_to_gic_dist(cpu_mask,
				GIC_DIST_INT_PROC_TARG_BASE_OFF + iter * 4 / 4);
	}
}

/**
 * @brief 
 *  The priority level of all interrupts are set here
 */
static void sw_set_int_priorities(void)
{
	sw_uint iter;
	/* All the interrupts are set equal priority here */
    for (iter = 32; iter < sw_gic_info.max_irq_supported; iter += 4){
		sw_write_to_gic_dist(0xA0A0A0A0,
				GIC_DIST_INT_PRI_BASE_OFF + iter * 4 / 4);
	}
}
	
/**
 * @brief
 *	 Configuring the Interrupts as Secure and Non-secure
 *   Interrupts as required
 */
static void sw_config_sec_non_sec_int(void)
{
	int iter;
	/* All the global interrupts are configured as Non-secure Interrupts */
	for(iter = 32 ; iter < sw_gic_info.max_irq_supported; iter += 32){
		sw_write_to_gic_dist(0xFFFFFFFF, 
				GIC_DIST_INT_GRP_BASE_OFF + (4 * (iter/32)));
	}

	/* Free running and Timer tick interrupts are configured as 
	   Secure Interrupts */
    sw_config_sec_timer_interrupts();

	/* Configure the interrupt used by secure shell
	   as secure Interrupt */
#ifdef CONFIG_SHELL
    sw_config_sec_shell_interrupt();
#endif

}

/**
* @brief
*	Enabling the required interrupts and disabling rest of the interrupts 
*  which can be turned ON later as and when required
*/
static void sw_init_enab_disb_ints(void)
{
	sw_uint iter;
	/* First disable all interrupts. The interrupts can be turned on 
	   later as and when required */
    for (iter = 32; iter < sw_gic_info.max_irq_supported; iter += 32) {
		sw_write_to_gic_dist(0xFFFFFFFF, 
				GIC_DIST_INT_CLR_EN_BASE_OFF + iter * 4 / 32);
	}
}

/**
 * @brief
 *	GIC supports a maximum of upto 1020 interrupt
 *  Sources. So The upper limit is set here
 */
static void sw_set_max_int_limit(void)
{
	sw_uint num_int_regs, num_ints;
	num_int_regs = sw_read_frm_gic_dist(GIC_DIST_ICTRL_TYPE_OFF) & 0x1F;
	num_ints = (num_int_regs + 1) * 32;
	if(num_ints > max(1020, GIC_NR_IRQS))
		num_ints = max(1020, GIC_NR_IRQS);

	sw_gic_info.max_irq_supported = num_ints;
}

/**
 * @brief
 *  The base address of the CPU interfaces are calculated 
 * and assigned here
 */
static void sw_init_cpu_interface_base(void)
{
	int cpu_id;
	for(cpu_id = 0;cpu_id < MAX_CORES ; cpu_id++)
	{
		sw_gic_info.cpu_intface_base_addr[cpu_id]
			=	GIC_CPU + GIC_BANK_OFFSET * cpu_id;
	}
}

/**
 * @brief
 * 	Initialization of GIC Distributor Registers
 *  In case of Multiprocessor system, this function gets called only once
 * from the primary CPU
 */
void sw_gic_distributor_init(void)
{
	/* Base address of the GIC distributor
	   and the Base interrupt number are initialized here */	
	sw_gic_info.dist_regs_base_addr = GIC_DIST;
	sw_gic_info.base_irq	= (IRQ_GIC_START - 1) & ~31;

	/* GIC Distributor is disabled before configuring the GIC */
	sw_write_to_gic_dist(0, GIC_DIST_CTRL_OFF);
	
	/* Initialize the cpu interface base address */
	sw_init_cpu_interface_base();

	/* Initialize the maximum number of Interrupts
	   supported by the Distributor*/
	sw_set_max_int_limit();

	/* Configure Interrupts as either secure or
	   non-secure interrupts */
	sw_config_sec_non_sec_int();

	/* Configure the trigger level and active
	   state(high/low) of the interrupts */
	sw_config_trig_level_info();

	/* Configure the target cpus for all the global interrupts */
	sw_set_target_cpus_for_glob_ints();

	/*Assign priorities for all global interrupts */
	sw_set_int_priorities();

	/* Enable the required interrupts and disable the other
	   interrupts */
	sw_init_enab_disb_ints();

	/* Enabling both group0 and group1 interrupts*/
	sw_write_to_gic_dist(0x3, GIC_DIST_CTRL_OFF);

	return;
}


/**
* @brief
* 	This function initializes the  cpu interface registers
* 	In case of multi processor CPUs This function should be 
*	called Once from each cpu.
*/
void sw_gic_cpu_interface_init(void)
{
	sw_uint reg_val = 0;

    /* Set the Software generated interrupts (SGI) 15-7 as Secure
	   and the rest of the Per-cpu interrupts(both SGI and PPI)
	   as Non-secure */
	sw_write_to_gic_dist(0xFFFF00FF,
					GIC_DIST_INT_GRP_BASE_OFF);

    /* Enable all SGIs for the CPU */
	sw_write_to_gic_dist(0x0000FFFF,
							GIC_DIST_INT_SET_EN_BASE_OFF);
    /* Disable all PPIs for the CPU */
	sw_write_to_gic_dist(0xFFFF0000,
								GIC_DIST_INT_CLR_EN_BASE_OFF);

    /*Priority should be set higher or equal than 0x80 for non-secure access */
	sw_write_to_gic_cpu_if(0xF0, GIC_CPU_PRI_MASK_OFF);

	/* Signal group0 interrupts from FIQ */
	reg_val |= 1 << 3;
	/* Secure read of Interrupt acknowledge register does not 
	   acknowledges the Non-secure Interrupts */
	reg_val |= 0 << 2; 
	/* Enable group1 Interrupts */
	reg_val |= 1 << 1;
	/* Enable group0 Interrupts */ 
	reg_val |= 1 << 0;
	sw_write_to_gic_cpu_if(reg_val,GIC_CPU_CTRL_OFF);
}


/**
* @brief 
*	To generate a Software Generated Interrupt(SGI).This is used for 
*  Inter core communication.A processor can send interrupts to other
*  Processor to notify an event etc.,.
*
* @param int_num
*		The interrupt number of the SGI to be generated
* @param dest_core
* 		The core number of the CPU to which the interrupt need to be directed
*/
void sw_generate_soft_int(sw_uint int_num, sw_uint dest_core)
{
	sw_uint reg_val, non_sec_int, soft_int_reg;

	int_num -=  sw_gic_info.base_irq;
	reg_val = sw_read_frm_gic_dist(GIC_DIST_INT_GRP_BASE_OFF + (4 * (int_num/32)));

	non_sec_int = reg_val & (1 << (reg_val % 32));

	soft_int_reg = sw_read_frm_gic_dist(GIC_DIST_SOFT_GEN_INT_BASE_OFF);

	soft_int_reg |= int_num & 0xF;
	soft_int_reg |= GIC_DIST_SOFTINT_TARGET((dest_core & 0x7));
	if(non_sec_int){
		soft_int_reg |= GIC_DIST_SOFTINT_NSATT_SET; 
	}

	sw_write_to_gic_dist(soft_int_reg, GIC_DIST_SOFT_GEN_INT_BASE_OFF);
}

/**
 * @brief 
 *		The GIC state is saved. This is especially useful during context 
 *	switches when running multiple guests.The interrupts which are all enabled
 *  for this guests are saved. 
 * @param gc_ctxt
 *  	The structure to which the GIC state is saved 
 */
void sw_gic_dist_save_regs(struct gic_context* gc_ctxt)
{
    int iter;

    for (iter = 0; iter <= GIC_ITLINES; iter++) {
        gc_ctxt->gic_icdiser[iter] =
			sw_read_frm_gic_dist(GIC_DIST_INT_SET_EN_BASE_OFF + iter * 4);
        /* disable the interrupts for a clean restore */
		sw_write_to_gic_dist(gc_ctxt->gic_icdiser[iter],
					 GIC_DIST_INT_CLR_EN_BASE_OFF + iter * 4);
    }

}

/**
 * @brief 
 *		The GIC state is restored. This is especially useful during context
 * Switches when running multiple guests. The interrupt which were all enabled
 * during the previous time instance in which this guest was running are 
 * restored.
 * @param gc_ctxt
 *		The structure from which the context is restored
 */
void sw_gic_dist_restore_regs(struct gic_context* gc_ctxt)
{
    int iter;

    for (iter = 0; iter <= GIC_ITLINES; iter++) {
		sw_write_to_gic_dist(gc_ctxt->gic_icdiser[iter],
					GIC_DIST_INT_SET_EN_BASE_OFF + iter * 4);
    }
}
