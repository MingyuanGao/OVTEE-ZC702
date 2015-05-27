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
 *  Shell command manager implementation
 */

#include <sw_types.h>
#include <sw_debug.h>
#include <tzhyp_global.h>
#include <sw_mem_functions.h>
#include <sw_string_functions.h>
#include <shell_process_task.h>
#include <shell_command_manager.h>
#include <global.h>
#include <sw_errno.h>

/**
 * @brief
 * shell_commands - Structure contains commands and pointer to the corresponding
 * functions
 */
struct shell_commands commands[NUM_OF_CMDS] = {
	{"help", help},
	{"setenv", sw_setenv},
	{"saveenv", sw_saveenv},
	{"printenv", sw_printenv},
	{"list", list},
	{"start", start},
	{"ps", ps}
};

/**
 * @brief
 * shell_env_alloc - Allocates memory and initializes value for environment
 * structure.
 *
 * @param env - Pointer to environment structure
 *
 * @return - On success return the structure variable for which memory is
 * allocated successfully
 */
struct shell_env *shell_env_alloc(struct shell_env *env)
{
	env = (struct shell_env *)sw_malloc(sizeof(struct shell_env));
	if(!env) {		
		sw_seterrno(SW_ENOMEM);
		goto shell_env_alloc_ret;
	}
	env->env_name = (char *)sw_malloc(MAX_NUM_CHAR);
	env->env_value = (char *)sw_malloc(MAX_NUM_CHAR);
	if(!env->env_name || !env->env_value) {
		sw_seterrno(SW_ENOMEM);
		goto shell_env_alloc_ret;
	}
	env->start_env = true;
	env->next = NULL;

shell_env_alloc_ret:
	return env;
}

/**
 * @brief
 * shell_env_add - Allocates memory for environment structure and adds it to
 * the list.
 *
 * @param env - Pointer to environment structure
 *
 * @return - On success return the structure variable which is added
 * successfully in the list.
 */
struct shell_env *shell_env_add(struct shell_env *env)
{
	if(!env) {
		env = shell_env_alloc(env);
		goto shell_env_add_ret;
	}
	while(env->next)
		env = env->next;
	env->next = shell_env_alloc(env->next);
	env = env->next;

shell_env_add_ret:
	return env;
}

/**
 * @brief
 * skip_trailing_spaces - Copies characters untill first KEY_SPACE character is 
 * seen, if KEY_SPACE character comes it will be skipped untill next non 
 * KEY_SPACE character is seen.
 *
 * @param check	- Pointer where the characters has to be copied
 * @param cmd	- Pointer whose character has to be copied
 * @param current_index - Current character count
 *
 * @return	- On success return the current character count else 0 if all the 
 * characters are KEY_SPACE
 */
sw_uint skip_trailing_spaces(char *check, char *cmd, sw_uint current_index) 
{
	sw_uint count_char = 0;
	while(cmd[current_index] != KEY_SPACE && cmd[current_index] != '\0')
		check[count_char++]	= cmd[current_index++];
   	while(cmd[current_index] == KEY_SPACE && cmd[current_index] != '\0')
		current_index++;
	check[count_char] = '\0';
	if(count_char)
		return current_index;
	else 
		return 0;
}

/**
 * @brief 
 * execute_command - Executes the corresponding command based on the string
 * passed or environmental variable is executed.
 *
 * @param cmd  	- Pointer to the command string
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 *
 * @return - On success command is executed
 * "Invalid Command" is printed on failure
 */
int execute_command(char *cmd, struct shell_env *head)
{
	struct shell_env *env = NULL;
	env = head;
	int ret_val = SW_OK;
	sw_uint current_index = 0, i = 0, cmp_val = 0;
	char *check;
	check = (char *)sw_malloc(MAX_NUM_CHAR);
	if(!check) {
		ret_val = SW_ENOMEM;
		sw_seterrno(SW_ENOMEM);
		goto execute_cmd_ret;
	}

	/* skiping white spaces */
	while(cmd[current_index] == KEY_SPACE)
		current_index++;

	current_index = skip_trailing_spaces(check, cmd, current_index);
	if(!current_index)
		goto execute_cmd_ret;

	while(i < NUM_OF_CMDS) {
		cmp_val = sw_strncmp(commands[i].cmd_name,(const char*)check,
													MAX_NUM_CHAR);
		if(cmp_val == 0) {
			(commands[i].function)(cmd, current_index, env);
			goto execute_cmd_ret;
		}
		i++;
	}
	if(env->start_env)
		execute_env(check, env);
	else
		sw_printk("Invalid Command\r\n");

execute_cmd_ret:
	if(check)
		sw_free(check);
	return ret_val;
}

