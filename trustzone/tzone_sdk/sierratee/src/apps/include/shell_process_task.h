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
 * Shell Commands processing functions
 */

#ifndef __SHELL_PROCESS_TASK_H__
#define __SHELL_PROCESS_TASK_H__

#include <task.h>
#include <sw_types.h>
#include <shell_command_manager.h>

#define KEY_BACKSPACE			0x08
#define KEY_HORIZONTAL_TAB		0x09
#define KEY_CARRIAGE_RETURN		0x0D
#define KEY_SPACE				0x20
#define KEY_DOUBLE_QUOTES		0x22
#define KEY_SINGLE_QUOTES		0x27
#define KEY_SEMICOLON			0x3B
#define KEY_EQUAL_TO			0x3D
#define KEY_DELETE				0x7F

/* Special Keys */
#define KEY_ESCAPE				0x1B
#define KEY_ESCAPE_SEQUENCE		0x5B	/* Escape character for special keys */
#define KEY_PAGE_UP				0x35
#define KEY_PAGE_DOWN			0x36
#define KEY_UP					0x41
#define KEY_DOWN				0x42
#define KEY_RIGHT				0x43
#define KEY_LEFT				0x44
#define KEY_END					0x46
#define KEY_HOME				0x48

#define MASK_KEY_UP				128
#define MASK_KEY_DOWN			129	
#define MASK_KEY_RIGHT			130	
#define MASK_KEY_LEFT			131
#define MASK_KEY_END			133
#define MASK_KEY_HOME			134

#define MAX_NUM_CHAR			256

/**
 * @brief
 * shell_global - Structure holds the character received from the keyboard and
 * uart_id where the interrupt is received
 */
struct shell_global
{
	char data;
	sw_uint uart_id;
};

extern void serial_irq_handler(sw_uint irq, void* data);

/**
 * @brief
 * shell_process_task - Registers keyboard interrupt, then starts the linux
 * guest by executing the environment variable "autoboot".Task is scheduled to
 * process received characters continuously 
 *
 * @param task_id 	- Task identifier
 * @param tls 		- Pointer to task local storage
 */
void shell_process_task(int task_id,sw_tls *tls);

/**
 * @brief 
 * process_input - Stores upto 255 the characters untill newline character is
 * received , if '\n' is received characters are sent for command processing 
 * else it will not be sent. Characters are cleared only if '\n' is received.
 *
 * @param ch 	- Character to process
 * @param head  - Head of the structure shell_env which holds environmental
 * variables
 */
void process_input(char ch, struct shell_env *head);

#endif

