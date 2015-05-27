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
 * Implementation of sleep and other timer based functions.
 */

#include "sw_timer_functions.h"
#include "sw_io.h"
#include <sw_timer.h>
#include <sw_types.h>
#include <sw_debug.h>
#include <task.h>
#include <global.h>
#include "sw_board.h"
#include <debug_config.h>
#include <sw_heap.h>

/**
 * @brief 
 *      This function wakes up the task which has called sleep
 *
 * @param tevent
 *      The timer_event structure used for putting this function to sleep
 */
void wake_up_from_sleep(struct timer_event* tevent)
{
	int task_id = *(int*)(tevent->data);

	/* This is called from interrupt context so no locking may require */
	schedule_task_id(task_id);
	tevent->state &= ~TIMER_STATE_EXECUTING;
	
	sw_free_private(COMMON_HEAP_ID, tevent->data);
	timer_event_destroy(tevent);
	return;
}

/**
 * @brief 
 *      This function sleeps for given number of seconds
 * @param secs
 *      Number of secs to sleep
 */
void sw_sleep(sw_uint secs)
{
	sw_timeval time;
	time.tval.nsec = 0;
	time.tval.sec = secs;

	int *current_context;
	struct timer_event *tevent;

	current_context = sw_malloc_private(COMMON_HEAP_ID,sizeof(int));

	*(current_context) = get_current_task_id();

	tevent = timer_event_create(&wake_up_from_sleep, current_context);
	if(!tevent){
		sw_printk("SW: Out of memory : Cannot Perform Sleep\n");
		return;
	}

	timer_event_start(tevent, &time);

	suspend_task(*(current_context), TASK_STATE_WAIT);
	schedule();
	return;
}

/**
 * @brief 
 *      This function sleeps for given number of micro-seconds
 * @param usecs
 *      Number of micro-seconds to sleep
 */

void task_usleep(sw_uint usecs)
{
	int *current_context;
	struct timer_event *tevent;
	sw_timeval time;

	time.tval.sec = (usecs >> 19) - (usecs >> 20);
	
/* After seconds has been calculated, find out remaining nonoseconds */
	time.tval.nsec = (time.tval.sec << 10) + (time.tval.sec << 3) - (time.tval.sec << 5); /* Multiply seconds by 1000 */
	time.tval.nsec = (time.tval.nsec << 10) + (time.tval.nsec << 3) - (time.tval.nsec << 5); /* Multiply result by 1000 */
	
	time.tval.nsec = usecs - time.tval.nsec; /* Find remaining microseconds */
	time.tval.nsec = (time.tval.nsec << 10) + (time.tval.nsec << 3) - (time.tval.nsec << 5); /* Convert msecs to nsecs by multiplying 1000 */
	
	current_context = sw_malloc_private(COMMON_HEAP_ID,sizeof(int));
	*(current_context) = get_current_task_id();

	tevent = timer_event_create(&wake_up_from_sleep, current_context);
	if(!tevent){
		sw_printk("SW: Out of memory : Cannot Perform Sleep\n");
		return;
	}

	suspend_task(*(current_context), TASK_STATE_WAIT);
	timer_event_start(tevent, &time);
}

/**
 * @brief 
 *      This function sleeps for given number of micro-seconds and schedules the next task
 * @param usecs
 *      Number of micro-seconds to sleep
 */
void sw_usleep(sw_uint usecs)
{
	task_usleep(usecs);
	schedule();
	return;
}

