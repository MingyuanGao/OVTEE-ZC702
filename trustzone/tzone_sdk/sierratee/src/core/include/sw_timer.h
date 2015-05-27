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
 * SW Timer declarations.
 */

#ifndef __TIMER_FRAME_WORK_
#define __TIMER_FRAME_WORK_

#include <sw_types.h>
#include <sw_link.h>
#include <sw_lock.h>
#include <sw_time.h>

/** @addtogroup timerapi Kernel Timer APIs. 
 *  Internal Timer APIs. Can be used directly inside the kernel or via POSIX User apis
 *  @{
 */

#define NANO_SECONDS_PER_SECOND   1000000000L
/*Seconds will be set to maximum value and the number of nanoseconds
 * will be zero */
#define TIMEVAL_MAX     \
	(((sw_big_long)~((sw_big_ulong)1 << 63)) & \
		 (~((sw_big_ulong)0xFFFFFFFF)))
	
#define MAX_NUM_OF_TIMERS 2

/*The timer event is Inactive*/
#define    TIMER_STATE_INACTIVE     0x00
/* The timer event is active and is waiting for expiration*/
#define    TIMER_STATE_ACTIVE       0x01
/*The timer is expired and is waiting on the callback list to be executed*/
#define    TIMER_STATE_PENDING      0x02
/* The timer event is currently being executed */
#define    TIMER_STATE_EXECUTING    0x04


/**
 * @brief 
 */
struct timer_clock_info {
	sw_timeval   clock_period;
	sw_timeval   timer_period;
	int    clock_id;

	struct lock_shared lock_shared;
	struct link active;    


	int shift;
	unsigned long mult;

	struct timer_cpu_info* cpu_info;
};

/**
 * @brief 
 */
struct timer_cpu_info {

	int free_run_clock_id;
	int tick_timer_id;

	sw_timeval next_timelen;
	unsigned long num_events;
	struct timer_clock_info clock_info[MAX_NUM_OF_TIMERS];
};

/**
 * @brief 
 */
struct timer_event {
	struct link node;  

	sw_timeval   timelen;
	struct timer_clock_info* clk_info;
	int (*handler)(void* );
	int state;
	void* data;
};

typedef void (*sw_timer_event_handler) (struct timer_event*);

/**
 * @brief 
 *
 * @param handler
 * @param priv_data
 *
 * @return 
 */
struct timer_event* timer_event_create(sw_timer_event_handler handler, void* priv_data);

/**
 * @brief 
 *
 * @param tevent
 *
 * @return 
 */
int timer_event_destroy(struct timer_event* tevent);

/**
 * @brief 
 *
 * @param tevent
 * @param time
 */
void timer_event_start(struct timer_event *tevent, sw_timeval *time);

/**
 * @brief 
 *
 * @param value1
 * @param value2
 *
 * @return 
 */
sw_big_ulong subtract_time(const sw_timeval *value1 , const sw_timeval *value2);

/**
 * @brief 
 */
void free_running_cntr_intr(void);

/**
 * @brief 
 *
 * @return 
 */
sw_big_ulong read_timestamp(void);

/**
 * @brief 
 */
void timer_interrupt(void);

/**
 * @brief 
 */
void init_sw_timer(void );

/**
 * @brief 
 *
 * @return 
 */
sw_uint get_clock_period_us(void);

/** @} */ // end of timerapi

#endif
