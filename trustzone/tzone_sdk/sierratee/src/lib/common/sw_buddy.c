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
 * Buddy allocator implementation.
 */

#include <sw_types.h>
#include <sw_debug.h>
#include <sw_mem_functions.h>
#include <sw_buddy.h>
#include <debug_config.h>
#include <sw_math.h>

/**
 * @brief Add sorted binary tree node in the allocate tree
 *
 * @param parent: Parent node of the tree
 * @param new_node: Node to be added
 */
static void add_sorted_btree_node(struct sw_area *parent,
		struct sw_area *new_node)
{
	while(parent != NULL) {
		if(new_node->mem_addr < parent->mem_addr) {
			if(parent->lchild)
				parent = parent->lchild;
			else {
				parent->lchild = new_node;
				new_node->parent = parent;
				return;
			}
		}   
		else {
			if(parent->rchild) {
				if(new_node->mem_addr < parent->rchild->mem_addr) {
					new_node->parent = parent;
					new_node->rchild = parent->rchild;
					parent->rchild->parent = new_node;
					parent->rchild = new_node;
					return;
				}
				else {
					parent = parent->rchild;
				}
			}
			else {
				parent->rchild = new_node;
				new_node->parent = parent;
				return;
			}
		}
	}
}

/**
 * @brief Add node into the allocation tree
 *
 * @param parent: Parent node of the tree
 * @param new_node: Node to be added
 */
static void add_btree_node(struct sw_area *parent, struct sw_area *new_node)
{
	while(parent != NULL) {
		if(new_node->mem_addr < parent->mem_addr) {
			if(parent->lchild)
				parent = parent->lchild;
			else {
				parent->lchild = new_node;
				new_node->parent = parent;
				return;
			}
		}
		else if(new_node->mem_addr > parent->mem_addr){
			if(parent->rchild)
				parent = parent->rchild;
			else {
				parent->rchild = new_node;
				new_node->parent = parent;
				return;
			}
		}
		else {
			sw_printf("!!!!THE NODE ALREADY EXIST!!!!!!\n");
			return ;
		}
	}
}

/**
 * @brief Get maximum node of the subtree
 *
 * @param node: Current node
 *
 * @return : Maximum node of the sub-tree
 */
static struct sw_area *get_max_node(struct sw_area *node)
{
	while(node != NULL) {
		if(node->rchild == NULL)
			return node;
		else {
			node = node->rchild;
		}
	}

	/*unreachable code */
	return NULL;
}

/**
 * @brief Get the minimum node of the sub-tree
 *
 * @param node: Current node
 *
 * @return : Minimum node of the sub-tree
 */
static struct sw_area *get_min_node(struct sw_area *node)
{
	while(node != NULL) {
		if(node->lchild == NULL)
			return node;
		else {
			node = node->lchild;
		}
	}

	/* unreachable code */
	return NULL; 
} 

/**
 * @brief Remove a node from the allocated tree
 *
 * @param parent: Parent node of the tree
 * @param del_node: Node to be deleted
 */
