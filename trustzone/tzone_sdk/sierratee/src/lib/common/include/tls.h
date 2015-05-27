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

#ifndef _TLS_H
#define _TLS_H

#include <sw_types.h>
#include <sw_buddy.h>
#include <otz_common.h>


typedef int (*process_fn)(sw_uint svc_cmd_id,void *req_buf, sw_uint req_buf_len,
			void *resp_buf, sw_uint resp_buf_len, struct otzc_encode_meta *
			meta_data, sw_uint *ret_res_buf_len); 
typedef struct sw_tls
{
	/*! Command parameters */
	sw_uint params[4];
	/*! Return value of the secure API */
	sw_uint ret_val;
	/*! Return value of the IPC */
	sw_uint ipi_ret_val;
	/* Status of task_init */
	sw_uint task_init_done;
	struct sw_heap_info heap_info;
	/*! process pointer */
	process_fn process;
	/*! Heap size of the task */
	sw_uint       heap_size;
	/*! Min_alloc_size of the task */
    sw_uint       min_alloc_size;
	/*! task id */
	sw_uint       task_id;
	/*! Error No */
	sw_int task_errno;
	/*! Private data */
	void *private_data;
	/*! File data */
	void *file_data;
}sw_tls;
#endif 
