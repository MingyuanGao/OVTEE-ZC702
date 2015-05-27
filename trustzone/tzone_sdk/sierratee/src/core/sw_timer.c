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
 * Timer framework Implementation.
 */

#include <sw_types.h>
#include <sw_timer.h>
#include <global.h>
#include <task.h>
#include <sw_buddy.h>
#include <sw_debug.h>
#include <sw_mem_functions.h>
#include <global.h>
#include <sw_heap.h>
#include <debug_config.h>


/** @defgroup timerapi Kernel Timer APIs. 
 *  Internal Timer APIs. Can be used directly inside the kernel or via POSIX User apis
 *  @{
 */

/**
 * @brief 
 * The elements of this structure are useful
 * for implementing the sw_timer
 */
static struct sw_tmr_info{

	sw_timeval sw_timestamp;
	sw_big_ulong abs_cycles_count;
	sw_big_ulong cycles_count_old;
	sw_big_ulong cycles_count_new;

	sw_timeval timer_period;
	sw_timeval clock_period;
}sw_timer_info;

/**
 * @brief
 * This function initializes the Timer structure variables
 */
void init_sw_timer(void )
{
	/* TODO :Later, this function should be made to read values
	 * from Device Tree */
	int iter;

	global_val.timer_cpu_info.next_timelen.tval64 = TIMEVAL_MAX;
	global_val.timer_cpu_info.num_events = 0;

	for(iter = 0; iter < MAX_NUM_OF_TIMERS; iter++)
	{
		global_val.timer_cpu_info.clock_info[iter].cpu_info = &global_val.timer_cpu_info;
		global_val.timer_cpu_info.clock_info[iter].clock_id = iter;
		link_init(&global_val.timer_cpu_info.clock_info[iter].active);
		RESET_BUSY_LOCK(&global_val.timer_cpu_info.clock_info[iter].lock_shared);
		global_val.timer_cpu_info.clock_info[iter].clock_period.tval64 = 
			get_clock_period();
		global_val.timer_cpu_info.clock_info[iter].timer_period.tval64 = 
			get_timer_period();
	}

	/* TODO :Later this should be made to obtain from Device Tree */
	global_val.timer_cpu_info.free_run_clock_id = 0;
	global_val.timer_cpu_info.tick_timer_id = 1;

#define SW_FREE_RUNNING_CNTR    global_val.timer_cpu_info.free_run_clock_id
#define SW_TICKTIMER            global_val.timer_cpu_info.tick_timer_id


	sw_timer_info.sw_timestamp.tval64 = 0;
	sw_timer_info.abs_cycles_count = 0;
	sw_timer_info.cycles_count_new = 0;
	sw_timer_info.cycles_count_old = 0;
	sw_timer_info.timer_period.tval64 = get_timer_period();
	sw_timer_info.clock_period.tval64 = get_clock_period();

}

/**
 * @brief 
 *      This function adds the two elements and returns the sum
 * @param value1
 *      Time value 1
 * @param value2
 *      Time value 2
 * @return
 *      The Sum of the input arguments
 */
static sw_big_ulong add_time(const sw_timeval *value1, const sw_timeval *value2)
{
	sw_timeval sum;
	sum.tval64 = value1->tval64 + value2->tval64;

	if(sum.tval.nsec >  (NANO_SECONDS_PER_SECOND- 1)){
		sum.tval.nsec -= NANO_SECONDS_PER_SECOND;
		sum.tval.sec += 1;
	}

	return sum.tval64;
}

/**
 * @brief 
 *      This function subtracts value 1 from value 2 and
 *      returns the difference
 * @param value1
 *      Time value 1
 * @param value2
 *      Time value 2
 * @return 
 *      The difference between the Input arguments
 */
sw_big_ulong subtract_time(const sw_timeval *value1 , const sw_timeval *value2)
{
	sw_timeval diff;
	diff.tval64 = value1->tval64 - value2->tval64;

	if(diff.tval.nsec < 0){
		diff.tval.nsec += NANO_SECONDS_PER_SECOND;
	}

	return diff.tval64;
}


/**
 * @brief 
 *      This function adds the timer event to the list;
 *      The list elements are such that they are in the ascending order of time
 *      of expiry
 *
 * @param head
 *      The head of the list
 * @param new
 *      The new event to be added
 */
