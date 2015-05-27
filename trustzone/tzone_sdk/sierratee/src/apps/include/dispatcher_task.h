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
 * Header for Global task implementation
 */

#ifndef __OTZ_DISPATCHER_TASK_H__
#define __OTZ_DISPATCHER_TASK_H__

#include <secure_api.h>

/**
 * @brief Global variables for the task should be defined as a member 
 * of the global structure 
 */
typedef struct dispatch_global
{
	/*! Flag to notify that task completed the processing of API command */
	int secure_api_set;
	/*! Return value of API command */
	int secure_api_ret_val;
}dispatch_global;

/**
 * @brief Dispatcher task exit
 *
 * This function gets called before the task deletion
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_FAIL \n 
 */
void dispatch_task(int task_id, sw_tls *tls);

/**
 * @brief Task return value notification
 *
 * This function gets called after the API completion to notify the dispatcher 
 * task and it also sets the return value of API.
 * @param ret_val: API return value
 */
void set_secure_api_ret(int ret_val);


#endif /* __OTZ_DISPATCHER_TASK_H__ */
