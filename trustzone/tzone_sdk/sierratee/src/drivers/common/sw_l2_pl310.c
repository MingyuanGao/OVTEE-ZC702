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

#include <sw_l2_pl310.h>
#include <sw_io.h>
#include <cpu.h>

#define CACHE_LINE_SIZE     32

/**
 * @brief 
 * This structure contains all the properties of Level2
 * Cache controller
 */
struct l2_pl310_variables{
	sw_uint base_addr;
	sw_uint cache_id;
	sw_uint num_of_sets;
	sw_uint num_of_ways;
	sw_uint way_mask;   /* Bitmask of active ways */
	sw_uint cache_size;
	sw_uint initialized;
};

static struct l2_pl310_variables sw_l2_pl310;
static sw_uint l2_params_init = 0;


/**
 * @brief 
 *		Write to a register in PL310 cache controller
 * @param val
 *		Value to be written to the register
 * @param offset
 *		Offset of the register from the Start address of
 *	the PL310 cache controller
 */
static inline void sw_write_to_l2_pl310_reg(sw_uint val, sw_uint offset)
{
	sw_writel(val,(volatile void*)(sw_l2_pl310.base_addr + offset)); 
}


/**
 * @brief 
 *		Read a value from Pl310 cache controller register
 * @param offset
 *		Offset of the register from Start address of
 *	the PL310 cache controller
 */
static inline sw_uint sw_read_from_l2_pl310_reg(sw_uint offset)
{
	return sw_readl((volatile void*)(sw_l2_pl310.base_addr + offset));
}


/**
 * @brief 
 *		Initialize PL310 structure used by the
 *	driver for the operations
 * @param addr
 *		Base address of the PL310 cache controller
 */
void sw_init_l2_pl310_params(void* addr)
{
	if((sw_uint)addr == 0){
		sw_printk("SW: Error- Invalid address\n");
		sw_printk("SW: Cannot Init L2 PL310 parameters\n");
		return;
	}
	sw_l2_pl310.base_addr 	= (sw_uint)addr;
	sw_l2_pl310.cache_id = sw_read_from_l2_pl310_reg(L2_PL310_CACHE_ID_OFF);
	sw_l2_pl310.num_of_sets = 0;
	sw_l2_pl310.num_of_ways = 0;
	sw_l2_pl310.way_mask = 0;   /* Bitmask of active ways */
 	/* L2 cache Size = (Number of ways) * (Size of a way)*/
	sw_l2_pl310.cache_size = 0; 
	sw_l2_pl310.initialized = 0;
	l2_params_init = 1;
}

/**
 * @brief 
 *		This function checks if this L2 cache controller
 * is of a particular version
 * @param rev
 *		The version against which the cache controller is checked
 * @return 
 * 	1 - If the version matches
 *	0 - If the version doesn't match
 */
static inline sw_bool check_pl310_rev(int rev)
{
	return((sw_l2_pl310.cache_id &
			(L2_PL310_CACHE_ID_MASK | L2_PL310_CACHE_REV_MASK))
			== (L2_PL310_CACHE_ID_TYPE_PL310 | rev));
}

/**
 * @brief 
 *		This function checks whether the cache operation which had been 
 *	performed is completed.
 * 		All the operations involving cache_lines are atomic in PL310
 *  So no need to call this function in those cases.
 *		Only in cases where the operation involves WAY, this function is
 * required to be called.
 *
 * @param reg_addr
 *		Address of the register on which the last operation was performed.
 * @param reg_mask
 *		Mask value to check if the operation is complete	
 */
static void wait_for_completion_of_cache_operation(sw_uint offset, sw_uint
		reg_mask)
{
	while(sw_readl((volatile void*)(sw_l2_pl310.base_addr + offset)) &&
																reg_mask) {
		invoke_dsb();
	}
}


/**
 * @brief
 * 	It drains all the buffers. And the drain buffer operation is complete when
 *  LRB, LFB, STB, and EB, are empty.
 */
static void drain_cache_buffers(void)
{
#ifdef CONFIG_ARM_ERRATA_753970
	sw_write_to_l2_pl310_reg(0, L2_PL310_UNMAPPED_REG_OFF);
#else
	sw_write_to_l2_pl310_reg(0, L2_PL310_CACHE_SYNC_OFF);
#endif
}

#if defined(CONFIG_PL310_ERRATA_588369) || defined(CONFIG_PL310_ERRATA_727915)
/**
 * @brief 
 *		Here it is used to force write-through behaviour and
 * disabling Cache linefills in certain circumstances.
 * @param val
 *		The config value
 */