static void timer_queue_add(struct link* head , struct timer_event* new )
{
	struct link *l;
	struct timer_event *temp;
	int added_to_list = 0;


	if(link_empty(head)){
		set_link_data(&new->node, new);
		add_link(head, &new->node, TAIL);
		added_to_list = 1;
	}
	else {
		l = head->next;
		while (l != head) {
			temp = l->data;
			if(new->timelen.tval64 < temp->timelen.tval64){
				set_link_data(&new->node, new);
				add_link(&temp->node, &new->node, TAIL);			
				added_to_list = 1;
				break;
			}
			l = l->next;
		}
	}

	if(!added_to_list) {
		set_link_data(&new->node, new);	
		add_link(head, &new->node, TAIL);
	}

	new->state = TIMER_STATE_ACTIVE;

	return;
}

/**
 * @brief 
 *      This function deletes the timer event from the 
 *      list
 * @param tevent
 *      The event which needs to be deleted
 */
static void timer_queue_del(struct timer_event* tevent )
{

	if(!tevent)
		return;

	remove_link(&tevent->node);
	tevent->state = TIMER_STATE_INACTIVE;
}

/**
 * @brief 
 *      This function returns the next element in the list
 * @param head
 *      The element whose next element need to be returned
 * @return 
 *      -The next element 
 *      -NULL : If the next element is not present 
 */
static struct link* timer_queue_getnext(struct link* head)
{
	struct link* next;

	if(link_empty(head))
		next = NULL;
	else
		next =  head->next;

	return next;
}

/**
 * @brief
 *  This function updates the timestamp
 *  This function is called only when the free running counter interrupt is
 *  generated.(When free running counter reaches zero this interrupt arises and
 *  after that the counter reload itself to the max value(mostly 0xFFFFFFFF) and
 *  continue to count down)
 *
 *  Otherwise the timestamp is updated by read_timestamp function
 */
void free_running_cntr_intr(void)
{
	sw_big_ulong tmp_cycles;
	sw_timeval tmp_timeval;

	sw_timer_info.cycles_count_new = TIMER_COUNT_MAX;   
	tmp_cycles = sw_timer_info.cycles_count_new - 
		sw_timer_info.cycles_count_old;

	sw_timer_info.abs_cycles_count += tmp_cycles;
	sw_timer_info.cycles_count_old = 0;

	tmp_timeval.tval64 = clockcycles_to_timeval(tmp_cycles);
	sw_timer_info.sw_timestamp.tval64 = add_time(&sw_timer_info.sw_timestamp,
			&tmp_timeval);
	return;
}

/**
 * @brief 
 *      It updates and returns the current timestamp value
 * @return 
 *      The current timestamp value
 */
sw_big_ulong read_timestamp(void)
{
	sw_big_ulong tmp_cycles;
	sw_timeval tmp_timeval;

	sw_timer_info.cycles_count_new = read_freerunning_cntr();
	tmp_cycles = sw_timer_info.cycles_count_new - 
		sw_timer_info.cycles_count_old;

	sw_timer_info.abs_cycles_count += tmp_cycles;
	sw_timer_info.cycles_count_old = sw_timer_info.cycles_count_new;

	tmp_timeval.tval64 = clockcycles_to_timeval(tmp_cycles);
	sw_timer_info.sw_timestamp.tval64 = add_time(&sw_timer_info.sw_timestamp,
			&tmp_timeval);
	return sw_timer_info.sw_timestamp.tval64;
}


/**
 * @brief 
 *      This function is called to create a new timer event.
 *      It allocates the memory for timer event struct, initializes the
 *      elements and returns the structure.
 *      If it cannot allocate the memory then it returns NULL
 *
 * @param handler
 *      The handler which will be called on the expiration of the timer
 * @param priv_data
 *      The data which may be needed by the handler
 *
 * @return
 *      -The timer_event structure allocated and initialized
 *      -NULL if it fails to allocate the structure
 */
struct timer_event* timer_event_create(sw_timer_event_handler handler,
		void* priv_data)
{
	struct timer_event* new_event;

	new_event = (struct timer_event*)sw_malloc_private(COMMON_HEAP_ID,
			sizeof(struct timer_event));
	if(!new_event){
		sw_printk("SW: Malloc failed in Creating New Timer event \n");
		return NULL;
	}

	sw_memset( new_event, 0, sizeof(struct timer_event));

	new_event->clk_info = &global_val.timer_cpu_info.clock_info[SW_TICKTIMER];
	link_init(&new_event->node);
	new_event->handler = (void*)handler;
	new_event->timelen.tval64 = TIMEVAL_MAX;
	new_event->state = TIMER_STATE_INACTIVE;
	new_event->data = priv_data;

	return new_event;
}

