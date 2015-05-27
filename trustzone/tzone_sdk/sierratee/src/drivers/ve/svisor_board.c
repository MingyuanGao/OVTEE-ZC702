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
 * versatile express RS1 board configuration
 */

#include <sw_board.h>
#include <sw_board_asm.h>
#include <page_table.h>
#include <arm_asm.h>
#include <lpae_page_table_def.h>
#include <cpu_vcore.h>

static const struct devmap ve_guest_devmap[] =  {
	{
		.dv_va = UART0_ADDR & 0xfffff000,
		.dv_pa = UART0_ADDR & 0xfffff000,
		.dv_size = 0x10000,
			
	},
#if 0	
	{
		.dv_va = UART1_ADDR & 0xfffff000,
		.dv_pa = UART1_ADDR & 0xfffff000,
		.dv_size = 0x10000,
			
	},
	{
		.dv_va = TIMER0_BASE & 0xfffff000,
		.dv_pa = TIMER0_BASE & 0xfffff000,
		.dv_size = 0x10000,
			
	},
#endif	
	{
		.dv_va = SYSCTL_BASE & 0xfffff000,
		.dv_pa = SYSCTL_BASE & 0xfffff000,
		.dv_size = 0x10000,
			
	},
#if 0	
	{
		.dv_va = VE_RS1_L2CC & 0xfffff000,
		.dv_pa = VE_RS1_L2CC & 0xfffff000,
		.dv_size = 0x10000,
			
	},
	{
		.dv_va = VE_RS1_MPIC & 0xfffff000,
		.dv_pa = VE_RS1_MPIC & 0xfffff000,
		.dv_size = 0x10000,
			
	},

	
	{
		.dv_va = GIC_DIST & 0xfffff000,
		//.dv_pa = GIC_DIST & 0xfffff000,
		.dv_pa = 0xFFFF9000 & 0xfffff000,
		.dv_size = 0x10000,
			
	},
#endif	

	{
		.dv_va = GIC_CPU & 0xfffff000,
		.dv_pa = GIC_VCPU & 0xfffff000,
		.dv_size = 0x1000,
			
	},
#if 1	
	{
		.dv_va = 0x14000000 & 0xfffff000,
		.dv_pa = 0x14000000 & 0xfffff000,
		.dv_size = 0x4000000,
			
	},
	{
		.dv_va = 0x18000000 & 0xfffff000,
		.dv_pa = 0x18000000 & 0xfffff000,
		.dv_size = 0x4000000,
			
	},
#endif	
	{
		.dv_va = 0x1C000000 & 0xfffff000,
		.dv_pa = 0x1C000000 & 0xfffff000,
		.dv_size = 0x4000000,
			
	},
#if 0
	{
		.dv_va = VE_CLCD_BASE & 0xfffff000,
		.dv_pa = VE_CLCD_BASE & 0xfffff000,
		.dv_size = 0x10000,
			
	},
#endif	
	{
		.dv_va = 0x08000000 & 0xfffff000,
		.dv_pa = 0x08000000 & 0xfffff000,
		.dv_size = 0x4000000,
			
	},
	{
		.dv_va = 0x0c000000 & 0xfffff000,
		.dv_pa = 0x0c000000 & 0xfffff000,
		.dv_size = 0x4000000,
			
	},
	{0}
};


/**
* @brief 
*/
void svisor_board_init(void)
{

}
void test_interrupt(int irq, void *data)
{

}

extern void maintenance_interrupt(int irq, void *data);
extern void svisor_timer_irq_handler(int irq, void* data);
//extern void sys_timer_dummy_handler(int irq, void* data);

void map_hypervisor_irq(void)
{

	/* Maintenance interrupt */
	map_irq_to_hypervisor(VGIC_MAINTENANCE_IRQ, 1u << 0 /* smp_processor_id */);
	register_irq(VGIC_MAINTENANCE_IRQ, maintenance_interrupt, 
					"irq-maintenance", NULL);

#if 1
	/* Hypervisor Timer */
	map_irq_to_hypervisor(HYPERVISOR_TIMER_IRQ, 1u << 0 /* smp_processor_id */);    
	register_irq(HYPERVISOR_TIMER_IRQ, svisor_timer_irq_handler, 
	                  "svisor-timer", NULL);
#endif
}

void map_guest_irq(struct vcore *v)
{
	/* Fix Me: Better framework fo partioning */
	if (v->guest_no == 0) {
		map_irq_to_guest(34, v->guest_no, "timer0", 1);
		map_irq_to_guest(38, v->guest_no, "uart1", 1);
		map_irq_to_guest(41, v->guest_no, "mmc0-1", 1);
		map_irq_to_guest(42, v->guest_no, "mmc0-2", 1);
#if 0
		map_irq_to_guest(44, v->guest_no, "keyboard", 1);
		map_irq_to_guest(45, v->guest_no, "mouse", 1);
		map_irq_to_guest(46, v->guest_no, "lcd", 1);  
		map_irq_to_guest(47, v->guest_no, "eth", 1);
#endif
		map_irq_to_guest(73, v->guest_no, "vmfs", 1);
	} 
	if (v->guest_no == 1) {
		map_irq_to_guest(30, v->guest_no, "system-timer", 1);
		map_irq_to_guest(37, v->guest_no, "uart0", 1);
	}    
}

void svisor_guest_dev_init(void)
{
	struct devmap *dt = ve_guest_devmap;
	struct ld_params params;
	/* Temp code */
	/* Map all the passthrough IO device in Stage-2 page table */
	while (*(sw_uint *)dt != 0) {
		params.virt_addr = dt->dv_va;
		params.phys_addr = dt->dv_pa;
		params.size = dt->dv_size;
		params.mem_attr = 0b0001;

		map_stage2_three_level_ld(&params);
		dt++;
	};

	return;
}


/**
 * @brief 
 *      This functions returns the starting address of the 
 *      Svisor RAM
 * @return 
 *      Starting address of the Svisor RAM
 */
sw_phy_addr get_svisor_ram_start_addr(void)
{   
        return SVISOR_RAM_START;
}

/**
 * @brief 
 *      This function returns the End address of the svisor RAM
 * @return 
 *      End address of the svisor RAM
 */
sw_phy_addr get_svisor_ram_end_addr(void)
{
        return SVISOR_RAM_END;
}


void board_generate_virq(void)
{

//	arch_generate_virq();
}

sw_phy_addr get_ram_start_addr(void)
{
	return get_svisor_ram_start_addr();
}

/**
 * @brief 
 *      This function returns the End address of the Secure RAM
 * @return 
 *      End address of the Secure RAM
 */
sw_phy_addr get_ram_end_addr(void)
{
	return get_svisor_ram_end_addr();
}

/**
* @brief 
*/
void board_init(void)
{
}
