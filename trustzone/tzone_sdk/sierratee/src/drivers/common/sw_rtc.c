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

/* RTC Driver Framework 
 * 
 * Hardware clocks are present on devices which record the current "wall clock" time. These are called "Real Time Clocks"
 * (RTCs).  The have battery power backup so that they function even when the computer is turned off. RTC's provide alarms and other interrupts.
 * 
 * This is not a system clock. The difference between a RTC and a system clock is that a system clock cannot function when the system is in a low power state while the RTC can (even when it's off)
*/

#include <sw_modinit.h>
#include <sw_debug.h>
#include <sw_user_mgmt.h>
#include <otz_id.h>
#include <sw_device_id.h>
#include <sw_rtc.h>
/**
* @brief Open the RTC device
*
* @param name - Name of the device
*
* @return 
*/
sw_int rtc_dev_open(char *name)
{
	return RTC_DEVICE_ID;
}

/**
* @brief Close the RTC device
*
* @param fd - File descriptor of the device
*
* @return 
*/
sw_int rtc_dev_close(sw_int fd)
{
	return SW_OK;
}

/**
* @brief Read from an RTC
*
* @param fd - File Descriptor
* @param buf
* @param count
*
* @return - Returns the epoch value
*/
sw_big_ulong rtc_dev_read(sw_int fd, void* buf, sw_uint count)
{
	/* This must return the epoch value */
	return 0;
}

/**
* @brief Write into RTC
*
* @param fd - File Descriptor
* @param buf
* @param count
*
* @return 
*/
sw_int rtc_dev_write(sw_int fd, void* buf, sw_uint count)
{
	return NULL;
}

/**
* @brief  RTC IOCTL
*
* @param fd - File Descriptor
* @param req
* @param res
*
* @return 
*/
sw_int rtc_dev_ioctl(sw_int fd, void *req, void *res)
{
	sw_seterrno(SW_ENOSYS);
	return -1;
}
/**
 * @brief RTC File operations structure
 */
static struct sw_file_operations rtc_dev_fops = {
	.open = rtc_dev_open,
	.close = rtc_dev_close,
	.read = rtc_dev_read,
	.write = rtc_dev_write,
	.ioctl = rtc_dev_ioctl,
	.sw_dev_name = RTC_DEVICE_NAME,
	.dev_id = RTC_DEVICE_ID,
};



/**
 * @brief RTC device intialization function.
 *
 * @return n/A
 */
static void __init rtc_dev_init(void)
{
	sw_device_register(&rtc_dev_fops);

	return;
}

/**
 * @brief  Device exit
 * Unregisters the device
 */
static void rtc_dev_exit(void)
{
	sw_device_unregister(&rtc_dev_fops);
	return;
}

SW_MODULE_INIT(&rtc_dev_init);