/**
 * @brief
 * execute_env - Executes the value of the environmental variable using its
 * name. "Invalid Command" is printed if name is not found in list.
 *
 * @param name - Name of the environmental variable whose value to be executed
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 */
void execute_env(char *name, struct shell_env *head)
{
	struct shell_env *env = NULL;
	env = head;
	sw_uint j = 0, k = 0, cmp_val = 0;
	char *local_buf, *check;
	local_buf = (char *)sw_malloc(MAX_NUM_CHAR);
	check = (char *)sw_malloc(MAX_NUM_CHAR);
	if((!local_buf) || (!check)) {
		sw_seterrno(SW_ENOMEM);
		goto execute_env_ret;
	}
    while(env) {
		/* check for matching name in env_variables*/
		cmp_val = sw_strncmp((const char*)name,
						(const char*)env->env_name,MAX_NUM_CHAR);
		if(cmp_val == 0) {
			sw_strncpy(check, env->env_value, MAX_NUM_CHAR-1);
			check[MAX_NUM_CHAR-1] = '\0';
            /* saperate values based on semicolon */
			while(check[j] != '\0') {
				while((check[j] != KEY_SEMICOLON) && (check[j] != '\0'))
					local_buf[k++] = check[j++];
				local_buf[k] = '\0';
				head->start_env = false;
				execute_command(local_buf, head);
				head->start_env = true;
				sw_memset((void *)local_buf, NULL, k);
				k = 0;
				if(check[j] == KEY_SEMICOLON)
					j++;
			}
			goto execute_env_ret;
		}
		env = env->next;
	}
	sw_printk("Invalid Command\r\n");

execute_env_ret:
	if(local_buf)
		sw_free(local_buf);
	if(check)
		sw_free(check);
	return;
}

/**
 * @brief
 * help - Prints information about each commands
 *
 * @param cmd  	- Pointer to the command string
 * @param current_index - Current character count
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 */
void help(char *cmd, sw_uint current_index, struct shell_env *head)
{   
	if(cmd[current_index] == '\0') {
		sw_printk("help		");
		sw_printk("Display information about commands\r\n");
		sw_printk("list images	");
		sw_printk("Display information about images\r\n");
		sw_printk("setenv		");
		sw_printk("Set environment variables\r\n");
		sw_printk("printenv	");
		sw_printk("Print environment variables\r\n");
		sw_printk("saveenv		");
		sw_printk("Save environment variables\r\n");
		sw_printk("start guest	");
		sw_printk("Start linux guest\r\n");
		sw_printk("ps		");
		sw_printk("Print details of all processes\r\n");
		return;
	}
	sw_printk("Invalid Argument\r\n");
	sw_seterrno(SW_EINVAL);
	return;
}

/**
 * @brief
 * sw_setenv	- Populates the environment structure and adds it to the list 
 * on success. Memory for list head should be allocated before passed into the
 * function. On failure error message is printed.
 *
 * @param cmd  	- Pointer to the command string
 * @param current_index - Current character count
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 */
void sw_setenv(char *cmd, sw_uint current_index, struct shell_env *head)
{
	char *name, *value;
	sw_uint i = 0, all_space = 0, count_char = 0, name_len = 0, value_len = 0;
	static sw_bool env_init = false;
	struct shell_env *env = NULL;
	env = head;
	name = (char *)sw_malloc(MAX_NUM_CHAR);
	value = (char *)sw_malloc(MAX_NUM_CHAR);
	if((!name) || (!value)) {
		sw_seterrno(SW_ENOMEM);
		goto setenv_ret;
	}
	/* skip white spaces */
	while(cmd[current_index] == KEY_SPACE)
		current_index++;

	if(cmd[current_index] != '\0') {
		while(cmd[current_index] != KEY_EQUAL_TO && 
			cmd[current_index] != KEY_SPACE && cmd[current_index] != '\0')
			name[count_char++] = cmd[current_index++];

		name[count_char] = '\0';
		if((count_char != 0) && (cmd[current_index] == KEY_EQUAL_TO)) {
			count_char = 0;
			current_index++;
			while(cmd[current_index] != '\0')
				value[count_char++] = cmd[current_index++];
			value[count_char] = '\0';

			while(value[i] != '\0') {
				if(value[i] == KEY_SPACE)
					all_space++;
				i++;
			}
			if(all_space != count_char) {
				name_len = sw_strnlen(name,MAX_NUM_CHAR);
				value_len = sw_strnlen(value,MAX_NUM_CHAR);
				if((name_len >= (MAX_NUM_CHAR - 1)) || 
					(value_len >= (MAX_NUM_CHAR - 1))) {
					sw_printk("Can't process.....\r\n");
					sw_seterrno(SW_ENOMEM);
					goto setenv_ret;
				}	
				if(env_init) {
					env = shell_env_add(env);
				}
				if(env) {
					sw_strncpy(env->env_name, name, name_len);
					env->env_name[name_len]='\0';
					sw_strncpy(env->env_value, value, value_len);
					env->env_value[value_len]='\0';
					if(!env_init)
						env_init = true;
				}
				goto setenv_ret;
			}
			sw_printk("Invalid Argument\r\n");
			sw_seterrno(SW_EINVAL);
			goto setenv_ret;
		}
		sw_printk("Invalid syntax, env name should be followed by =\r\n");
		sw_seterrno(SW_EINVAL);
		goto setenv_ret;
	}
	sw_printk("Arguments is needed to process the command\r\n");
	sw_seterrno(SW_EINVAL);

setenv_ret:
	if(name)
		sw_free(name);
	if(value)		
		sw_free(value);
	return;
}

