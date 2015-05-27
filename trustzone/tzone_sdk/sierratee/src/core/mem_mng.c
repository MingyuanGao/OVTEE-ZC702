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
 * Memory manager implementation
 */

#include <sw_types.h>
#include <sw_board.h>
#include <sw_mem_functions.h>
#include <sw_debug.h>
#include <page_table.h>
#include <cpu_data.h>
#include <sw_mmu_helper.h>
#include <mem_mng.h>
#ifndef CONFIG_SVISOR_SUPPORT
#include <task.h>
#endif

/**
 * @brief 
 *  Initializes the address space in secure world by allocating 
 *  and mapping the required pages 
 * @return 
 *      Returns the address of the secure world page table
 */
sw_uint* sw_meminfo_init(mem_info* sw_mem_info)
{
	sw_uint sw_code_len = 0;
	sw_uint map_pa , map_va; 
	sw_uint alc_phy_add = 0;
	sw_uint alc_vir_add = 0;
	sw_uint tot_len_to_alloc; 

	sw_memset( sw_mem_info, 0, sizeof(mem_info));

	sw_mem_info->phy_addr_start      =    get_ram_start_addr();
	sw_mem_info->phy_memregion_size  =    get_ram_end_addr() -
		get_ram_start_addr() + 1;
	sw_mem_info->num_phy_pag_free = 
		((sw_mem_info->phy_memregion_size) >> PAGE_SHIFT);


	sw_mem_info->vir_addr_start = get_sw_code_start() ;
	//sw_mem_info->vir_memregion_size = VIR_ASPACE_SIZE ;
	sw_mem_info->vir_memregion_size =
		sw_mem_info->phy_memregion_size ;
	sw_mem_info->num_vir_pag_free =
		((sw_mem_info->vir_memregion_size) >> PAGE_SHIFT);

	if ( sw_mem_info->phy_addr_start & PAGE_OFFSET_MASK){
		sw_mem_info->phy_addr_start +=
			(PAGE_SIZE - (sw_mem_info->phy_addr_start & PAGE_OFFSET_MASK));
		sw_mem_info->phy_memregion_size -=
			(PAGE_SIZE - (sw_mem_info->phy_addr_start & PAGE_OFFSET_MASK));
	}

	if( sw_mem_info->phy_memregion_size & PAGE_OFFSET_MASK){
		sw_mem_info->phy_memregion_size -= 
			(sw_mem_info->phy_memregion_size & PAGE_OFFSET_MASK) ;
	}

	sw_mem_info->phy_mem_bmap_len = (sw_mem_info->phy_memregion_size >> 
			(PAGE_SHIFT + 3)) + 1;

	/* memory word alignment */
	if(sw_mem_info->phy_mem_bmap_len & 0x3) {
		sw_mem_info->phy_mem_bmap_len  += 
			4 - (sw_mem_info->phy_mem_bmap_len & 0x3);
	}

	sw_mem_info->vir_mem_bmap_len = (sw_mem_info->vir_memregion_size >> 
			(PAGE_SHIFT + 3)) + 1;

	sw_mem_info->phy_mem_bmap = get_mem_info_start();

	sw_mem_info->vir_mem_bmap = (sw_uint*)((sw_uint)sw_mem_info->phy_mem_bmap
			+ sw_mem_info->phy_mem_bmap_len);



	sw_memset(sw_mem_info->phy_mem_bmap , 0 ,
			(sw_mem_info->phy_mem_bmap_len + sw_mem_info->vir_mem_bmap_len));
	sw_code_len = (sw_uint)get_sw_code_end() - (sw_uint)get_sw_code_start();

	sw_mem_info->page_scope = KERN_PAGE;

	map_pa = sw_mem_info->phy_addr_start; 
	map_va = (sw_uint)sw_mem_info->vir_addr_start; 
	alc_phy_add = 0;
	alc_vir_add = 0;

	tot_len_to_alloc = ( sw_code_len + SECTION_SIZE);

	while( map_pa < (sw_mem_info->phy_addr_start + tot_len_to_alloc) )
	{
		alc_phy_add = ((map_pa - sw_mem_info->phy_addr_start) >> PAGE_SHIFT);

		sw_mem_info->phy_mem_bmap[alc_phy_add >> 5]
			|= 0x1 << (31 - (alc_phy_add & 0x1F));

		sw_mem_info->num_phy_pag_free--;
		map_pa += PAGE_SIZE;
	}

	while( map_va < ((sw_uint)sw_mem_info->vir_addr_start + tot_len_to_alloc) )
	{
		alc_vir_add = ((map_va - (sw_uint)sw_mem_info->vir_addr_start) >> PAGE_SHIFT);

		sw_mem_info->vir_mem_bmap[alc_vir_add >> 5]
			|= 0x1 << (31 - (alc_vir_add & 0x1F));


		sw_mem_info->num_vir_pag_free--;
		map_va += PAGE_SIZE;
	}
#ifndef CONFIG_SVISOR_SUPPORT
	return map_secure_page_table();
#else   
	/* Fix Me: Svisor uses static mapping */
#endif	
}