static void remove_btree_node(struct sw_area *parent, struct sw_area *del_node)
{
	struct sw_area *min_node;
	struct sw_area *max_node;

	while(parent != NULL) {
		if (del_node->mem_addr < parent->mem_addr) {
			if(parent->lchild)
				parent = parent->lchild;
			else {
				return;
			}
		}   
		else if (del_node->mem_addr > parent->mem_addr) {
			if(parent->rchild)
				parent = parent->rchild;
			else {
				return;
			}
		}
		else { /* Found the node. Delete it */
			if(del_node->lchild != NULL && del_node->rchild != NULL) {
				min_node = get_min_node(del_node->rchild);
				if(min_node == del_node->rchild) {
					if(del_node->parent->rchild == del_node) {
						del_node->parent->rchild = min_node;
					}
					else if(del_node->parent->lchild == del_node) {
						del_node->parent->lchild = min_node;
					}
					min_node->lchild = del_node->lchild;
					min_node->lchild->parent=min_node;
					min_node->parent = del_node->parent;
				}
				else {
					if(del_node->parent->rchild == del_node) {
						del_node->parent->rchild = min_node;
					}
					else if(del_node->parent->lchild == del_node) {
						del_node->parent->lchild = min_node;
					}
					if(min_node->rchild!=NULL) {
						min_node->parent->lchild=min_node->rchild;
						min_node->rchild->parent=min_node->parent;
					}
					else {
						min_node->parent->lchild = NULL;
					}
					min_node->lchild = del_node->lchild;
					min_node->lchild->parent=min_node;
					min_node->rchild = del_node->rchild;
					min_node->rchild->parent=min_node;
					min_node->parent = del_node->parent;
				}
				del_node->parent = NULL;
				del_node->lchild = NULL;
				del_node->rchild = NULL;
				return;
			}
			else if(del_node->lchild == NULL && del_node->rchild == NULL) {
				if(del_node->parent->lchild == del_node) {
					del_node->parent->lchild = NULL;
				}
				else if(del_node->parent->rchild == del_node) {
					del_node->parent->rchild = NULL;
				}
				del_node->parent = NULL;
				del_node->lchild = NULL;
				del_node->rchild = NULL;
				return;
			}
			else {
				if(del_node->lchild) {
					max_node=get_max_node(del_node->lchild);
					if(del_node->lchild==max_node) {
						if(del_node->parent->lchild == del_node) {
							del_node->parent->lchild = max_node;
						}
						else if(del_node->parent->rchild == del_node) {
							del_node->parent->rchild = max_node;
						}
					}
					else {
						if(del_node->parent->lchild == del_node) {
							del_node->parent->lchild = max_node;
						}
						else if (del_node->parent->rchild == del_node) {
							del_node->parent->rchild = max_node;
						}
						if(max_node->lchild!=NULL) {
							max_node->parent->rchild=max_node->lchild;
							max_node->lchild->parent=max_node->parent;
						}
						else {
							max_node->parent->rchild = NULL;
						}
						max_node->lchild = del_node->lchild;
						max_node->lchild->parent=max_node;
					}
					max_node->parent = del_node->parent;
				}
				else {
					min_node=get_min_node(del_node->rchild);
					if(del_node->rchild==min_node) {
						if(del_node->parent->lchild == del_node) {
							del_node->parent->lchild = min_node;
						}
						else if(del_node->parent->rchild == del_node) {
							del_node->parent->rchild = min_node;
						}
					}
					else {
						if(del_node->parent->lchild == del_node) {
							del_node->parent->lchild = min_node;
						}
						else if(del_node->parent->rchild == del_node) {
							del_node->parent->rchild = min_node;
						}
						if(min_node->rchild!=NULL) {
							min_node->parent->lchild=min_node->rchild;
							min_node->rchild->parent=min_node->parent;
						}
						else {
							min_node->parent->lchild = NULL;
						}
						min_node->rchild = del_node->rchild;
						min_node->rchild->parent=min_node;
					}
					min_node->parent = del_node->parent;
				}
				del_node->parent = NULL;
				del_node->lchild = NULL;
				del_node->rchild = NULL;
				return;
			}
		}
	}
}

/**
 * @brief Search the allocated tree for the matched data
 *
 * @param parent: Parent node of the tree
 * @param data: Node data to be searched
 *
 * @return : Matched node or NULL
 */
static struct sw_area* search_btree_node(struct sw_area *parent, 
		void *data)
{
	struct sw_area* node;
	node = parent;
	while(node != NULL) {
		if(node->mem_addr == data)
			return node;
		else if(data < node->mem_addr)
			node = node->lchild;
		else
			node = node->rchild;
	}
	return NULL;
}

#ifdef DEBUG_BUDDY
/**
 * @brief Print the allocated tree
 *
 * @param parent: Start node of the tree
 */
static void print_btree(struct sw_area *parent)
{
	if(parent == NULL)
		return;
	if(parent==parent->lchild || parent==parent->rchild) {
		sw_printf("Self-Loop is Deducted\n");
		return ;
	}
	print_btree(parent->lchild);
	sw_printf("node memory addr 0x%x and corresponding bin 0x%x \
			and count 0x%x\n", 
			parent->mem_addr,
			parent->bin,
			parent->cnt);
	print_btree(parent->rchild);
}
#endif
/**
 * @brief Get the free node from buddy pool
 *
 * @param buddy_pool: Pointer to the buddy pool
 *
 * @return Valid free node or NULL
 */
