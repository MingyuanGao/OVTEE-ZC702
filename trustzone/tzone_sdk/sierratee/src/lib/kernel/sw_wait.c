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
 * SW Wait-Queue Implementation.
 */
#include <task.h>
#include <sw_wait.h>
#include <sw_debug.h>
#include <sw_timer.h>
#include <global.h>
#include <debug_config.h>

/** @ingroup OS_KernelApi Kernel API for Secure OS
 *  API for Sierra OS Kernel tasks
 *  @{
 */
/**
 * @brief 
 *      This function is called to add the notification element to the 
 *      pending queue without locking
 * @param queue
 *      Task pending queue to which the element need to be added
 * @param queue_item
 *      The notification element
 */
void sw_no_lock_pending_queue_add(struct task_pending_queue *queue, 
		struct task_queue_item *queue_item)
{
	set_link_data(&queue_item->head, queue_item);
	add_link(&queue->elements_list, &queue_item->head, TAIL);
	queue->elements_count++;
	return;
}

/**
 * @brief 
 *      This function is called to add the notification element to the 
 *      pending queue
 * @param queue
 *      Task pending queue to which the element need to be added
 * @param wq
 *      The notification element
 */
void sw_pending_queue_add(struct task_pending_queue *queue, 
		struct task_queue_item *queue_item)
{
	sw_uint cpsr;
	lock_irq_shared_resource(&queue->lock_shared, &cpsr);
	sw_no_lock_pending_queue_add(queue, queue_item);
	unlock_irq_shared_resource(&queue->lock_shared, cpsr);
}

/**
 * @brief 
 *      This function is called to remove the notification element from 
 *      the pending queue
 * @param queue
 *      Task pending queue to which the element need to be deleted
 * @param queue_item
 *      The notification element
 */
void sw_pending_queue_remove(struct task_pending_queue *queue, 
		struct task_queue_item *queue_item)
{
	sw_uint cpsr;
	lock_irq_shared_resource(&queue->lock_shared, &cpsr);
	remove_link(&queue_item->head);
	queue->elements_count--;
	unlock_irq_shared_resource(&queue->lock_shared, cpsr);
}

/**
 * @brief 
 *  Initializes notificaiton item to current task data and default wakeup 
 *  handler
 *
 * @param queue_item
 *  The notification element
 */
void sw_set_default_queue_item(struct task_queue_item *queue_item)
{
	if(queue_item){
		queue_item->data = get_current_task();
		queue_item->func = schedule_task_function;
	}
	return;
}

/**
 * @brief 
 *      Assign wakup for the notification entry
 *
 * @param queue_item
 *      Notification element
 * @param func
 *      The function to be called while waking up the element fom pending queue
 */
void sw_init_wakeup_func(struct task_queue_item *queue_item,
		wakeup_function func)
{
	if(queue_item){
		queue_item->data = get_current_task();
		queue_item->func = func;
	}
	return;
}

/**
 * @brief 
 *      This function sets the state of the TASK 
 * @param state
 *      The state to which the current task need to be moved
 */
void sw_set_task_state(int state)
{
	int task_id = get_current_task_id();
	suspend_task(task_id, state);
}

/**
 * @brief    
 *      This function is called when the task is woken up and this function
 *      removes the notification element from the pending queue
 * @param queue
 *      Task pending queue to which this element is attached
 * @param wq
 *      The notification element
 */
void sw_wait_done(struct task_pending_queue *queue, 
		struct task_queue_item *queue_item)
{
	sw_uint cpsr;

	if(!link_empty(&queue_item->head)){
		lock_irq_shared_resource(&queue->lock_shared, &cpsr);
		remove_link(&queue_item->head);
		link_init(&queue_item->head);
		queue->elements_count--;
		unlock_irq_shared_resource(&queue->lock_shared, cpsr);
	}

	return;
}

/**
 * @brief 
 *      This function wakes up all the elements attached to the given 
 *      queue
 * @param queue
 *      The queue from which all the elements need to be woken up
 *      
 * @param wakeup_flag
 *      It says whether the element need to be woken up immediately or can be
 *      woken up later
 */

void sw_wakeup_all(struct task_pending_queue *queue, sw_uint wakeup_flag)
{
	struct link *l, *head;
	sw_uint cpsr;
	struct task_queue_item *queue_item;

	lock_irq_shared_resource(&queue->lock_shared, &cpsr);

	head=  &queue->elements_list;
	l = head->next;
	while (l != head) {
		queue_item = l->data;
		if(queue_item) {
			unlock_irq_shared_resource(&queue->lock_shared, cpsr);
			queue_item->func(queue_item, wakeup_flag);
			lock_irq_shared_resource(&queue->lock_shared, &cpsr);
		}
		l = l->next;
	}
	
	unlock_irq_shared_resource(&queue->lock_shared, cpsr);

}

/**
 * @brief 
 *      This function wakes up an element from the given queue
 *
 * @param queue
 *      The queue from which all the elements need to be woken up
 * @param wakeup_flag
 *      It says whether the element need to be woken up immediately or can be
 *      woken up later
 */

void sw_wakeup(struct task_pending_queue *queue, sw_uint wakeup_flag)
{
	struct link *l, *head;
	sw_uint cpsr;
	struct task_queue_item *queue_item;

	lock_irq_shared_resource(&queue->lock_shared, &cpsr);

	head=  &queue->elements_list;
	l = head->next;
	while (l != head) {
		queue_item = l->data;
		if(queue_item) {
			unlock_irq_shared_resource(&queue->lock_shared, cpsr);
			queue_item->func(queue_item, wakeup_flag);
			return;
		}
		l = l->next;
	}
	
	unlock_irq_shared_resource(&queue->lock_shared, cpsr);

}

/**
 * @brief 
 *     This function wakes up a task by changing the state of the task and
 *     adding it to the ready to run list 
 * @param queue_item
 *      The queue element which need to be waken up
 * @param wakeup_flag
 *      It says whether the element need to be woken up immediately or can be
 *      woken up later
 */
void schedule_task_function(struct task_queue_item *queue_item, sw_uint wakeup_flag)
{
	sw_uint cpsr;
	struct sw_task* task = queue_item->data;

	lock_irq_shared_resource(&global_val.ready_to_run_lock, &cpsr);

	task->state = TASK_STATE_READY_TO_RUN;
	if(wakeup_flag == WAKE_UP_IMMEDIATE)
		schedule_task_next(task);
	else
		schedule_task(task);

	unlock_irq_shared_resource(&global_val.ready_to_run_lock, cpsr);

	if(wakeup_flag == WAKE_UP_IMMEDIATE)
		schedule();

	return;
}


/**
* @brief Adds the notification item and sets task state with locking
*
* @param queue
* @param node
*/

void sw_atomic_queue_add_and_state(struct task_pending_queue *queue, 
		struct task_queue_item *node, sw_uint state)
{
	sw_uint cpsr;
	lock_irq_shared_resource(&queue->lock_shared, &cpsr); 
	if(link_empty(&node->head)){				
		sw_no_lock_pending_queue_add(queue, node);	
	}					
	sw_set_task_state(state);	
	unlock_irq_shared_resource(&queue->lock_shared,cpsr);	
}

/** @} */ // end of OS_KernelApi
