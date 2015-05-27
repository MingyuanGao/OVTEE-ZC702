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
 * SW Wait-Queue Definition. 
 */

#ifndef __SW_WAIT_H_
#define __SW_WAIT_H_

#include <sw_types.h>
#include <sw_lock.h>
#include <sw_link.h>
#include <sw_heap.h>
#include <sw_timer.h>

#define WAKE_UP             0
#define WAKE_UP_IMMEDIATE   1    

struct task_queue_item;

/**
 * @brief 
 *
 * @param 
 * @param sw_uint
 *
 * @return 
 */
typedef void (*wakeup_function)(struct task_queue_item* , sw_uint);

/**
 * @brief
 *  Each notification element gets added to this queue.
 *  
 */
struct task_pending_queue{
	struct lock_shared lock_shared;
	int elements_count;
	struct link elements_list;
};

/**
 * @brief 
 *  queue element which requires notification
 */
struct task_queue_item{
	void* data;
	wakeup_function func;
	struct link head;
};

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
void schedule_task_function(struct task_queue_item *queue_item, sw_uint flag);

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

void sw_wakeup(struct task_pending_queue *queue, sw_uint wakeup_flag);

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

void sw_wakeup_all(struct task_pending_queue *queue, sw_uint wakeup_flag);



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
		wakeup_function func);
/**
 * @brief 
 *  Initializes notificaiton item to current task data and default wakeup 
 *  handler
 *
 * @param queue_item
 *  The notification element
 */
void sw_set_default_queue_item(struct task_queue_item *queue_item);

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
		struct task_queue_item *queue_item);

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
		struct task_queue_item *queue_item);


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
		struct task_queue_item *queue_item);


/**
 * @brief 
 *      This function sets the state of the TASK 
 * @param state
 *      The state to which the current task need to be moved
 */
void sw_set_task_state(int state);

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
		struct task_queue_item *queue_item);

/**
* @brief Adds the notification item and sets task state with locking
*
* @param queue
* @param node
*/
void sw_atomic_queue_add_and_state(struct task_pending_queue *queue, 
	struct task_queue_item *node, sw_uint state);
	
/**
 * @brief 
 *  This is called to put the current task in a pending queue.
 *  When the condition is satisfied the task is woken up. 
 *  It invokes the default handler on resume. Default handler 
 *  schedules the waiting task.
 *
 * @param queue
 *  The pending queue to which the current task need to be added
 *
 * @param condition
 *   The condition which needs to be satisfied to resume the task
 *
 * @return 
 *     Returns nothing
 */
#define sw_wait_event(queue, condition) \
do { \
	struct task_queue_item *node; 			\
	if(*condition)					\
		break;					\
	node = sw_malloc_private(COMMON_HEAP_ID, \
		sizeof(struct task_queue_item)); \
	if(node) { \
		link_init(&node->head);			\
		node->data = get_current_task();			\
		node->func = schedule_task_function;		\
		while (1) {					\
			if(*condition)				\
				break;				\
			sw_atomic_queue_add_and_state(queue, node, TASK_STATE_WAIT);\
			/* Call schedule to yield this task */	\
			schedule();				\
		}						\
		/* Comes here after the wake up */		\
		sw_wait_done(queue, node);			\
		sw_free_private(COMMON_HEAP_ID, node); \
	} \
} while(0)
	

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT

extern void set_secure_api_ret(int ret_val);


/**
 * @brief 
 *      This function is called to put the current task to sleep
 *      and notify the non-secure world about this
 *      Task pending signal is sent to non-secure world
 *      
 * @param wq_head
 *      The pending queue to which the current task need to be added
 * @param condition
 *      The condition to wake up
 * @param retval
 *      The flag to be sent to non-secure world
 * @return 
 */

#define  sw_wait_event_async(queue,  condition, retval) \
do { 							\
	struct task_queue_item *node; 			\
	if(condition)					\
		break;					\
	node = sw_malloc_private(COMMON_HEAP_ID, \
		sizeof(struct task_queue_item)); \
	if(node) { \
		link_init(&node->head);			\
		node->data = get_current_task();			\
		node->func = schedule_task_function;		\
		sw_uint call_smc = 1;				\
		while (1) {					\
			if(condition)				\
				break;				\
			sw_atomic_queue_add_and_state(queue, node, TASK_STATE_WAIT);\
			if(call_smc) {				\
				call_smc = 0;			\
				set_secure_api_ret(retval);	\
			}					\
			/* Call schedule to yield this task */	\
			schedule();				\
		}						\
		/* Comes here after the wake up */		\
		sw_wait_done(queue, node);			\
		sw_free_private(COMMON_HEAP_ID, node); \
	} \
}while(0)

#endif
#endif
