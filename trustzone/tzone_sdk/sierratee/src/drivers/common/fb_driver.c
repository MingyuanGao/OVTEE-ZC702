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
 * A Framebuffer driver
 */

#include <uart.h>
#include <sw_modinit.h>
#include <sw_debug.h>
#include <otz_id.h>
#include <fb.h>
#include <sw_device_id.h>

/**
 * @brief 
 */
static sw_int fb_dev_open(char *name)
{
	if(sw_fb_open() != SW_OK)
		return -1;
	else 
		return FB_DEVICE_ID;
}

/**
 * @brief 
 */
static sw_int fb_dev_close(void)
{
	return sw_fb_close();
}

/**
* @brief 
*
* @param fd
* @param ioctl_id
* @param req
* @param res
*
* @return 
*/
static sw_int fb_dev_ioctl(sw_int fd, sw_uint ioctl_id, void* req, void* res)
{
	return sw_fb_ioctl(ioctl_id, req, res);
}
/**
 * @brief 
 */
static struct sw_file_operations fb_dev_fops = {
	.open = fb_dev_open,
	.close = fb_dev_close,
	.ioctl = fb_dev_ioctl,
	.sw_dev_name = FB_DEVICE_NAME,
	.dev_id = FB_DEVICE_ID,
};

/**
 * @brief 
 *
 * @return 
 */
static void __init fb_dev_init(void)
{
	sw_device_register(&fb_dev_fops);

	sw_printk("\n\rSW: Framebuffer driver initialized successfully\r\n");
	
	return;
}

/**
 * @brief 
 */
static void fb_dev_exit(void)
{
	sw_device_unregister(&fb_dev_fops);
	return;
}

SW_MODULE_INIT(&fb_dev_init);
