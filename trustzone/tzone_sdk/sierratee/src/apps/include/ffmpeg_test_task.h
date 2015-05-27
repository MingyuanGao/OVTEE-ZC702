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
 * Virtual Keybiard Tester implementation
 */

#include <sw_types.h>
#include <tls.h>

#define NUM_ARGS 4
#define ARG_LEN  32

int process_otz_ffmpeg_test_svc(sw_uint svc_cmd_id);


/**
 * @brief
 * global variables for the task should be defined as a member of the global 
 * structure 
 */
typedef struct ffmpeg_test_global {
}ffmpeg_test_global;

/**
 * @brief FFmpeg test task entry point
 *
 * This function implements the commands to test the mutex and 
 * semaphore operations
 *
 * @param task_id: Task identifier
 * @param tls: Pointer to task local storage
 */
void ffmpeg_test_task(int task_id, sw_tls* tls);