/**
 * @brief 
 *      Allocates physical pages for the required size
 *
 * @param size
 *      Size of the memory to be allocated
 *
 * @return 
 *  Returns the address of the memory allocated
 */
sw_phy_addr sw_phy_page_alloc(sw_uint size, mem_info *info)
{
	int req_cnt,start = 0, iter=0, available;
	int avl_cnt=0, cnt; 
	sw_uint phy_addr;

	/*Align to next page size */
	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE - 1))) + PAGE_SIZE;
	req_cnt = size >> PAGE_SHIFT;

	if(req_cnt > info->num_phy_pag_free){
		sw_printk("SW: Page allocation failed \n");
		return SW_OK;
	}

	available = 0;

	for(start = 0; 
			start < ((info->phy_mem_bmap_len << 3) - req_cnt);
			start++)
	{
		avl_cnt = 0;
		for(iter = start; iter < ( start + req_cnt) ; iter++){
			if(info->phy_mem_bmap[iter>>5] & (1 << (31-(iter & 0x1F))))
				break;
			avl_cnt++;
		}
		if(avl_cnt == req_cnt){
			available = 1;
			break;
		}
	}

	if(!available)
		return SW_OK;

	for(cnt = 0;cnt < req_cnt; cnt++)
	{
		info->phy_mem_bmap[(start+cnt) >> 5] 
			|= 0x1 << (31 - ((start+cnt) & 0x1F));
		info->num_phy_pag_free--;
	}

	phy_addr = ((start <<  PAGE_SHIFT) + info->phy_addr_start);

	return phy_addr;
}

/**
 * @brief 
 *      Allocates physical pages for the required size
 *
 * @param size
 *      Size of the memory to be allocated
 *      bitmap from which the memory will be allocated
 *      page scope - either global map or process specific map
 *
 * @return 
 *  Returns the address of the memory allocated
 */
