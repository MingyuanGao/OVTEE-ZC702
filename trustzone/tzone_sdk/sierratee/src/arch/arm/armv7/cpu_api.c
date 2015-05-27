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
 * API's specific to architecture
 */
#include <sw_types.h>
#include <sw_board.h>
#include <sw_debug.h>
#include <global.h>
#include <cpu.h>
#include <sw_gic.h>
#include <secure_timer.h>
#include <task.h>
#include <debug_config.h>

/** @defgroup kerneltimer Kernel Low level Timer
 *  Low level timer implementation. Internal to the kernel
 *  @{
 */

/**
 * @brief Init secure kernel timer
 * 
 * This function initialize the secure kernel timer
 */
void timer_init(void)
{
	secure_timer_init(FREE_RUNNING_TIMER_BASE);
	secure_timer_init(TICK_TIMER_BASE);
}

/**
 * @brief Enable secure kernel timer
 * 
 * This function enables the secure kernel timer
 */
void enable_timer(void)
{
	secure_timer_init(FREE_RUNNING_TIMER_BASE);
	secure_set_timer(FREE_RUNNING_TIMER_BASE,0,
			MODE_FREE_RUNNING, TIMER_COUNT_MAX);
	secure_timer_enable(FREE_RUNNING_TIMER_BASE, 0);
}

/**
 * @brief Disable secure kernel timer
 * 
 * This function disables the secure kernel timer
 */
void disable_timer(void)
{
	secure_timer_disable(FREE_RUNNING_TIMER_BASE, 0);
}


/**
 * @brief 
 *
 * @param usecs
 */
void trigger_tick(sw_big_ulong usecs)
{
	if(usecs > TIMER_COUNT_MAX)
		usecs = TIMER_COUNT_MAX;

	if(usecs == 0) {
		usecs = 1;
	}

	secure_set_timer(TICK_TIMER_BASE, 0, MODE_ONESHOT, (sw_uint)usecs);
	secure_timer_enable(TICK_TIMER_BASE, 0);
}

/**
 * @brief 
 *
 * @param clockcycles
 *
 * @return 
 */
sw_big_ulong clockcycles_to_timeval(sw_uint clockcycles)
{
	sw_timeval time;
	sw_big_ulong tmp_nsecs;
	sw_big_ulong tmp_secs;
	sw_big_ulong tmp_usecs;
	sw_big_ulong tmp_calc;
	sw_big_ulong usecs;


	/* Divide by 1000
	 * x/1000 = x>>10 + 3*x>>17 +9*x>>24;
	 *   or
	 *   y = x>>10;
	 * ==> x/1000 =  y + (3*y)>>7 + (9*y)>>14
	 */

	usecs  =  clockcycles * get_clock_period_us();

	/* Find number of seconds */
	tmp_secs = usecs;
	tmp_secs = (tmp_secs >> 10);
	tmp_secs = tmp_secs + ((3 * tmp_secs)>>7) + ((9 * tmp_secs)>>14);

	tmp_secs = (tmp_secs >> 10);
	tmp_secs = tmp_secs + ((3 * tmp_secs)>>7) + ((9 * tmp_secs)>>14);

	/* Multiply number of seconds with 10^6 to convert it to number of
	 * clockcycles and then subtract it from the total number of
	 * clockcycles to obtain the number of microseconds
	 */
	/* Multiply by thousand
	 * 1000 = 1024 + 8 - 32
	 *      = 2^10 + 2^3 - 2^5
	 */
	tmp_calc = (tmp_secs << 10) + (tmp_secs << 3) - (tmp_secs << 5);
	tmp_calc = (tmp_calc << 10) + (tmp_calc << 3) - (tmp_calc << 5);
	tmp_usecs = usecs - tmp_calc;

	tmp_nsecs = (tmp_usecs<<10) + (tmp_usecs <<3) - (tmp_usecs<<5);

	time.tval.nsec = tmp_nsecs;
	time.tval.sec = tmp_secs;

	return time.tval64;
}

/**
 * @brief 
 *
 * @param time
 *
 * @return 
 */
sw_big_ulong timeval_to_clockcycles(sw_timeval *time)
{
	sw_big_ulong useconds = 0;
	sw_big_ulong clockcycles;
	sw_big_ulong tmp_sec , tmp_nsec;

	if((time->tval.sec < 0) || (time->tval.nsec < 0))
		return 0;

	tmp_sec = time->tval.sec;

	if(tmp_sec > 0) {
		/* multiply by 1000 * 1000 */
		tmp_sec = (tmp_sec<<10) + (tmp_sec <<3) - (tmp_sec<<5);
		tmp_sec = (tmp_sec<<10) + (tmp_sec <<3) - (tmp_sec<<5);
	}

	/* Divide by 1000
	 * x/1000 = x>>10 + 3*x>>17 +9*x>>24;
	 *   or
	 *   y = x>>10;
	 * ==> x/1000 =  y + (3*y)>>7 + (9*y)>>14
	 */
	tmp_nsec = time->tval.nsec;
	tmp_nsec = tmp_nsec >> 10;
	tmp_nsec = tmp_nsec + ((3 * tmp_nsec)>>7) + ((9*tmp_nsec)>>14);

	useconds = tmp_nsec;
	useconds += tmp_sec;

	clockcycles = (sw_uint)useconds / (sw_uint)get_clock_period_us();

	/* Minimum clock cycles to be 1 clock period */
	if(clockcycles == 0)
		clockcycles = get_clock_period_us(); 
	return clockcycles;
}

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
/**
 * @brief Invoke non-secure kernel callback handler
 *
 * @param guest_no: guest_no
 * @param svc_id: Service ID
 * @param session_id: Session ID
 * @param enc_id: Encoded context ID
 * @param client_pid: Client process ID
 */
void invoke_ns_callback(sw_int guest_no, sw_uint svc_id, sw_uint session_id, sw_uint enc_id, 
		sw_uint client_pid, sw_uint dev_file_id)
{
	set_notify_data(guest_no, svc_id, session_id, enc_id, client_pid, dev_file_id);
	return;
}
#endif

/** @} */ // end of kernel_lowlevel_timer