static struct sw_area * get_free_node(struct sw_buddy_pool *buddy_pool)
{
	int index, max_check_cnt, start_index;
	struct sw_area *free;

	start_index = buddy_pool->next_free_index;
	index = start_index;
	free = &buddy_pool->area[index];
	max_check_cnt = buddy_pool->total_area_cnt;

	for (;index < max_check_cnt; index++) {
		if(index == max_check_cnt -1 )
			start_index = start_index;
		if (free->mem_addr) {
			free++;
		} else {
			if(index + 1 != buddy_pool->total_area_cnt)
				buddy_pool->next_free_index = index + 1;
			else
				buddy_pool->next_free_index = 0;

			sw_memset(free, 0, sizeof(struct sw_area));
			return free;
		}
	}

	index = 0;
	free = &buddy_pool->area[index];
	max_check_cnt = start_index;

	for (;index < max_check_cnt; index++) {
		if (free->mem_addr) {
			free++;
		} else {
			buddy_pool->next_free_index = index + 1;

			sw_memset(free, 0, sizeof(struct sw_area));
			return free;
		}
	}

	return NULL;    
}

/**
 * @brief Get the contiguous block from the given pool based on the requested
 * block for the corresponding bin
 *
 * @param buddy_pool: Pointer to the buddy pool
 * @param num_blocks: Number of blocks requested
 * @param bin: Bin number
 *
 * @return Buddy area or Null
 */
static struct sw_area * get_contiguous_block(struct sw_buddy_pool *buddy_pool, 
		sw_uint num_blocks, sw_uint bin)
{
	struct sw_area *prev_node = NULL, *cur_node = NULL, *first_node = NULL;
	sw_uint count = 0;
	if(bin > buddy_pool->max_bin)
		return NULL;

	if(buddy_pool->free_area[bin].rchild == NULL)
		return NULL;

	if(buddy_pool->free_area[bin].cnt < num_blocks) {
		return NULL;
	}

	count = 1;

	cur_node = buddy_pool->free_area[bin].rchild;
	while(cur_node != NULL) {
		if (first_node == NULL) {
			first_node = cur_node;
			prev_node = cur_node;
		}
		else {
			if (prev_node->mem_addr + buddy_pool->max_alloc_size ==
					cur_node->mem_addr) {
				prev_node = cur_node;
				if (++count == num_blocks) {
					break;
				}
				continue;
			}
			first_node = cur_node;
			prev_node = cur_node;
			count = 1;
		} 
		cur_node = cur_node->rchild;
	}

	if(first_node == NULL || count != num_blocks)
		return NULL;

	cur_node = first_node;
	while(count) {
		prev_node = cur_node->rchild;
		remove_btree_node(cur_node->parent, cur_node);
		cur_node = prev_node;
		count--;
		buddy_pool->free_area[bin].cnt--;
	}
	return first_node;
}

/**
 * @brief Check for empty bin
 *
 * @param buddy_pool: Pointer to the buddy pool
 * @param bin: Bin to be checked
 *
 * @return True or False
 */
static sw_uint is_empty_bin(struct sw_buddy_pool *buddy_pool, sw_uint bin)
{
	return (buddy_pool->free_area[bin].rchild == NULL) ? 1: 0;
}

/**
 * @brief Free buddy area for the given bin
 *
 * @param buddy_pool: Pointer to the buddy pool
 * @param bin: Requested bin number 
 *
 * @return Free area pointer or NULL
 */
static struct sw_area * get_free_buddy(struct sw_buddy_pool *buddy_pool,sw_uint bin)
{
	sw_uint idx, idx2;
	struct sw_area *node, *freenode;

