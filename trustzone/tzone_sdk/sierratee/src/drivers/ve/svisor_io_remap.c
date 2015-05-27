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
 * Device VA to PA implementation implementation 
 */

#include <stdio.h>
#include <sw_board_asm.h>

static unsigned int DEVICE_VA_BASE = 0xF8000000;

void mapper(char *str, unsigned int pa, unsigned int size){
	printf("#undef %s\n", str);
	printf("#define %s\t\t\t0x%08x\n", str, pa);
	printf("#define %s_PA\t\t\t0x%08x\n\n", str, pa);
}

void redefiner(char *def, char *val){
	printf("#undef %s\n", def);
	printf("#define %s\t\t\t%s\n\n", def, val);
}

int main(void)
{
	printf("/*\n* Auto-gen file. Dont edit\n*/\n");
	printf("#ifndef _SVISOR_DEVICE_H_\n#define _SVISOR_DEVICE_H_\n\n");
	printf("#include <sw_board_asm.h>\n");
	mapper("VE_FRAME_BASE", VE_FRAME_BASE, VE_FRAME_SIZE);	
	mapper("VE_SYSTEM_REGS", VE_SYSTEM_REGS, VE_SYSTEM_REGS_SIZE);
	mapper("VE_RS1_MPIC", VE_RS1_MPIC, VE_RS1_MPIC_SIZE);
	mapper("VE_RS1_L2CC", VE_RS1_L2CC, VE_RS1_L2CC_SIZE);
	mapper("VE_CLCD_BASE", VE_CLCD_BASE, VE_CLCD_BASE_SIZE);
	mapper("SYSCTL_BASE", SYSCTL_BASE, SYSCTL_BASE_SIZE);
    mapper("UART0_ADDR", UART0_ADDR, UART0_SIZE);
	mapper("UART1_ADDR", UART1_ADDR, UART1_SIZE);
	mapper("UART2_ADDR", UART2_ADDR, UART2_SIZE);
	mapper("UART3_ADDR", UART3_ADDR, UART3_SIZE);
	mapper("TIMER0_BASE", TIMER0_BASE, TIMER0_SIZE);
	mapper("TIMER2_BASE", TIMER2_BASE, TIMER2_SIZE);

	printf("#ifndef CONFIG_CORTEX_A15\n");
	redefiner("GIC_CPU", "(VE_RS1_MPIC + 0x0100)");
	printf("#else\n");
	redefiner("GIC_CPU", "(VE_RS1_MPIC + 0x2000)");
	printf("#endif\n");
	redefiner("GIC_DIST", "(VE_RS1_MPIC + 0x1000)");
	redefiner("VE_RS1_SCU", "(VE_RS1_MPIC + 0x0000)");
	redefiner("VE_RS1_MPCORE_TWD", "(VE_RS1_MPIC + 0x0600)");
	redefiner("VE_SYS_FLAGSSET_ADDR", "(VE_SYSTEM_REGS + 0x30)");
	redefiner("VE_SYS_FLAGSCLR_ADDR", "(VE_SYSTEM_REGS + 0x34)");
	redefiner("FREE_RUNNING_TIMER_BASE", "TIMER2_BASE");
	redefiner("FREE_RUNNING_TIMER_BASE_PA", "TIMER2_BASE_PA");		
	redefiner("FREE_RUNNING_TIMER_BASE_SIZE", "TIMER2_SIZE");		
	redefiner("TICK_TIMER_BASE", "(TIMER2_BASE + 0x20)");
	redefiner("TIMER1_BASE", "(TIMER0_BASE + 0x20)");
	redefiner("TIMER3_BASE", "(TIMER2_BASE + 0x20)");
	redefiner("SECURE_UART_BASE", "UART3_ADDR");
	redefiner("SECURE_UART_BASE_PA", "UART3_ADDR_PA");	

	printf("#endif\n");
	return 0;
}
