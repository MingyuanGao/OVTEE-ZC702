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

#ifndef __SW_L2_PL310_H_
#define __SW_L2_PL310_H_

#include <sw_types.h>

/*! Register Offsets */
#define L2_PL310_CACHE_ID_OFF					0x000
#define L2_PL310_CACHE_TYPE_OFF 				0x004

#define L2_PL310_CONTROL_OFF					0x100
#define L2_PL310_AUX_CNTRL_OFF					0x104
#define L2_PL310_TAG_RAM_CTRL_OFF				0x108
#define L2_PL310_DATA_RAM_CTRL_OFF				0x10C

#define L2_PL310_EVT_COUNT_CNTRL_OFF			0x200
#define L2_PL310_EVT_COUNT1_CONFIG_OFF			0x204
#define L2_PL310_EVT_COUNT0_CONFIG_OFF			0x208
#define L2_PL310_EVT_COUNT1_VAL_OFF				0x20C
#define L2_PL310_EVT_COUNT0_VAL_OFF				0x210
#define L2_PL310_INT_MASK_OFF					0x214
#define L2_PL310_MASKED_INT_STATUS_OFF			0x218
#define L2_PL310_RAW_INT_STATUS_OFF				0x21C
#define L2_PL310_INT_CLEAR_OFF					0x220

#define L2_PL310_CACHE_SYNC_OFF			 		0x730
#define L2_PL310_INV_CACHELINE_BY_PA_OFF 		0x770
#define L2_PL310_INV_CACHE_BY_WAY_OFF			0x77C
#define L2_PL310_CLEAN_CACHELINE_BY_PA_OFF		0x7B0
#define L2_PL310_CLEAN_CACHELINE_BY_INDEX_OFF	0x7B8
#define L2_PL310_CLEAN_CACHE_BY_WAY_OFF			0x7BC
#define L2_PL310_CLN_INV_CACHELINE_BY_PA_OFF 	0x7F0	
#define L2_PL310_CLN_INV_CACHELINE_BY_INDEX_OFF 0x7F8	
#define L2_PL310_CLN_INV_CACHELINE_BY_WAY_OFF 	0x7FC	

#define L2_PL310_UNMAPPED_REG_OFF				0x740

#define L2_PL310_DATA_LOCK0_BY_WAYD_OFF 		0x900	
#define L2_PL310_INS_LOCK0_BY_WAYD_OFF		 	0x904  
#define L2_PL310_DATA_LOCK1_BY_WAYD_OFF		 	0x908 
#define L2_PL310_INS_LOCK1_BY_WAYD_OFF       	0x90C 
#define L2_PL310_DATA_LOCK2_BY_WAYD_OFF		 	0x910 
#define L2_PL310_INS_LOCK2_BY_WAYD_OFF		 	0x914 
#define L2_PL310_DATA_LOCK3_BY_WAYD_OFF		 	0x918 
#define L2_PL310_INS_LOCK3_BY_WAYD_OFF		 	0x91C 
#define L2_PL310_DATA_LOCK4_BY_WAYD_OFF		 	0x920 
#define L2_PL310_INS_LOCK4_BY_WAYD_OFF		 	0x924 
#define L2_PL310_DATA_LOCK5_BY_WAYD_OFF		 	0x928
#define L2_PL310_INS_LOCK5_BY_WAYD_OFF		 	0x92C 
#define L2_PL310_DATA_LOCK6_BY_WAYD_OFF		 	0x930 
#define L2_PL310_INS_LOCK6_BY_WAYD_OFF		 	0x934 
#define L2_PL310_DATA_LOCK7_BY_WAYD_OFF		 	0x938 
#define L2_PL310_INS_LOCK7_BY_WAYD_OFF		 	0x93C 
#define L2_PL310_LOCK_BY_LINE_EN_OFF 		 	0x950 
#define L2_PL310_UNLOCK_ALL_LINES_BY_WAY_OFF 	0x954 

#define L2_PL310_ADDR_FILTERING_START_OFF		0xC00
#define L2_PL310_ADDR_FILTERING_END_OFF			0xC04

#define L2_PL310_DEBUG_CTRL_REG_OFF				0xF40
#define L2_PL310_PREFETCH_CTRL_REG_OFF			0xF60
#define L2_PL310_POWER_CTRL_REG_OFF				0xF80

#define REV_PL310_R2P0              4

/*! Register Masks */
#define L2_PL310_CACHE_REV_MASK			(0x3F)
#define L2_PL310_CACHE_ID_MASK			(0xF << 6)
#define L2_PL310_CACHE_ID_TYPE_PL310	( 3	 << 6)
#define L2_PL310_WAY_SIZE_MASK			(0x7 << 17)

struct l2_cache_config_vars{
	sw_uint tag_ram_val;
	sw_uint data_ram_val;
	sw_uint power_val;

	sw_uint filter_init;
	sw_uint filter_start;
	sw_uint filter_end;

	sw_uint prefetch_val;
};

void sw_init_l2_pl310_params(void* addr);

void sw_clean_l2cache_line(sw_uint addr);

void sw_invalidate_l2cache_line(sw_uint addr);

void sw_flush_l2cache_line(sw_uint addr);

void sw_invalidate_l2cache_multi_line(sw_uint start, sw_uint end);

void sw_flush_l2cache_multi_line(sw_uint start, sw_uint end);

void sw_clean_l2cache_multi_line(sw_uint start, sw_uint end);

void sw_clean_l2cache_all(void);

void sw_invalidate_l2cache_all(void);

void sw_flush_l2cache_all(void);

void sw_l2_pl310_enable(void);

void sw_l2_pl310_disable(void);

void sw_l2_pl310_init(sw_uint aux_val, sw_uint aux_mask,
		            struct l2_cache_config_vars* sw_l2_pl310_config);
#endif