sw_uint* sw_vir_page_alloc(sw_uint size, mem_info *info)
{
	int req_cnt,start = 0, iter=0, available;
	int avl_cnt=0, cnt; 
	sw_uint vir_addr;
	sw_uint phy_addr;
	/*Align to next page size */
	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE -1))) + PAGE_SIZE;
	req_cnt = size >> PAGE_SHIFT;

	if(req_cnt > info->num_vir_pag_free){
		sw_printk("SW: Page allocation failed \n");
		sw_seterrno(SW_ENOMEM);
		return NULL;
	}

	available = 0;

	for(start = 0; start < ((info->vir_mem_bmap_len << 3) - req_cnt);
			start++)
	{
		avl_cnt = 0;
		for(iter = start; iter < ( start + req_cnt) ; iter++){
			if(info->vir_mem_bmap[iter>>5] & (1 << (31-(iter & 0x1F))))
				break;
			avl_cnt++;
		}
		if(avl_cnt == req_cnt){
			available = 1;
			break;
		}
	}

	if(!available) {
		sw_printk("No available Virtual Memory\n");
		sw_seterrno(SW_ENOMEM);
		return NULL;
	}

	phy_addr = (sw_uint)sw_phy_page_alloc(size, info);
	if(phy_addr == 0){
		sw_printk("No available Physical Memory\n");
		sw_seterrno(SW_ENOMEM);
		return NULL;
	}

	for(cnt = 0;cnt < req_cnt; cnt++)
	{
		info->vir_mem_bmap[(start+cnt) >> 5] 
			|= 0x1 << (31 - ((start+cnt) & 0x1F));
		info->num_vir_pag_free--;
	}

	vir_addr = ((start << PAGE_SHIFT) + (sw_uint)info->vir_addr_start);

	map_kernel_data_memory(vir_addr, phy_addr, size);

	return (sw_uint*)vir_addr;
}

/**
 * @brief 
 *      Reserves the required pages in the bitmap
 *
 * @param phy_addr
 *      start address of the area to be reserved
 * @param size
 *      size of the area to be reserved
 *
 * @return 
 *  Returns zero on success
 */
int sw_phy_addr_reserve(sw_phy_addr phy_addr, int size, mem_info *info)
{
	sw_uint res_pa;
	sw_uint temp_sz,pg_cnt;
	int avl_cnt;

	if(phy_addr < info->phy_addr_start ||
			phy_addr > ( info->phy_addr_start + info->phy_memregion_size)){
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE -1))) + PAGE_SIZE;

	pg_cnt = (size >> PAGE_SHIFT);
	res_pa = (phy_addr - info->phy_addr_start) >> PAGE_SHIFT;
	temp_sz = size;

	avl_cnt = 0;

	sw_uint chk_pa = res_pa;
	for( ;chk_pa < (res_pa + pg_cnt); chk_pa++)
	{
		if(info->phy_mem_bmap[chk_pa >> 5] & 
				( 0x1 << (31 - (chk_pa & 0x1F)))){
			break;
		}
		avl_cnt++;
	}

	if(avl_cnt != pg_cnt) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	while(temp_sz > 0){

		info->phy_mem_bmap[res_pa >> 5] |=
			0x1 << (31 - (res_pa & 0x1F));
		temp_sz -= PAGE_SIZE;
		info->num_phy_pag_free--;
		res_pa++;
	}

	return SW_OK;

}

/**
 * @brief 
 *      Reserves the required pages in the bitmap
 *
 * @param phy_addr
 *      start address of the area to be reserved
 * @param size
 *      size of the area to be reserved
 *
 * @return 
 *  Returns zero on success
 */
int sw_vir_addr_reserve(sw_vir_addr vir_addr, int size, mem_info *info)
{
	sw_uint res_va;
	sw_uint temp_sz,pg_cnt;
	int avl_cnt;

	if(vir_addr < (sw_uint)info->vir_addr_start ||
			vir_addr > ((sw_uint) info->vir_addr_start 
				+ info->vir_memregion_size)){
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE -1))) + PAGE_SIZE;

	pg_cnt = (size >> PAGE_SHIFT);
	res_va = ((sw_uint)vir_addr - (sw_uint)info->vir_addr_start) >> PAGE_SHIFT;
	temp_sz = size;

	avl_cnt = 0;

	sw_uint chk_va = res_va;
	for( ;chk_va < (res_va + pg_cnt); chk_va++)
	{
		if(info->vir_mem_bmap[chk_va >> 5] & 
				( 0x1 << (31 - (chk_va & 0x1F)))){
			break;
		}
		avl_cnt++;
	}

	if(avl_cnt != pg_cnt) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	avl_cnt = 0;

	while(temp_sz > 0){

		info->vir_mem_bmap[res_va >> 5] |=
			0x1 << (31 - (res_va & 0x1F));
		temp_sz -= PAGE_SIZE;
		info->num_vir_pag_free--;
		res_va++;
	}

	return SW_OK;

}