	for(idx = bin; idx <=  buddy_pool->max_bin; idx++) {
		if(is_empty_bin(buddy_pool, idx))
			continue;

		if(idx == bin) {
			node = buddy_pool->free_area[idx].rchild;
			remove_btree_node(node->parent, node);
			buddy_pool->free_area[idx].cnt--;
			return node;
		}

		for(idx2 = idx; idx2 > bin; idx2--) {
			if(idx2 == idx) {
				node = buddy_pool->free_area[idx2].rchild;
				remove_btree_node(node->parent, node);
				buddy_pool->free_area[idx2].cnt--;
			}

			freenode = get_free_node(buddy_pool);
			if (!freenode) {
				sw_printf("no free node\n");
				return NULL;
			}
			if (freenode) {
				freenode->bin = idx2-1;
				freenode->block_size =  buddy_pool->min_alloc_size << (idx2-1);
				freenode->mem_addr = node->mem_addr + freenode->block_size;

				add_sorted_btree_node(&buddy_pool->free_area[idx2-1],
						freenode);
				buddy_pool->free_area[idx2-1].cnt++;    

				if(bin == idx2 -1) {
					node->block_size = buddy_pool->min_alloc_size << (idx2 -1);
					node->bin = idx2-1;

					return node;
				}
			}
		}    
		return NULL;
	}
	return NULL;
}

/**
 * @brief Return the free area to the bin and merge the bins if it needed
 *
 * @param buddy_pool: Pointer to the buddy pool
 * @param freenode: Free area to be returned to the pool
 * @param bin: Bin number
 */
static void return_to_free_area(struct sw_buddy_pool *buddy_pool, 
		struct sw_area *freenode,
		sw_uint bin)
{
	sw_uint idx;
	struct sw_area *merge_node, *temp_node;
	sw_uint buddy;

	temp_node = freenode;
	freenode->bin = bin;
	freenode->block_size = (buddy_pool->min_alloc_size << bin);
	buddy_pool->free_area[bin].cnt++;

	if(bin == buddy_pool->max_bin) {
		add_sorted_btree_node(&buddy_pool->free_area[bin], freenode);
		return;
	}
	else {
		add_sorted_btree_node(&buddy_pool->free_area[bin], freenode);
	}

	for (idx = bin; idx <= buddy_pool->max_bin; idx++) {
		freenode = buddy_pool->free_area[idx].rchild;
		if(!freenode)
			break;

		buddy =  ((sw_uint)freenode->mem_addr + 
				(buddy_pool->min_alloc_size << idx));
		merge_node = search_btree_node(&buddy_pool->free_area[idx], 
				(void*)buddy);

		if(merge_node) {
			remove_btree_node(&buddy_pool->free_area[idx], freenode);
			remove_btree_node(&buddy_pool->free_area[idx], merge_node);
			buddy_pool->free_area[idx].cnt -= 2;

			if(buddy < (sw_uint)freenode->mem_addr) {
				freenode->mem_addr = 0;
				freenode = merge_node;
			}
			else {
				merge_node->mem_addr = 0;
			}

			freenode->bin = idx + 1;
			freenode->block_size = (buddy_pool->min_alloc_size << (idx + 1));
			buddy_pool->free_area[idx+1].cnt++;


			if(idx + 1 == buddy_pool->max_bin) {
				add_sorted_btree_node(&buddy_pool->free_area[idx + 1], freenode);
				return;
			}
			else {
				add_sorted_btree_node(&buddy_pool->free_area[idx +1], freenode);
			}
		}
		else {
			if(merge_node ==  temp_node || temp_node == freenode)   
				break;
		}
	}

	return;
}

/**
 * @brief Get managed area size for the given heap and minimum allocation size
 *
 * @param heap_size: Heap Size 
 * @param min_alloc_size: Minium allocation size of the buddy
 *
 * @return 
 */
int get_manage_area(size_t heap_size,sw_uint min_alloc_size) 
{
	sw_uint manage_area_len, temp_len;
	sw_uint min_alloc_shift;
	min_alloc_shift=log_of_2(min_alloc_size);

	temp_len  = heap_size >> min_alloc_shift;
	manage_area_len = temp_len * sizeof(struct sw_area);
	if(manage_area_len & (PAGE_SIZE - 1))
		manage_area_len = (manage_area_len & (~(PAGE_SIZE -1) )) + PAGE_SIZE;
	return manage_area_len;
}