static void write_to_debug_reg(sw_uint val)
{
	if(!sw_l2_pl310.initialized){
		sw_printk("SW:WARNING L2 cache not initialized\n");
		sw_printk("Cannot write to Debug Register\n");
		return;
	}

	sw_write_to_l2_pl310_reg(val, L2_PL310_DEBUG_CTRL_REG_OFF);
}
#else
static inline void write_to_debug_reg(sw_uint val)
{
	return;
}
#endif

/**
 * @brief 
 *	Checks if the L2 cache is enabled
 * @return 
 *	1 - If L2 cache is enabled
 *	0 - If L2 cache is not enabled
 */
static inline int is_l2_enabled(void)
{
	return (sw_read_from_l2_pl310_reg(L2_PL310_CONTROL_OFF) & 1);
}

/**
 * @brief 
 *		In certain cases instead of performing the operation by calling the
 * ALL function, the operations are iterated through the sets and ways
 * @param offset
 * 		The offset of the register to which the values must be written
 * Like clean/Invalidate/Flush etc.,.
 */
static void cache_operation_by_set_way(sw_uint offset)
{
	int set, way;
	for(way = 0;way < sw_l2_pl310.num_of_ways; way++){
		for(set = 0;set < sw_l2_pl310.num_of_sets; set++){
			sw_write_to_l2_pl310_reg((way << 28) | (set << 5), offset);
		}
		drain_cache_buffers();
	}
}

/**
 * @brief 
 *		Clean L2 cacheline by address
 * @param addr
 */
void sw_clean_l2cache_line(sw_uint addr)
{
	sw_write_to_l2_pl310_reg(addr, L2_PL310_CLEAN_CACHELINE_BY_PA_OFF);
}


/**
 * @brief 
 *		Invalidate cacheline by address
 * @param addr
 */
void sw_invalidate_l2cache_line(sw_uint addr)
{
	sw_write_to_l2_pl310_reg(addr, L2_PL310_INV_CACHELINE_BY_PA_OFF);
}


/**
 * @brief 
 *	Clean and Invalidate the cacheline by address
 * @param addr
 */
void sw_flush_l2cache_line(sw_uint addr)
{
#ifdef CONFIG_PL310_ERRATA_588369
	sw_write_to_l2_pl310_reg(addr, L2_PL310_CLEAN_CACHELINE_BY_PA_OFF);
	sw_write_to_l2_pl310_reg(addr, L2_PL310_INV_CACHELINE_BY_PA_OFF);
#else
	sw_write_to_l2_pl310_reg(addr, L2_PL310_CLN_INV_CACHELINE_BY_PA_OFF);
#endif
}

/**
 * @brief 
 *	This function is used to invalidate more than one cache line
 * @param start
 * 	Start address from which invalidation should be performed
 * @param end
 * 	End address till which the invalidation should be performed
 */
void sw_invalidate_l2cache_multi_line(sw_uint start, sw_uint end)
{
	if(start & (CACHE_LINE_SIZE-1)){
		write_to_debug_reg(0x03);
		sw_flush_l2cache_line(start & ~(CACHE_LINE_SIZE-1));		
		write_to_debug_reg(0x00);
		start = (start & ~(CACHE_LINE_SIZE-1)) + CACHE_LINE_SIZE;
	}
	if(end & (CACHE_LINE_SIZE-1)){
		write_to_debug_reg(0x03);
		sw_flush_l2cache_line(end & ~(CACHE_LINE_SIZE-1));		
		write_to_debug_reg(0x00);
		end = (start & ~(CACHE_LINE_SIZE-1));
	}

	while(start < end){
		sw_invalidate_l2cache_line(start);		
		start += CACHE_LINE_SIZE;
	}
	drain_cache_buffers();
}

/**
 * @brief 
 *	This function is used to flush more than one cache line
 * @param start
 * 	Start address from which flush should be performed
 * @param end
 * 	End address till which the flush should be performed
 */
void sw_flush_l2cache_multi_line(sw_uint start, sw_uint end)
{
	if(!sw_l2_pl310.initialized){
		sw_printk("SW:Warning - L2 Cache controller not initialized\n");
		sw_printk("Cannot flush L2 cache\n");
	}

	if((end - start) > sw_l2_pl310.cache_size){
		sw_flush_l2cache_all();
		return;
	}

	start = start & ~(CACHE_LINE_SIZE - 1);

	write_to_debug_reg(0x03);
	while(start < end){
		sw_clean_l2cache_line(start);		
		start += CACHE_LINE_SIZE;
	}
	write_to_debug_reg(0x00);
	drain_cache_buffers();
}

/**
 * @brief 
 *	This function is used to clean more than one cache line
 * @param start
 * 	Start address from which clean should be performed
 * @param end
 * 	End address till which the clean should be performed
 */
void sw_clean_l2cache_multi_line(sw_uint start, sw_uint end)
{
	if((end - start) > sw_l2_pl310.cache_size){
		sw_clean_l2cache_all();
		return;
	}

	start = start & ~(CACHE_LINE_SIZE - 1);

	while(start < end){
		sw_clean_l2cache_line(start);		
		start += CACHE_LINE_SIZE;
	}
	drain_cache_buffers();
}