/**
 * @brief 
 *      Free the reserved pages in the bitmap
 *
 * @param vir_addr
 *      start address of the area to be reserved
 * @param size
 *      size of the area to be reserved
 *
 * @return 
 *  Returns zero on success
 */
int sw_vir_free_reserve(sw_vir_addr vir_addr, int size, mem_info *info)
{
	sw_uint res_va;

	if(vir_addr < (sw_uint)info->vir_addr_start ||
			(vir_addr > ((sw_uint) info->vir_addr_start )
				+ info->vir_memregion_size)){
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE -1))) + PAGE_SIZE;

	res_va = ((sw_uint)vir_addr - (sw_uint)info->vir_addr_start) >> PAGE_SHIFT;

	while(size > 0){

		info->vir_mem_bmap[res_va >> 5] &=
			~(0x1 << (31 - (res_va & 0x1F)));
		size -= PAGE_SIZE;
		info->num_vir_pag_free++;
		res_va++;
	}

	return SW_OK;

}

/**
 * @brief 
 *      Frees the pages
 * @param phy_addr
 *      start address of the pages to be freed
 * @param size
 *      Size of the memory to be freed
 *
 * @return 
 *      Returns zero on success 
 */
int sw_phy_addr_free(sw_phy_addr phy_addr , int size, mem_info *info)
{
	sw_uint fr_phy;
	int tmp_cnt;

	if(phy_addr < info->phy_addr_start ||
			phy_addr > (info->phy_addr_start + info->phy_memregion_size))
	{
		sw_printk("SW: Memory out of range\n");
		sw_seterrno(SW_ENOMEM);
		return SW_ERROR;
	}
	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE -1))) + PAGE_SIZE;


	fr_phy = (phy_addr - info->phy_addr_start) >> PAGE_SHIFT;

	tmp_cnt = fr_phy + (size >> PAGE_SHIFT);

	for( ; fr_phy < tmp_cnt ; fr_phy++)
	{
		info->phy_mem_bmap[fr_phy >> 5] &=
			~(0x1 << (31 - (fr_phy & 0x1F)));
		info->num_phy_pag_free++;
	}
	return SW_OK;
}

/**
 * @brief 
 *      Reserves the required pages in the bitmap
 *
 * @param vir_addr:
 *      start address of the area to be reserved
 * @param size:
 *      size of the area to be reserved
 *
 * @return 
 *  Returns zero on success
 */
int sw_vir_addr_free(sw_uint vir_addr , int size, mem_info *info)
{
	sw_uint fr_vir;
	int tmp_cnt;
	sw_phy_addr phy_addr;
	sw_vir_addr sec_vir_addr = vir_addr;

	if(vir_addr < (sw_uint)info->vir_addr_start ||
			vir_addr > (sw_uint)(info->vir_addr_start + info->vir_memregion_size))
	{
		sw_printk("SW: Memory out of range\n");
		sw_seterrno(SW_ENOMEM);
		return SW_ERROR;
	}
	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE -1))) + PAGE_SIZE;


	fr_vir = (vir_addr - (sw_uint)info->vir_addr_start) >> PAGE_SHIFT;

	tmp_cnt = fr_vir + (size >> PAGE_SHIFT);

	for( ; fr_vir < tmp_cnt ; fr_vir++)
	{
		info->vir_mem_bmap[fr_vir >> 5] &=
			~(0x1 << (31 - (fr_vir & 0x1F)));
		info->num_vir_pag_free++;
	}

#ifndef CONFIG_SVISOR_SUPPORT
	phy_addr =  secure_vir_to_phy(vir_addr);
#else
	phy_addr =  svisor_vir_to_phy(vir_addr);
#endif
	sw_phy_addr_free(phy_addr , size, info);

	unmap_kernel_memory(sec_vir_addr, size);

	return SW_OK;
}