/**
 * @brief Init buddy allocator
 * 
 * Initialize the buddy allocator based on the minimum and maximum allocation
 * size. 
 *
 * @param buddy_pool: Pointer to buddy pool
 * @param pool_start: Heap start address
 * @param pool_size: Buddy size include the heap size and management area
 * @param min_alloc_size: Minimum allocation size
 * @param max_alloc_size: Maximum allocation size
 *
 * @return 
 * SW_OK - Buddy initialization done successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
int sw_buddy_init(struct sw_buddy_pool *buddy_pool, 
		sw_uint pool_start, sw_big_ulong pool_size, 
		sw_uint min_alloc_size, sw_uint max_alloc_size)
{
	sw_big_ulong mem_size;
	sw_uint max_bin;
	sw_uint min_alloc_shift, max_alloc_shift;
	sw_uint manage_area_len;
	sw_uint total_nodes = 0, tnode_cnt, idx;
	void *mem_start;
	struct sw_area * freenode;

	if(!is_pow_of_2(min_alloc_size)) {
		sw_printf("Minimum alloc size should be in power of 2\n");	
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	if(!is_pow_of_2(max_alloc_size)) {
		sw_printf("Heap size is should be in power of 2\n");	
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	min_alloc_shift = log_of_2(min_alloc_size);
	max_alloc_shift = log_of_2(max_alloc_size);

	buddy_pool->total_area_cnt = max_alloc_size >> min_alloc_shift;

	manage_area_len = get_manage_area((size_t)max_alloc_size, 
									(sw_uint)min_alloc_size);

	total_nodes = max_alloc_size >> max_alloc_shift;
	if(total_nodes > 0) {
		max_bin = max_alloc_shift - min_alloc_shift + 1;
	}
	else {
		total_nodes = 1;
		max_bin = log_of_2(buddy_pool->total_area_cnt);
	}

	max_alloc_shift = log_of_2(max_alloc_size);

	if((max_alloc_shift - min_alloc_shift + 1) > MAX_BINS) {
		sw_printf("log_of_2(max alloc size) - log_of_2(min alloc size) + 1 should be \
				less than maximum bins 0x%x\n", MAX_BINS);
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}        

	buddy_pool->area = (void*)pool_start;
	buddy_pool->next_free_index = 0;
	buddy_pool->alloc_cnt = 0;

	sw_memset(&buddy_pool->alloc_tree, 0, sizeof(struct sw_area));

	mem_start = (void*)pool_start + manage_area_len ;
	mem_size = pool_size - manage_area_len;

	if(mem_size <= 0) {
		sw_printf("Buddy allocator manageable area is greater than the \
				supplied pool size. Adjust max and min size\n");
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
	buddy_pool->mem_start = mem_start;
	buddy_pool->mem_size = mem_size;
	buddy_pool->curr_heap_size = mem_size;	
	buddy_pool->pool_start = (void*)pool_start;
	buddy_pool->pool_size = pool_size;
	sw_memset(&buddy_pool->free_area[0], 0, sizeof(struct sw_area) * MAX_BINS);


	for(idx = 0; idx < buddy_pool->total_area_cnt; idx++) {
		buddy_pool->area[idx].mem_addr = 0;
		buddy_pool->area[idx].lchild = NULL;
		buddy_pool->area[idx].rchild = NULL;
		buddy_pool->area[idx].parent = NULL;
		buddy_pool->area[idx].cnt = 0;
		buddy_pool->area[idx].block_size = 0;
		buddy_pool->area[idx].bin = 0;
	}



	/*    sw_memset(buddy_pool->area, 0, manage_area_len); */

	buddy_pool->min_alloc_size = min_alloc_size;
	buddy_pool->max_alloc_size = max_alloc_size;
	buddy_pool->max_bin = max_bin;
	tnode_cnt = total_nodes;
	/* Create initial free list on the biggest bin */
	do {
		freenode = get_free_node(buddy_pool);
		if (!freenode) {
			sw_printf("no free node\n");
			sw_seterrno(SW_ENODATA);
			return SW_ERROR;
		}

		freenode->mem_addr = mem_start;
		freenode->bin = max_bin;
		freenode->block_size = max_alloc_size;

		mem_size -= max_alloc_size;
		mem_start += max_alloc_size;
		add_sorted_btree_node(&buddy_pool->free_area[max_bin],
				freenode);
		buddy_pool->free_area[max_bin].cnt++;
		tnode_cnt--;

	}while(tnode_cnt > 0);

	return SW_OK;
}

