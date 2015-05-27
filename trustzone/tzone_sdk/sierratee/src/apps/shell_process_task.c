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
 * Shell Command processing task implementation
 */

#include <shell_process_task.h>
#include <global.h>
#include <task.h>
#include <sw_types.h>
#include <otz_id.h>
#include <cpu_data.h>
#include <sw_board.h>
#include <sw_mem_functions.h>
#include <sw_string_functions.h>
#include <sw_gic.h>
#include <sw_board.h>
#include <uart.h>
#include <sw_mmu_helper.h>
#include <sw_app_api.h>
#include <sw_heap.h>

/**
 * @brief
 * shell_process_task - Registers keyboard interrupt, then starts the linux
 * guest by executing the environment variable "autoboot".Task is scheduled to
 * process received characters continuously 
 *
 * @param task_id 	- Task identifier
 * @param tls 		- Pointer to task local storage
 */
void shell_process_task(int task_id, sw_tls *tls)
{
	int fd, flags, r_val, items_to_read = MAX_NUM_CHAR - 1;
	char *file_path = SHELL_ENV_FILE, *buf = NULL, *cmd = NULL;
	sw_uint i = 0, j = 0, n = 0, nl_index = 0;
	static sw_bool start_linux = true;
	struct sw_task *task;
	struct shell_env *head = NULL, *env = NULL;
	
	task_init(task_id, tls);
	task = get_task(task_id);
	sw_gic_unmask_interrupt(KEY_PRESS_IRQ);
	register_secure_irq_handler(KEY_PRESS_IRQ, serial_irq_handler, (void *)task_id);

	/* Allocate memory for the shell_env list head */
	env = shell_env_alloc(env);
	if(!env)
		goto start_linux_end;
	head = env;
	if(start_linux) {
		start_linux = false;
#ifdef CONFIG_FILESYSTEM_SUPPORT
		flags = FILE_READ;
		buf = (char *)sw_malloc(MAX_NUM_CHAR);
		cmd = (char *)sw_malloc(MAX_NUM_CHAR);
		if((!buf) || (!cmd)) {
			sw_seterrno(SW_ENOMEM);
			goto start_linux_end;
		}
		fd = file_open((const char *)file_path, flags);
		if(fd != SW_ERROR) {			
			r_val = file_read(fd, buf, items_to_read);
			if(r_val > 0) {
				buf[r_val] = '\0';
				while(buf[i] != '\0') {
					while(buf[i] !='/' && buf[i] != '\0')
						cmd[j++] = buf[i++];
					nl_index = i-1;
					if(buf[nl_index] == '\n')
						j--;
					cmd[j] = '\0';
					sw_setenv(cmd, 0, env);
					execute_env(env->env_name, env);
					sw_memset((void *)cmd, NULL, j);
					j = 0,n++;
					if(buf[i] == '/')
						i++;
					if(buf[i] == '\n')
						i++;
					env = env->next;
				}
				sw_memset((void *)buf, NULL, r_val);
			}
			file_close(fd);
		}
#else
	sw_setenv("autoboot=start guest", 0, head);
	execute_env("autoboot", head);
#endif
	start_linux_end:
		if(buf)
			sw_free(buf);
		if(cmd)
			sw_free(cmd);
	}
	while(1){		
		process_input((((struct shell_global *)task->tls->private_data)->data),
				head);
		sw_set_task_state(TASK_STATE_WAIT);
		schedule();				
	}
}

/**
 * @brief 
 * process_input - Stores upto 255 the characters untill newline character is
 * received , if '\n' is received characters are sent for command processing 
 * else it will not be sent. Characters are cleared only if '\n' is received.
 *
 * @param ch	- Character to process
 * @param head	- Pointer to the environment structure which holds environmental
 * variables 
 * */
void process_input(char ch, struct shell_env *head)
{
	static char cmd[MAX_NUM_CHAR];
	sw_uint cmd_len = 0;
	static sw_uint i = 0, key_usual = 0, key_back = 0, max_char = 0;
	if((ch == MASK_KEY_UP) || (ch == MASK_KEY_DOWN))
		;
	else if(ch == MASK_KEY_RIGHT) {
		if(i < max_char)
			serial_putc(cmd[i++]);
	}
	else if(ch == MASK_KEY_LEFT) {
		if(i != 0)
			serial_putc('\b'),i--;
	}
	else if((ch == MASK_KEY_END) || (ch == MASK_KEY_HOME))
		;
	else if(ch == KEY_CARRIAGE_RETURN) {
		serial_puts("\r\n");
		cmd[max_char] = '\0';
		if(max_char) {
			for(i = 0; i < max_char; i++) {
				if(cmd[i] != ' ')
					break;
			}
			if(i == max_char)
				goto prompt;
			max_char = i = 0;
			key_usual = key_back = 0;
			execute_command(cmd, head);
		}
		goto prompt;
	}
	else if((ch == KEY_DELETE) || (ch == KEY_BACKSPACE)){
		if(i == 0)
			;
		else if(i < max_char) {
			key_back = i;
			serial_putc('\b');
			for(; key_back < max_char; key_back++) {
				cmd[key_back-1] = cmd[key_back];
			}
			cmd[--max_char] = ' ';
			key_back = --i;
			while( key_back <= max_char)
				serial_putc(cmd[key_back++]);
			while( key_back-- > i)
				serial_putc('\b');
		}
		else {
			serial_puts("\b \b");
			i--;
			max_char--;
		}
	}
	else if(ch == KEY_HORIZONTAL_TAB) {
		;
	}
	else if(ch >= KEY_SPACE) {
		if(i < max_char) {
			key_usual = max_char;
			for(; key_usual > i; key_usual--)
				cmd[key_usual] = cmd[key_usual-1];
			cmd[i++] = ch;
			if(++max_char != (MAX_NUM_CHAR - 1)) {
				while(key_usual < max_char)
					serial_putc(cmd[key_usual++]);
				while( key_usual-- > i)
					serial_putc('\b');
			}
		}
		else {	
			cmd[i++] = ch;
			serial_putc(ch);
			max_char++;	
		}
	}
	if((i == (MAX_NUM_CHAR - 1)) || (max_char == (MAX_NUM_CHAR - 1))) {
		serial_puts("\r\nCan't process the command\r\n");
prompt:
		cmd_len = sw_strnlen(cmd,MAX_NUM_CHAR);	
		if(max_char || i || key_usual || key_back) {
			sw_memset((void *)cmd, NULL, cmd_len);
			max_char = i = 0;
			key_usual = key_back = 0;
		}
		else
			sw_memset((void *)cmd, NULL, cmd_len);
		serial_puts("# ");
	}
	return; 	
}