/**
 * @brief
 *	This function cleans the entire level 2 cache
 */
void sw_clean_l2cache_all(void)
{
	if(!sw_l2_pl310.initialized){
		sw_printk("SW:WARN L2 cache not initialized");
		sw_printk("Cannot Clean the L2 cache");
		return;
	}

#ifdef CONFIG_PL310_ERRATA_727915
	if(check_pl310_rev(REV_PL310_R2P0)){
		cache_operation_by_set_way(L2_PL310_CLEAN_CACHELINE_BY_INDEX_OFF);
		return;
	}
#endif
	write_to_debug_reg(0x03);
	sw_write_to_l2_pl310_reg(sw_l2_pl310.way_mask, 
						L2_PL310_CLEAN_CACHE_BY_WAY_OFF);
	wait_for_completion_of_cache_operation(
			L2_PL310_CLEAN_CACHE_BY_WAY_OFF,
			sw_l2_pl310.way_mask);
	drain_cache_buffers();
	write_to_debug_reg(0x00);
}

/**
 * @brief
 * 	This function invalidates the entire level 2 cache
 */
void sw_invalidate_l2cache_all(void)
{
	if(!sw_l2_pl310.initialized){
		sw_printk("SW:WARN L2 cache not initialized");
		sw_printk("Cannot invalidate the L2 cache");
		return;
	}

	if(is_l2_enabled()){
		sw_printk("SW: WARN L2 cache controller is turned ON");
		sw_printk("Cannot perform Invalidate all Operation\n");
		return;
	}

	sw_write_to_l2_pl310_reg(sw_l2_pl310.way_mask, 
						L2_PL310_INV_CACHE_BY_WAY_OFF);
	wait_for_completion_of_cache_operation(
			L2_PL310_INV_CACHE_BY_WAY_OFF,
			sw_l2_pl310.way_mask);
	drain_cache_buffers();
}

/**
 * @brief 
 *	This function flushes the entire level 2 cache
 */
void sw_flush_l2cache_all(void)
{
	if(!sw_l2_pl310.initialized){
		sw_printk("SW:WARN L2 cache not initialized");
		sw_printk("Cannot Flush the L2 cache");
		return;
	}

#ifdef CONFIG_PL310_ERRATA_727915
	if(check_pl310_rev(REV_PL310_R2P0)){
		cache_operation_by_set_way(L2_PL310_CLN_INV_CACHELINE_BY_INDEX_OFF);
		return;
	}
#endif
	write_to_debug_reg(0x03);
	sw_write_to_l2_pl310_reg(sw_l2_pl310.way_mask, 
						L2_PL310_CLN_INV_CACHELINE_BY_WAY_OFF);
	wait_for_completion_of_cache_operation(
			L2_PL310_CLN_INV_CACHELINE_BY_WAY_OFF,
			sw_l2_pl310.way_mask);
	drain_cache_buffers();
	write_to_debug_reg(0x00);
}


/**
 * @brief 
 * 	Enabling L2 cache 
 */
void sw_l2_pl310_enable(void)
{
	if(!sw_l2_pl310.initialized){
		sw_printk("SW:Warning: L2_PL310 not Initialized; Cannot enable L2");
		return;
	}

	sw_write_to_l2_pl310_reg(0x1, L2_PL310_CONTROL_OFF);
}


/**
 * @brief 
 * Disables the L2 cache
 * The entire L2 cache is flushed before getting disabled
 */
void sw_l2_pl310_disable(void)
{
	if(!sw_l2_pl310.initialized)
		return;

	sw_flush_l2cache_all();
	sw_write_to_l2_pl310_reg(0,L2_PL310_CONTROL_OFF);
	invoke_dsb();
}

/**
 * @brief
 * 	The set way configuration is initialized during the first time 
 *  Initialization of the L2 cache controller
 *
 * @param aux_ctrl_val
 *  The auxillary control register value which is used in 
 * the configuration 
 */
static void init_l2_set_ways(sw_uint aux_ctrl_val)
{
	sw_uint id_val;
	sw_uint size_of_way = 0;

	id_val = sw_l2_pl310.cache_id & L2_PL310_CACHE_ID_MASK;

	if(id_val == L2_PL310_CACHE_ID_TYPE_PL310){
		if(aux_ctrl_val & (1 << 16))
			sw_l2_pl310.num_of_ways = 16;
		else
			sw_l2_pl310.num_of_ways = 8;
	}
	else {
		sw_printk("SW:WARN: L2 Controller type Not Supported \n");
		sw_l2_pl310.num_of_ways = 8;
	}

	sw_l2_pl310.way_mask = (1 << sw_l2_pl310.num_of_ways) - 1;

    size_of_way = (aux_ctrl_val & L2_PL310_WAY_SIZE_MASK) >> 17;
    size_of_way = 1 << (size_of_way + 3);
    sw_l2_pl310.cache_size  = sw_l2_pl310.num_of_ways * size_of_way * 1024;
    sw_l2_pl310.num_of_sets = size_of_way / CACHE_LINE_SIZE;
}