/**
 * @brief Buddy memory allocation routine
 *
 * This function allocates the memory from buddy pool.
 *
 * @param buddy_pool: Pointer to the buddy pool
 * @param size: Size of the memory requested
 *
 * @return : Valid memory address or NULL pointer
 */
void* sw_buddy_alloc(struct sw_buddy_pool *buddy_pool, sw_uint size)
{
	sw_big_ulong next_pow_size;
	sw_uint needed_blocks, bin, min_alloc_shift;
	struct sw_area *node;

	if (size > buddy_pool->curr_heap_size) {
		sw_printf("Request Memory Exceeds the current Heap Size\n");
		return NULL;
	}

	next_pow_size = next_pow_of_2(size);

	if (next_pow_size > buddy_pool->curr_heap_size) {
		sw_printf("Request Memory Exceeds the current Heap Size\n");
		return NULL;
	}
	
	if (next_pow_size > buddy_pool->max_alloc_size) {
		needed_blocks =  next_pow_size >> buddy_pool->max_bin;

		node = get_contiguous_block(buddy_pool, needed_blocks, buddy_pool->max_bin);
		if(node) {
			node->block_size = next_pow_size;
			buddy_pool->curr_heap_size-=node->block_size;			
			add_btree_node(&buddy_pool->alloc_tree, node);
			buddy_pool->alloc_cnt++;
			return node->mem_addr;
		}
		else {
			return NULL;
		}        
	}

	min_alloc_shift = log_of_2(buddy_pool->min_alloc_size);
	bin = log_of_2 ((next_pow_size >> min_alloc_shift));

	node = get_free_buddy(buddy_pool, bin);

	if(node != NULL) {
		buddy_pool->curr_heap_size-=node->block_size;				  
		add_btree_node(&buddy_pool->alloc_tree, node);
		buddy_pool->alloc_cnt++;
		return node->mem_addr;
	}

	return NULL;    
}

/**
 * @brief Free the previosuly allocated memory from Buddy
 *
 * This function frees the previously allocated memory and re-arrange the
 * buddies
 *
 * @param buddy_pool: Pointer to buddy pool
 * @param ptr: Allocated memory pointer which needs to be freed.
 */
void sw_buddy_free(struct sw_buddy_pool *buddy_pool, void *ptr)
{
	struct sw_area *node;
	node = search_btree_node(&buddy_pool->alloc_tree, ptr);
	if (!node) {
		sw_printf("\n");
		sw_printf("!!! Freeing a invalid address 0x%x !!!\n",(sw_uint)ptr);
		sw_printf("\n");
		return;
	}    

	buddy_pool->curr_heap_size+=node->block_size;
	remove_btree_node(&buddy_pool->alloc_tree, node);
	buddy_pool->alloc_cnt--;

	return_to_free_area(buddy_pool, node, node->bin);
	return;
}

#ifdef DEBUG_BUDDY
/**
 * @brief 
 *
 * @param buddy_pool
 */
static void dump_buddy_info(struct sw_buddy_pool *buddy_pool)
{
	sw_uint idx;
	for(idx = 0; idx <= buddy_pool->max_bin; idx++) {
		sw_printf("printing free nodes of bin 0x%x\n", idx);
		print_btree(&buddy_pool->free_area[idx]);
	}

	sw_printf("printing allocated nodes and count 0x%x\n", buddy_pool->alloc_cnt);
	print_btree(&buddy_pool->alloc_tree);
}
#endif

/**
 * @brief Get the size of the pointer
 *
 * This function returns the buddy block size of the pointer
 *
 * @param buddy_pool: Pointer to the buddy pool
 * @param pointer: Memory address for which size is requested
 *
 * @return : Return the allocated size of the pointer
 */
sw_int get_buddy_ptr_size(struct sw_buddy_pool *buddy_pool, void *ptr) 
{
	struct sw_area *node;
	
	node = search_btree_node(&buddy_pool->alloc_tree, ptr);
	if (!node) {
		return 0;
	}   

	return(node->block_size);
}
