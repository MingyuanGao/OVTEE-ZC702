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

/* Neon application to test the VFP context switching */

#include <neon_app.h>
#include <sw_errno.h>

void add3 (uint8x16_t *data) {
	/* Set each sixteen values of the vector to 3.
	 *
	 * Remark: a 'q' suffix to intrinsics indicates
	 * the instruction run for 128 bits registers.
	 */
	uint8x16_t three = vmovq_n_u8 (3);

	/* Add 3 to the value given in argument. */
	*data = vaddq_u8 (*data, three);
}

void print_uint8 (uint8x16_t data, char* name) {
	int i;
	static uint8_t p[16];

	vst1q_u8 (p, data);

	sw_printf ("%s = ", name);
	for (i = 0; i < 16; i++) {
		sw_printf ("%02d ", p[i]);
	}
	sw_printf ("\n");
}

int test_neon_app() {
	/* Create custom arbitrary data. */
	const uint8_t uint8_data[] = { 1, 2, 3, 4, 5, 6, 7, 8,
		9, 10, 11, 12, 13, 14, 15, 16 };

	/* Create the vector with our data. */
	uint8x16_t data;

	/* Load our custom data into the vector register. */
	data = vld1q_u8 (uint8_data);

	print_uint8 (data, "data");

	/* Call of the add3 function. */
	add3(&data);

	print_uint8 (data, "data (new)");

	return SW_OK;
}
