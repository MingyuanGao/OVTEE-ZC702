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

static unsigned int DEVICE_VA_BASE = 0xF2000000;

void mapper(char *str, unsigned int pa, unsigned int size){
	if( size & 0x100000){
		DEVICE_VA_BASE = ((DEVICE_VA_BASE & ~0xFFFFF) + 0x100000);
	}
	printf("#undef %s\n", str);
	printf("#define %s\t\t\t0x%08x\n", str, DEVICE_VA_BASE);
	printf("#define %s_PA\t\t\t0x%08x\n\n", str, pa);
	DEVICE_VA_BASE = DEVICE_VA_BASE + size;
}

void redefiner(char *def, char *val){
	printf("#undef %s\n", def);
	printf("#define %s\t\t\t%s\n\n", def, val);
}

int main(void)
{
	printf("/*\n* Auto-gen file. Dont edit\n*/\n");
	printf("#ifndef _DEVICE_H_\n#define _DEVICE_H_\n\n");
	printf("#include <sw_board_asm.h>\n");
	mapper("SCU_BASE", SCU_BASE, SCU_SIZE);
	mapper("SLCR_BASE", SLCR_BASE, SLCR_SIZE);
	mapper("SECURITY_MOD2", SECURITY_MOD2, SECURITY_MOD2_SIZE);
	mapper("SECURITY_MOD3", SECURITY_MOD3, SECURITY_MOD3_SIZE);
	mapper("SECURE_UART_BASE", SECURE_UART_BASE, SECURE_UART_BASE_SIZE);
	mapper("OCM_HIGH_PHYS", OCM_HIGH_PHYS, OCM_HIGH_PHYS_SIZE);
	mapper("ZYNQ_L2_BASE", ZYNQ_L2_BASE, ZYNQ_L2_SIZE);
	printf("#endif\n");
	return 0;
}
