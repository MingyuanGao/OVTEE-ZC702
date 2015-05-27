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
 * FFmpeg test implementation
 */

#include <ffmpeg_test_task.h>
#include <otz_id.h>
#include <sw_debug.h>
#include <task_control.h>
#include <sw_user_app_api.h>
/**
 * @brief 
 *
 * @param svc_cmd_id
 *
 * @return 
 */
int process_otz_ffmpeg_test_svc(sw_uint svc_cmd_id) {
	switch (svc_cmd_id) {
		case OTZ_FFMPEG_TEST_CMD:
			play_video("/lib/out.rgb");
			break;
	}
	return SW_OK;
}


/**
 * @brief FFmpeg test task entry point
 *
 * This function implements the processing of ffmpeg test commands.
 *
 * @param task_id: task identifier
 * @param tls: Pointer to task local storage
 */
void ffmpeg_test_task(int task_id,sw_tls* tls)
{
	task_init(task_id, tls);
	tls->ret_val = process_otzapi(task_id, tls);

	task_exit(task_id, tls);
	while(1);
}