/**
 * @brief 
 *      This function deletes the timer event structure
 *      After expiration of the timer ,the application can choose to keep
 *      and reuse the same structure again or delete it.If it decides to delete
 *      the structure then this function is called.
 *      This function frees the memory
 *
 * @param tevent
 *      The structure which needs to be deleted
 *
 * @return
 *       0 on success 
 *      -1 on failure
 */
int timer_event_destroy(struct timer_event* tevent)
{
	if(!tevent) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	if(tevent->state == TIMER_STATE_INACTIVE){
		sw_free_private(COMMON_HEAP_ID, tevent);		
		return SW_OK;
	}
	sw_printk("SW: Timer Event in USE :Cannot Free this Timer Event \n");
	sw_seterrno(SW_EBUSY);
	return SW_ERROR;
}

/**
 * @brief 
 *      Sets the absolute expire time to the timer event
 * @param tevent
 *      timer event structure for which absolute expire time need to be assigned
 * @param abs_time
 *      Absolute expire time
 */
static void timer_event_set_expire(struct timer_event *tevent, 
		sw_timeval *abs_time)
{
	tevent->timelen = *abs_time;
}

/**
 * @brief 
 *      The expire time is obtained from the given timer event structure
 * @param tevent
 *      The timer event structure
 * @return 
 *  Absolute expire time
 */
static sw_big_ulong timer_get_expire(struct timer_event *tevent)
{
	return tevent->timelen.tval64;
}

/**
 * @brief 
 *      This function programs the hardware (by calling appropriate functions)
 *      to generate an interrupt at the timelen time(the argument)
 *
 * @param timelen
 *      The time at which an interrupt need to be generated
 */
static void program_tick_for_event(sw_big_ulong timelen_time)
{
	sw_timeval time_till_next_tick;
	sw_big_ulong clockcycles;

	sw_timeval timelen;
	sw_timeval now;

	timelen.tval64 = timelen_time;
	now.tval64 = read_timestamp();
	time_till_next_tick.tval64  = subtract_time(&timelen, &now);

	if((time_till_next_tick.tval.sec < 0) ||
			(time_till_next_tick.tval.nsec < 0)) {
		clockcycles = 1;
		goto set_trigger;
	}

	clockcycles = timeval_to_clockcycles(&time_till_next_tick);

set_trigger:
	global_val.timer_cpu_info.next_timelen = timelen;

	trigger_tick(clockcycles);
	return;
}

/**
 * @brief 
 *      It starts the timer event by calling the appropriate functions
 *      The time interval is written to the hardwire timer and the event is
 *      added to the list of events to be handled on timer expiry
 * @param tevent
 *      The timer event which needed to be started
 * @param time
 *      The time duration after which the event need to be expire  
 */
void timer_event_start(struct timer_event *tevent, sw_timeval *time)
{
	sw_timeval temp_time;
	sw_uint cpsr;

	if(!tevent)
		return ;

	temp_time.tval64 = read_timestamp();
	time->tval64 =  add_time(&temp_time, time);

	timer_event_set_expire(tevent, time);

	lock_irq_shared_resource(&tevent->clk_info->lock_shared, &cpsr);
	timer_queue_add(&tevent->clk_info->active, tevent); 
	unlock_irq_shared_resource(&tevent->clk_info->lock_shared, cpsr);


	if(global_val.timer_cpu_info.next_timelen.tval64 > tevent->timelen.tval64){
		program_tick_for_event(tevent->timelen.tval64);
	}
	return;
}
/**
 * @brief 
 *      This function is called from the timer interrupt function
 *      This function calls the handler function of the expired timer
 * @param tevent
 *      The event whose handler need to be called
 */
static void exec_timer_handler(struct timer_event* tevent)
{
	sw_timer_event_handler hndl;

	global_val.timer_cpu_info.num_events++;

	/* No need for spinlock as it is called from Interrupt context */
	timer_queue_del(tevent);

	hndl = (sw_timer_event_handler)tevent->handler;

	if(hndl)
		hndl(tevent);

	return;
}