/**
 * @brief
 * 	All the I and D caches are unlocked before
 * initializng the L2 cache
 */
static void sw_unlock_all_cache(void)
{
	sw_uint id_val, iter;
	sw_uint num_of_locks;

	id_val = sw_l2_pl310.cache_id & L2_PL310_CACHE_ID_MASK;
	if(id_val == L2_PL310_CACHE_ID_TYPE_PL310){
		num_of_locks = 8;
	}
	else{
		/* Unknown L2 type */
		num_of_locks = 1;
	}

	for(iter = 0; iter < num_of_locks; iter++){
		sw_write_to_l2_pl310_reg(0, L2_PL310_DATA_LOCK0_BY_WAYD_OFF + iter * 8);
		sw_write_to_l2_pl310_reg(0, L2_PL310_INS_LOCK0_BY_WAYD_OFF + iter * 8);
	}
}

/**
 * @brief 
 *
 * @param aux_val
 * @param aux_mask
 * @param sw_l2_pl310_config
 */
void sw_l2_pl310_init(sw_uint aux_val, sw_uint aux_mask,
	        struct l2_cache_config_vars* sw_l2_pl310_config)
{
    sw_uint tmp_aux_val;

	if(l2_params_init == 0){
		sw_printk("SW:Error - L2 params not Initialized");
		sw_printk("SW:Error- Cannot Initialize L2 PL310 \n");
		return;
	}

    tmp_aux_val = sw_read_from_l2_pl310_reg(L2_PL310_AUX_CNTRL_OFF);

    tmp_aux_val &= aux_mask;
    tmp_aux_val |= aux_val;

	init_l2_set_ways(tmp_aux_val);

    /*
     * If L2 controller is already enabled we cannot do the initialization.
	 * So make sure L2 controller is in disabled state.
     */
    if (!(is_l2_enabled())) {
        /* Unlock cache before configuring */
		sw_unlock_all_cache();
		/* Reconfigure the Auxillary control register */
		sw_write_to_l2_pl310_reg(tmp_aux_val, L2_PL310_AUX_CNTRL_OFF);
        /* Invalidate L2 cache*/
		sw_write_to_l2_pl310_reg(0xFFF,	L2_PL310_INV_CACHE_BY_WAY_OFF);
		sw_printk("poll for invalidate completion\n");
		wait_for_completion_of_cache_operation(	L2_PL310_INV_CACHE_BY_WAY_OFF,
														sw_l2_pl310.way_mask);
		drain_cache_buffers();

		/* Configure Tag RAM latency */ 
		sw_write_to_l2_pl310_reg(sw_l2_pl310_config->tag_ram_val,
												L2_PL310_TAG_RAM_CTRL_OFF);
		/* Configure Data RAM latency */ 
		sw_write_to_l2_pl310_reg(sw_l2_pl310_config->data_ram_val,
												L2_PL310_DATA_RAM_CTRL_OFF);
		/* Configuring Operating mode clock and power modes */
		sw_write_to_l2_pl310_reg(sw_l2_pl310_config->power_val, 
												L2_PL310_POWER_CTRL_REG_OFF);
		/* Configuring Prefetch related faetures */
		sw_write_to_l2_pl310_reg(sw_l2_pl310_config->prefetch_val, 
												L2_PL310_PREFETCH_CTRL_REG_OFF);

		/* Configuring address filters if more than one master is configured */
		if(sw_l2_pl310_config->filter_init){
			sw_write_to_l2_pl310_reg(sw_l2_pl310_config->filter_start,
											L2_PL310_ADDR_FILTERING_START_OFF);
			sw_write_to_l2_pl310_reg(sw_l2_pl310_config->filter_end,
											L2_PL310_ADDR_FILTERING_END_OFF);
		}
    }

	sw_l2_pl310.initialized = 1;

    sw_printk("SW: L2 cache controller enabled\n");
    sw_printk("SW: Cache id    : 0x%x \n", sw_l2_pl310.cache_id);
    sw_printk("SW: num of ways : 0x%x \n", sw_l2_pl310.num_of_ways);
    sw_printk("SW: num of sets : 0x%x \n", sw_l2_pl310.num_of_sets);
    sw_printk("SW: Cache size  : 0x%x \n", sw_l2_pl310.cache_size);
    sw_printk("SW: way mask    : 0x%x \n", sw_l2_pl310.way_mask);
}