/**
 * @brief
 * sw_saveenv 	- Writes the environmental variables into a file on success and
 * also prints the same. On failure error message is printed.
 *
 * @param cmd  	- Pointer to the command string
 * @param current_index - Current character count
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 */
void sw_saveenv(char *cmd, sw_uint current_index, struct shell_env *head)
{
	return;
}

/**
 * @brief
 * sw_printenv	- Prints environmental variables name and its value if valid 
 * arguments is passed. On failure error message is printed.
 *
 * @param cmd  	- Pointer to the command string
 * @param current_index - Current character count
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 */
void sw_printenv(char *cmd, sw_uint current_index, struct shell_env *head)
{
	struct shell_env *env = NULL;
	if(cmd[current_index] == '\0') {
		env = head;
		if(env) {
			do {
				sw_printk("%s ", env->env_name);
				sw_printk("%s\r\n", env->env_value);
				env = env->next;
			}while(env);
            goto printenv_ret;
		}
		sw_printk("Environment variables doesn't exist\r\n");
		sw_seterrno(SW_EINVAL);
		goto printenv_ret;    		
	}
	sw_printk("Invalid Argument\r\n");
	sw_seterrno(SW_EINVAL);

printenv_ret:
	return;
}

/**
 * @brief
 * list - Displays information about the guest image on success. 
 * Error message is printed on failure.
 *
 * @param cmd  	- Pointer to the command string
 * @param current_index - Current character count
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 */
void list(char *cmd, sw_uint current_index, struct shell_env *head)
{
	char *check;
	sw_uint cmp_val = 0;
	const char *arg1 = "images";
	check = sw_malloc(MAX_NUM_CHAR);
	if(!check) {
		sw_seterrno(SW_ENOMEM);
		goto list_ret;
	}
	if(cmd[current_index] != '\0') {
		current_index = skip_trailing_spaces(check, cmd, current_index);
		if(!current_index) {
			sw_printk("Invalid Argument\r\n");
			sw_seterrno(SW_EINVAL);
			goto list_ret;
		}
        if(cmd[current_index] == '\0') {
			cmp_val = sw_strncmp(arg1,(const char*)check,6);
			if(cmp_val == 0) {
				display_boot_info_nsk();
				goto list_ret;
			}
			sw_printk("Invalid Argument\r\n");
			sw_seterrno(SW_EINVAL);
			goto list_ret;
		}
		sw_printk("Enter one argument only\r\n");
		sw_seterrno(SW_EINVAL);
		goto list_ret;
	}
	sw_printk("Argument is needed to process the command\r\n");
	sw_seterrno(SW_EINVAL);

list_ret:
	if(check)
		sw_free(check);
	return;
}

/**
 * @brief
 * start - Launches current guest OS on success.
 * Error message is printed on failure.
 *
 * @param cmd  	- Pointer to the command string
 * @param current_index - Current character count
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 */
void start(char *cmd, sw_uint current_index, struct shell_env *head) 
{
	char *check;
	sw_uint cmp_val = 0;
	const char *arg1 = "guest";
	check = sw_malloc(MAX_NUM_CHAR);
	if(!check) {
		sw_seterrno(SW_ENOMEM);
		goto start_ret;
	}
	if(cmd[current_index] != '\0') {
		current_index = skip_trailing_spaces(check, cmd, current_index);
		if(!current_index) {
			sw_printk("Invalid Argument\r\n");
			sw_seterrno(SW_EINVAL);
			goto start_ret;
		}
		if(cmd[current_index] == '\0') {
			cmp_val = sw_strncmp(arg1,(const char*)check,5);
			if(cmp_val == 0) {
				start_guest();
            	goto start_ret;
			}
			sw_printk("Invalid Argument\r\n");
			sw_seterrno(SW_EINVAL);
			goto start_ret;
		}
		sw_printk("Enter one argument only\r\n");
		sw_seterrno(SW_EINVAL);
		goto start_ret;
	}
	sw_printk("Argument is needed to process the command\r\n");
	sw_seterrno(SW_EINVAL);

start_ret:
	if(check)
		sw_free(check);
	return;
}

/**
 * @brief
 * start_guest - Launches the current guest OS.
 */
void start_guest(void)
{  
	static sw_bool guest_started = false;
	if(!guest_started) {
		guest_started = true;
		sw_printk("Linux Guest started.....\r\n");
		launch_current_guest();
	}
	else
		sw_printk("Linux Guest already started.....\r\n");
	return;
}

/**
 * @brief
 * ps - Displays details about each process on success. 
 * Error message is printed on failure.
 *
 * @param cmd  	- Pointer to the command string
 * @param current_index - Current character count
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 */
void ps(char *cmd, sw_uint current_index, struct shell_env *head)
{
	if(cmd[current_index] == '\0') {
		sw_printk("TASK_ID SERVICE_ID STATE\tTASK_NAME\r\n");
		all_task_status();	
	}
	else {
		sw_printk("Invalid Argument\r\n");
		sw_seterrno(SW_EINVAL);
	}
	return;
}