/**
 * @brief
 *      This is the function which is called on expiry of any timer event.
 *      This fn goes through the list of registered timer events. If the expiry
 *      time of the event is already in the past then the corresponding 
 *      handler function is called.
 *      As the list is already sorted according to their time of
 *      expiration, the function returns either when it encounters the first event
 *      whose expiry time is in the future or when the list becomes empty.
 */
void timer_interrupt(void)
{
	struct timer_clock_info* clock_info;
	struct link *node;    
	struct timer_event* tevent;

	sw_timeval next_timelen, now, temp_time, time_diff;
	next_timelen.tval64 = TIMEVAL_MAX;

	clock_info = &global_val.timer_cpu_info.clock_info[SW_TICKTIMER];

	while((node = timer_queue_getnext(&clock_info->active))){

		tevent = node->data;
		temp_time.tval64 = timer_get_expire(tevent);

		now.tval64 = read_timestamp();


		time_diff.tval64 = subtract_time(&temp_time, &now);

		if((time_diff.tval.sec > 0) || 
				((time_diff.tval.sec == 0) && time_diff.tval.nsec > 1000)) { 
			/*        if( now.tval64 < temp_time.tval64){ */
			sw_timeval timelen;

			timelen = temp_time;

			if( timelen.tval64 < next_timelen.tval64) 
				next_timelen = timelen;

			program_tick_for_event(next_timelen.tval64);
			break;
		}

		tevent->state = TIMER_STATE_EXECUTING;
		exec_timer_handler(tevent);
		}

		global_val.timer_cpu_info.next_timelen.tval64 = TIMEVAL_MAX;
		return;
	}

	/**
	 * @brief 
	 *      checks whether the timer event is active 
	 * @param tevent
	 *      The timer event whose state need to be checked
	 * @return 
	 *      SW_OK - If active
	 *      SW_ERROR - If not active
	 */
	sw_int is_timer_event_active(struct timer_event* tevent)
	{
		if(!tevent)
			return SW_ERROR;

		if(tevent->state & TIMER_STATE_ACTIVE)
			return SW_OK;
		else
			return SW_ERROR;
	}

	/**
	 * @brief 
	 *      checks whether the timer event is waiting in callback mode
	 * @param tevent
	 *      The timer event whose state need to be checked
	 * @return 
	 *      SW_OK - If it is waiting on callback
	 *      SW_ERROR - If it is not waiting on callback
	 */
	sw_int is_timer_event_waiting_on_callback(struct timer_event* tevent)
	{
		if(!tevent)
			return SW_ERROR;

		if (tevent->state & TIMER_STATE_PENDING)
			return SW_OK;
		else 
			return SW_ERROR;
	}

	/**
	 * @brief
	 *      Stops a timer_event
	 *
	 * @param tevent
	 */
	void timer_event_stop(struct timer_event* tevent)
	{
		struct timer_clock_info* clock_info;
		struct timer_event* next_event;
		sw_timeval timelen,next_timelen;
		sw_uint cpsr;

		if(is_timer_event_waiting_on_callback(tevent) == SW_OK)
			return;

		if(is_timer_event_active(tevent) == SW_OK){

			clock_info = &global_val.timer_cpu_info.clock_info[SW_TICKTIMER];

			lock_irq_shared_resource(&tevent->clk_info->lock_shared, &cpsr);

			if(&tevent->node == timer_queue_getnext(&clock_info->active)){

				unlock_irq_shared_resource(&tevent->clk_info->lock_shared,
						cpsr);
				timelen.tval64 = timer_get_expire(tevent); 

				if(timelen.tval64 == global_val.timer_cpu_info.next_timelen.tval64){

					if(tevent->node.next == &clock_info->active)
						next_timelen.tval64 = TIMEVAL_MAX;
					else{
						next_event = tevent->node.next->data;
						next_timelen = next_event->timelen;
					}

					if(next_timelen.tval64 != global_val.timer_cpu_info.next_timelen.tval64)
						program_tick_for_event(next_timelen.tval64);

				}
			}
			else {
				unlock_irq_shared_resource(&tevent->clk_info->lock_shared, cpsr);
			}

			lock_irq_shared_resource(&tevent->clk_info->lock_shared, &cpsr);
			timer_queue_del(tevent);
			unlock_irq_shared_resource(&tevent->clk_info->lock_shared, cpsr);

		}
		return;
	}

/** @} */ // end of timerapi
