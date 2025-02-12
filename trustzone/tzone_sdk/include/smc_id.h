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
 * Header file for SMC identifiers 
 */

#ifndef __OTZ_SMC_ID_H__
#define __OTZ_SMC_ID_H__


/* SMC Identifiers for non-secure world functions */
#define CALL_TRUSTZONE_API  0x1
#if defined( CONFIG_S5PV310_BOARD) || defined (CONFIG_MVV4412_BOARD)
/* Based arch/arm/mach-exynos/include/mach/smc.h */
#define SMC_CMD_INIT		(-1)
#define SMC_CMD_INFO		(-2)
/* For Power Management */
#define SMC_CMD_SLEEP		(-3)
#define SMC_CMD_CPU1BOOT	(-4)
#define SMC_CMD_CPU0AFTR	(-5)
/* For CP15 Access */
#define SMC_CMD_C15RESUME	(-11)
/* For Framebuffer */
#define SMC_CMD_INIT_SECURE_WINDOW (-29)

#define SMC_CP15_REG			(-102)
#define SMC_CP15_AUX_CTRL		0x1
#define SMC_CP15_L2_PREFETCH	0x2
#define SMC_CACHE_CTRL			0x3
#endif

/* For L2 Cache Access */
#define SMC_CMD_L2X0FLTR_SETUP (-20)
#define SMC_CMD_L2X0CTRL	(-21)
#define SMC_CMD_L2X0SETUP1	(-22)
#define SMC_CMD_L2X0SETUP2	(-23)
#define SMC_CMD_L2X0INVALL	(-24)
#define SMC_CMD_L2X0DEBUG	(-25)
#define SMC_CMD_L2X0FLUSHALL (-26)
#define SMC_CMD_L2X0CLEANALL (-27)
#define SMC_CMD_L2X0FLUSHRANGE (-28)
#define SMC_CMD_L2X0INVRANGE   (-35)
#define SMC_CMD_L2X0INVLINE        (-36)
#define SMC_CMD_L2X0CLEANRANGE (-37)
#define SMC_CMD_L2X0CLEANLINE  (-38)
#define SMC_CMD_L2X0FLUSHLINE  (-39)

#ifdef CONFIG_ZYNQ7_BOARD
#define SMC_CMD_CPU1BOOT       (-4)
#define SMC_CMD_SECURE_READ		(-30)
#define SMC_CMD_SECURE_WRITE	(-31)
#endif

#endif /* __OTZ_SMC_ID__ */
