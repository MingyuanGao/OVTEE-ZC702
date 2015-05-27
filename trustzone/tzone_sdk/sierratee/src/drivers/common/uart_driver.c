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
 * A UART driver ## Have to be merged with orginal board specific uart driver.
 */

#include <uart.h>
#include <sw_modinit.h>
#include <sw_debug.h>
#include <otz_id.h>
#include <sw_device_id.h>

/**
* @brief 
*
* @param name
*
* @return 
*/
static sw_int uart_dev_open(char *name)
{
	return UART_DEVICE_ID;
}

/**
* @brief 
*
* @param fd
*
* @return 
*/
static sw_int uart_dev_close(sw_int fd)
{
	return SW_OK;
}

/**
* @brief 
*
* @param fd
* @param buf
* @param count
*
* @return 
*/
static sw_int uart_dev_read(sw_int fd, void* buf, sw_uint count)
{
	sw_seterrno(SW_ENOSYS);
	return -1;
}

/**
* @brief 
*
* @param fd
* @param buf
* @param count
*
* @return 
*/
static sw_int uart_dev_write(sw_int fd, void* buf, sw_uint count)
{
	sw_printk(buf);
	return count;
}

/**
* @brief 
*
* @param fd
* @param req
* @param res
*
* @return 
*/
static sw_int uart_dev_ioctl(sw_int fd, void *req, void *res)
{
	sw_seterrno(SW_ENOSYS);
	return -1;
}
/**
 * @brief 
 */
static struct sw_file_operations uart_dev_fops = {
	.open = uart_dev_open,
	.close = uart_dev_close,
	.read = uart_dev_read,
	.write = uart_dev_write,
	.ioctl = uart_dev_ioctl,
	.sw_dev_name = UART_DEVICE_NAME,
	.dev_id = UART_DEVICE_ID,
};

/**
 * @brief 
 *
 * @return 
 */
static void __init uart_dev_init(void)
{
	sw_device_register(&uart_dev_fops);

	sw_printk("\n\rSW: UART driver initialized successfully\r\n");
	return;
}

/**
 * @brief 
 */
static void uart_dev_exit(void) __attribute__((used));

/**
 * @brief 
 */
static void uart_dev_exit(void)
{
	sw_device_unregister(&uart_dev_fops);
	return;
}

SW_MODULE_INIT(&uart_dev_init);
