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
 * UART command manager declarations
 */

#ifndef __UART_COMMAND_MANAGER__
#define __UART_COMMAND_MANAGER__

#include <sw_types.h>

#define SHELL_ENV_FILE "/test.dir/env.txt"

#define NUM_OF_CMDS	7

#ifndef MAX_NUM_CHAR
#define MAX_NUM_CHAR	256
#endif

/**
 * @brief 
 * shell_env - Structure contains environmental variables name and
 * value, a flag to start environment variable execution
 */
struct shell_env {
	char *env_name;
	char *env_value;
	char start_env;
	struct shell_env *next;
};

/**
 * @brief
 * shell_commands - Structure contains commands and pointer to the corresponding
 * functions
 */
struct shell_commands {
	const char *cmd_name;
	void (*function)(char *cmd, sw_uint current_index,
			struct shell_env *head);
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
struct shell_env *shell_env_alloc(struct shell_env *env);

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
struct shell_env *shell_env_add(struct shell_env *env);

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
sw_uint skip_trailing_spaces(char *check, char *cmd, sw_uint current_index); 

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
int execute_command(char *cmd, struct shell_env *head);

/**
 * @brief
 * execute_env - Executes the value of the environmental variable using its
 * name. "Invalid Command" is printed if name is not found in list.
 *
 * @param name - Name of the environmental variable whose value to be executed
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 */
void execute_env(char *name, struct shell_env *head);

/**
 * @brief
 * help - Prints information about each commands
 *
 * @param cmd  	- Pointer to the command string
 * @param current_index - Current character count
 * @param head - Pointer to the environment structure which holds environmental
 * variables
 */
void help(char *cmd, sw_uint current_index, struct shell_env *head);

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
void sw_setenv(char *cmd, sw_uint current_index, struct shell_env *head);

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
void sw_saveenv(char *cmd, sw_uint current_index, struct shell_env *head);

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
void sw_printenv(char *cmd, sw_uint current_index, struct shell_env *head);

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
void list(char *cmd, sw_uint current_index, struct shell_env *head);

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
void start(char *cmd, sw_uint current_index, struct shell_env *head);

/**
 * @brief
 * start_guest - Launches the current guest OS.
 */
void start_guest(void);

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
void ps(char *cmd, sw_uint current_index, struct shell_env *head);

#endif
