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
 * User Management implementation functions
 */

#include <global.h>
#include <sw_buddy.h>
#include <task.h>
#include <sw_user_mgmt.h>
#include <sw_debug.h>
#include <otz_id.h>
#include <sw_heap.h>
#include <debug_config.h>

#ifdef DEBUG_ACL
#define DEBUG(fmt, args...) sw_printk(fmt,## args)
#else
#define DEBUG(fmt, args...)
#endif

static struct acl_device * get_next_acl_device(struct link *head, 
				struct link *current, 
				struct link **next)
{
	if(current != head) {
		*next = current->next;
		return current->data;
	}
	*next = NULL;
	return NULL;
}

static struct acl_group * get_next_acl_group(struct link *head, 
				struct link *current, 
				struct link **next)
{
	if(current != head) {
		*next = current->next;
		return current->data;
	}
	*next = NULL;
	return NULL;
}

static struct acl_user * get_next_acl_user(struct link *head, 
				struct link *current, 
				struct link **next)
{
	if(current != head) {
		*next = current->next;
		return current->data;
	}
	*next = NULL;
	return NULL;
}

/**
 * @brief 
 *
 * @param d_id
 *
 * @return 
 */
int get_permission(int d_id)
{
	struct link *current, *next = NULL;
	struct acl_device *acld;
	struct acl_group *aclg;
	struct acl_user *aclu;
	
	int found = -1, dev_found = 0, group_found = 0;
	struct sw_task *exec_task;
	if(global_val.exec_mode == 1) {
		exec_task = get_current_task();
		do {
			current = global_val.device_acl_list.next;
			do {
				acld = get_next_acl_device(&global_val.device_acl_list, 
								current, &next);
				if(!next)
					break;
				if(!acld)	
					break;
				if(acld->did == d_id) {
					dev_found = 1;
					break;	
				}
				current = next;
			}while(current);

			if(!dev_found)
				break;
				
			next = NULL;
			current = acld->group_head.next;
			do {
				aclg = get_next_acl_group(&acld->group_head, 
								current, &next);
				if(!next)
					break;
				if(!aclg)	
					break;
				if(aclg->gid == exec_task->acl.gid) {
					group_found = 1;
					break;	
				}
				current = next;
			}while(current);
			
			if(!group_found)
				break;
							
			next = NULL;
			current = aclg->user_head.next;
			do {
				aclu = get_next_acl_user(&aclg->user_head, 
								current, &next);
				if(!next)
					break;
				if(!aclu)	
					break;
				if(aclu->uid == exec_task->acl.uid) {
					found = 1;
					break;	
				}
				current = next;
			}while(current);
		} while(0);
	}else{
		found = 1;
	}
	return found;
}

/**
 * @brief 
 *
 * @param d_id
 * @param g_id
 * @param u_id
 */
void update_global_ACL(int d_id, int g_id, int u_id){  
	struct link *current, *next = NULL;
	struct acl_device *acld;
	struct acl_group *aclg = NULL;
	struct acl_user *aclu;
	int found = -1;
	
	current = global_val.device_acl_list.next;
	do {
		acld = get_next_acl_device(&global_val.device_acl_list, 
						current, &next);
		if(!next)
			break;
		if(!acld)	
			break;
		if(acld->did == d_id) {
			found = 1;
			break;	
		}
		current = next;
	}while(next);
	
	if(found == 1) {
		next = NULL;
		current = acld->group_head.next;
		do {
			aclg = get_next_acl_group(&acld->group_head, 
							current, &next);
			if(!next)
				break;
			if(!aclg)	
				break;
			if(aclg->gid == g_id) {
				found = 2;
				break;	
			}
			current = next;
		}while(next);

	
	}
		
	if(found == 2) {
		next = NULL;
		current = aclg->user_head.next;
		do {
			aclu = get_next_acl_user(&aclg->user_head, 
							current, &next);
			if(!next)
				break;
			if(!aclu)	
				break;
			if(aclu->uid == u_id) {
				found = 3;
				break;	
			}
			current = next;
		}while(next);
	}

	if(found == 3){
		DEBUG("ACL with DEVICE: %x USER: %x GROUP: %x already exist.\n", d_id, u_id, g_id);
		return;
	}
	if(found < 3){
		aclu = (acl_user *) sw_malloc_private(COMMON_HEAP_ID,(sizeof(acl_user)));
		if(aclu == NULL){
			tee_panic("Heap allocation failed while creating user Global ACL\n");
		}
		link_init(&aclu->head);
		aclu->uid = u_id;
	}
	if(found == 2){
		set_link_data(&aclu->head, aclu);
		add_link(&aclg->user_head, &aclu->head, TAIL);
		DEBUG("USER: %x added to GROUP: %x DEVICE: %x\n", u_id, g_id, d_id);
	}
	if(found < 2){
		aclg = (acl_group *) sw_malloc_private(COMMON_HEAP_ID,(sizeof(acl_group)));
		if(aclg == NULL){
			tee_panic("Heap allocation failed while creating group Global ACL\n");
		}
		link_init(&aclg->head);
		link_init(&aclg->user_head);				
		aclg->gid = g_id;

		set_link_data(&aclu->head, aclu);
		add_link(&aclg->user_head, &aclu->head, TAIL);
	}
	if(found == 1){
		set_link_data(&aclg->head, aclg);
		add_link(&acld->group_head, &aclg->head, TAIL);
		DEBUG("USER: %x GROUP:%x added to DEVICE: %x\n", u_id, g_id, d_id);
	}
	if(found < 1){
		acld = (acl_device *) sw_malloc_private(COMMON_HEAP_ID,(sizeof(acl_device)));
		if(acld == NULL){
			tee_panic("Heap allocation failed while creating device Global ACL\n");
		}
		link_init(&acld->head);
		link_init(&acld->group_head);
				
		acld->did = d_id;
		set_link_data(&aclg->head, aclg);
		add_link(&acld->group_head, &aclg->head, TAIL);

		set_link_data(&acld->head, acld);
		add_link(&global_val.device_acl_list, &acld->head, TAIL);

		DEBUG("Created GlobalACL for USER: %x GROUP:%x to access DEVICE: %x\n", u_id, g_id, d_id);
	}
}
