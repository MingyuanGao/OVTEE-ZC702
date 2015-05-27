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
 * syscalls implementation
 */

#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sw_debug.h>
#include <sw_mem_functions.h>
#include <sw_string_functions.h>
#include <sw_syscall.h>
#include <sw_file_tls.h>
#include <sw_stat.h>
#include <tls.h>
#include <syscalls.h>
#include <sw_user_heap.h>
#include <sw_time.h>

/** @defgroup OS_UserSyscalls Syscalls for User Tasks
 *  Syscalls for Sierra OS User tasks
 *  @{
 */
char *__env[1] = {0};

/**
 * @brief function  will  flushes  the stream pointed to by fd
 *
 * @param file file descriptor of the file
 *
 * @return Upon successful completion 0 is returned.
 *			Otherwise, EOF is returned and errno is
 *			set  to  indicate the  error
 */
int _close(int file) 
{
	sw_printf("file system configuration is not defined. \
			Enable CONFIG_FILESYSTEM_SUPPORT.\n");

	errno= EBADF;
	return (-1);
}

/**
 * @brief These functions return information about a file 
 *
 * @param file file descriptor of the file
 * @param st file pointed to by path and fills in st structure
 *
 * @return On success, zero is returned.  On error, -1 is returned,
 *			and errno is set appropriately
 */
int _fstat(int file, struct stat *st) 
{
	errno = EBADF;
	return(-1);
}

/**
 * @brief function tests whether fd is an open file descriptor
 *									referring to a terminal 
 *
 * @param file file descriptor of the file
 *
 * @return returns 1 if fd is an open file descriptor referring to
 *			a terminal; otherwise 0 is returned,and errno is set to
 *			indicate the error 
 */
int _isatty(int file) 
{
	if((file == SW_STDERR) ||(file == SW_STDOUT) || (file == SW_STDIN)) {
		errno = 0;
		return(1);
	}
	if(file < MAX_OPEN_FILES){
		errno = ENOTTY;
		return(0);
	}
	errno = EBADF;
	return(0);
}

/**
 * @brief function repositions the offset of the open file associated
 *			with the file descriptor fd 
 *
 * @param file file descriptor of the file
 * @param offset offset assigned according to whence as follows
 *					SEEK_SET,SEEK_CUR and SEEK_END
 * @param flags Moving position from offset 
 *
 * @return Upon successful completion, lseek() returns the resulting
 *			offset location as measured in  bytes  from the beginning
 *			of the file.  Otherwise, a value of (off_t) -1 is returned
 *			and errno is set to indicate the error 
 */
off_t _lseek(int file, off_t offset, int flags)
{
	off_t local_offset = offset;

	sw_tls *seek_tls;
	seek_tls=get_user_tls();

	FILE *file_pointers;
	file_pointers=((sw_tls_file *)seek_tls->file_data)->file_lib_pointers;
	if(!file_pointers) {
		sw_printf("Error in lseek\n");
		errno= ENOMEM;
		return (-1);
	}

	if((file_pointers[file].fd == SW_STDERR) ||
			(file_pointers[file].fd == SW_STDOUT) || 
			(file_pointers[file].fd == SW_STDIN)) {
		return(offset);
	}
	else {
	sw_printf("file system configuration is not defined. \
			Enable CONFIG_FILESYSTEM_SUPPORT.\n");

	errno= EBADF;
	return (-1);
	}
	errno = 0;
	return(local_offset);
}

/**
 * @brief Using This function get current free
 *			FILE structure  
 *
 * @return returns current free index of FILE structure
 */
static int get_free_index()
{
	int i = FILE_CNTR_START_INDEX;

	sw_tls *index_tls;
	index_tls=get_user_tls();

	FILE *file_pointers;
	file_pointers=((sw_tls_file *)index_tls->file_data)->file_lib_pointers;
	if(!file_pointers) {
		sw_printf("Error in get_free_index\n");
		errno= EBADF;
		return (-1);
	}

	while(i < MAX_OPEN_FILES) {
		if(file_pointers[i].fd == 0) {
			return(i);
		}
		i += 1;
	}
	return(0);
}

/**
 * @brief function  opens  the  file whose name is the string
 *			pointed to by path and associates a stream with it 
 *
 * @param name name of the file to be open
 * @param flags Mode Translated flag value for file to be open
 * @param ... optional third argument
 *
 * @return Returns the Index the of the FILE structure Otherwise,
 *			NULL is returned and errno is set to indicate the error
 */
int _open(const char *name, int flags, ...) 
{ 
	sw_printf("file system configuration is not defined. \
			Enable CONFIG_FILESYSTEM_SUPPORT.\n");

	errno= EBADF;
	return (-1);
}

/**
 * @brief reads nmemb elements of data, each size bytes long,
 *			from the stream pointed to by stream, storing them
 *			at the location given by fd 
 *
 * @param file file descriptor of the file
 * @param ptr readed data stores from memory location of ptr
 * @param len length of data to be read 
 *
 * @return the number of items successfully read
 *			If an error occurs, or the end-of-file is reached
 *			the return value  is  a  short  item count (or zero) 
 */
int _read(int file, char *ptr, int len)
{
	int bytes_read = 0;

	sw_tls *read_tls;
	read_tls=get_user_tls();

	FILE *file_pointers;
	file_pointers=((sw_tls_file *)read_tls->file_data)->file_lib_pointers;
	if(!file_pointers) {
		sw_printf("Error in fread\n");
		errno= ENOMEM;
		return (-1);
	}

	if(len == 0)
		return 0;
	if((file_pointers[file].fd == SW_STDERR) ||
			(file_pointers[file].fd == SW_STDOUT) || 
			(file_pointers[file].fd == SW_STDIN)) {
	} else {
		sw_printf("file system configuration is not defined. \
				Enable CONFIG_FILESYSTEM_SUPPORT.\n");

		errno= EBADF;
		return (-1);
	}
	errno = 0;
	return(bytes_read);
}

/**
 * @brief sbrk system call implementation
 *
 * @param incr address of the location of the program break,
 *				which defines the end of the process's data segment
 *
 * @return On success,returns  zero.
 *			On error, -1 is returned, and errno is set.
 */
static caddr_t sw_sys_sbrk(int incr) 
{
	unsigned char *heap_end = 0;
	unsigned char *heap_low = 0,*heap_top = 0;
	unsigned int  heap_size;
	
	unsigned int prev_heap_end; 
	
	__sw_sbrk((unsigned int*)&heap_low, &heap_size, &prev_heap_end);

	if(heap_low) {
		heap_top = heap_low + heap_size;
	}

	if (prev_heap_end == 0) {
		heap_end = heap_low;
		prev_heap_end = (unsigned int)heap_end;
	}
	else {
		heap_end = (unsigned char*)prev_heap_end;
	}
	
	if (heap_end + incr > heap_top) {
		return (caddr_t)0;
	} 
	heap_end += incr;
	__sw_sbrk_update((unsigned int)heap_end);
	return (caddr_t) prev_heap_end;
}



/**
 * @brief change data segment size 
 *
 * @param incr change of the location of the program break,
 *              which defines the end of the process's data segment
 *
 * @return On success,returns  zero.
 *          On error, -1 is returned, and errno is set.
 */
caddr_t _sbrk(int incr) 
{
	return sw_sys_sbrk(incr);
}

/**
 * @brief writes nmemb elements of data, each size bytes long,
 *			to the stream  pointed  to by stream, obtaining them
 *			from the location given by ptr
 *
 * @param file file descriptor of the file
 * @param ptr contains data to be write in a file
 * @param len length of data to be read
 *
 * @return  the number of items successfully read
 *          If an error occurs, or the end-of-file is reached
 *          the return value  is  a  short  item count (or zero)
 */
int _write(int file, char *ptr, int len) 
{
	int todo = 0;
	sw_short_int *local_buff = NULL;

	sw_tls *write_tls;
	write_tls=get_user_tls();

	FILE *file_pointers;
	file_pointers=((sw_tls_file *)write_tls->file_data)->file_lib_pointers;
	if(!file_pointers) {
		sw_printf("Error in fwrite\n");
		errno= ENOMEM;
		return (-1);
	}

	if((file_pointers[file].fd == SW_STDERR) || 
			(file_pointers[file].fd == SW_STDOUT) || 
			(file_pointers[file].fd == SW_STDIN)) {
		if((local_buff = (sw_short_int*)sw_malloc((len+1)*sizeof(sw_short_int)))
				!= NULL) {
			sw_memcpy(local_buff,ptr,len);
			local_buff[len] = '\0';
			sw_printf((char *)local_buff);
			sw_free(local_buff);
		}
		todo = len;
	} else {
		sw_printf("file system configuration is not defined. \
				Enable CONFIG_FILESYSTEM_SUPPORT.\n");

		errno= EBADF;
		return (-1);
	}
	errno = 0;
	return(todo);
}

/**
 * @brief the command does nothing beyond expanding arguments
 *              and performing  any  specified redirections
 *
 * @param val file descriptor of the file
 *
 * @return  A zero exit code is returned 
 */
void _exit(int val)
{
}

/**
 * @brief executes  the program pointed to by filename 
 *
 * @param name file name of the file
 * @param argv is an array of argument strings passed to the new program
 * @param envs an array of strings, conven‐tionally of the form
 *			key=value, which are passed as
 *			environment to the new program
 *
 * @return On success, _execve() does not return, on error -1 is returned,
 *			and errno is set appropriately 
 */
int _execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}

/**
 * @brief returns the process ID of the calling process 
 *
 * @return returns the process ID of the parent of the calling process 
 */
int _getpid()
{
	return __MYPID;
}


/**
 * @brief creates a new process by duplicating the calling process 
 *
 * @return On success, the PID of the child process is returned in the parent,
 *			and 0 is returned in  the  child.On  failure,  -1  is  returned
 *			in the parent, no child process is created, and errno is set
 *			appropriately 
 */
int _fork(void)
{
	errno = ENOTSUP;
	return -1;
}

/**
 * @brief terminate a process.Sends the specified signal to
 *			the specified process or process group 
 *
 * @param pid Specify the list of processes that kill should signal
 * @param sig Specify the signal to send.  The signal may be given
 *				as a signal name or number
 *
 * @return If sig is 0, then no signal is sent,
 *			but error checking is still performed 
 */
int _kill(int pid, int sig)
{
	if(pid == __MYPID)
		_exit(sig); 
	errno = EINVAL;
	return -1;
}

/**
 * @brief The parent process may then issue a wait system call,
 *			which suspends the execution of the parent process
 *				while the child executes 
 *
 * @param status status of the process
 *
 * @return On success, _wait() does not return, on error -1 is returned,
 *          and errno is set appropriately
 */
int _wait(int *status)
{ 
	errno = ECHILD;
	return -1;
}

/**
 * @brief call the link function to create a link to a file 
 *
 * @param old old existing file
 * @param new New file to be link with old one
 *
 * @return On success, _wait() does not return, on error -1 is returned,
 *          and errno is set appropriately
 */
int _link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

/**
 * @brief These functions return information about a file 
 *
 * @param file file descriptor of the file
 * @param st file pointed to by path and fills in st structure
 *
 * @return On success, zero is returned.  On error, -1 is returned,
 *          and errno is set appropriately
 */
int _stat(const char* file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

/**
 * @brief  call the unlink function to remove the specified file
 *
 * @param name name of the file to be unlink
 *
 * @return On success,return 0, on error -1 is returned,
 *          and errno is set appropriately
 */
int _unlink(char *name)
{
	sw_printf("file system configuration is not defined. \
			Enable CONFIG_FILESYSTEM_SUPPORT.\n");

	errno= EBADF;
	return (-1);
}

/**
 * @brief giving timing statistics about this program run 
 *
 * @param buf timezone structure contains information of the program
 *					runtime in term of seconds and microseconds
 *
 * @return On success, _times() does not return, on error -1 is returned,
 *          and errno is set appropriately
 *
 */
clock_t _times(struct tms *buf)
{
	errno = ENOTSUP;
	return -1;
}

/**
 * @brief get time of the day
 *
 * @param buf timezone structure contains information of the program
 *					runtime in term of seconds and microseconds
 * @param buf2  corresponding timezone structure contains information of
 *				the program runtime in term of minutes west of Greenwich
 *				with type of DST correction
 *
 * @return  On success, _times() does not return, on error -1 is returned,
 *          and errno is set appropriately 
 */
int _gettimeofday(struct timeval *buf, void *buf2)
{
	sw_timeval *tmp;
	int t;
	
	tmp = malloc(sizeof(sw_timeval));
	tmp->tval.sec = buf->tv_sec;
	tmp->tval.nsec = buf->tv_usec;
	t =__sw_gettimeofday(tmp,buf2);
	buf->tv_sec = tmp->tval.sec;
	buf->tv_usec = tmp->tval.nsec;
	return t;
}

/**
 * @brief finds the resolution (precision) of the specified clock clk_id
 *
 * @param clk_id structue contains id of specific clock
 * @param tp structue  contains information of the program
 *                  runtime in term of seconds and nanoseconds 
 *
 * @return  On success, clock_gettime() does not return, on error -1 is
 *			returned and errno is set appropriately 
 *
 */
int _clock_gettime(clockid_t clk_id, struct timeval *tv)
{

	errno = ENOTSUP;
	return(-1); 
}

/**
 * @brief Create the DIRECTORY 
 *
 * @param name name of the DIRECTORY to be created
 * @param mode set file mode (as in chmod)
 *
 * @return  On success, _mkdir() does not return, on error -1 is
 *          returned and errno is set appropriately
 */
int _mkdir(const char*name, int mode)
{
	errno = ENOTSUP;
	return -1;
}

/**
 * @brief rename the specified files by replacing the
 *			first occurrence of from in their name by to
 *
 * @param oldname old name of the renamed file
 * @param newname newname to be renamed
 *
 * @return On success return 0, on error -1 is
 *          returned and errno is set appropriately
 */
int _rename(const char*oldname, const char*newname)
{
	errno = ENOTSUP;
	return -1;
}

/**
 * @brief performs one of the operations described below on the open
 *			file descriptor fd
 *
 * @param fd	file descriptor of the file
 * @param cmd  	operation performed based on passing cmd
 *
 * @return On success, _fcntl() does not return, on error -1 is
 *          returned and errno is set appropriately 
 */
int _fcntl(int fd, int cmd)
{
	errno = ENOTSUP;
	return -1;
}

/**
 * @brief executes a command
 *
 * @param cmd execution performed based on passing cmd
 *
 * @return  On success, _system() does not return, on error -1 is
 *          returned and errno is set appropriately  
 */
int _system(const char*cmd)
{
	errno = ENOTSUP;
	return -1;
}

/**
 * @brief  examine and change blocked signals 
 *
 * @param how	in which the set is changed, and
 *				consists of one of the following values
 *				SIG_BLOCK,SIG_SETMASK and SIG_UNBLOCK
 * @param set	If the argument set is not a null pointer, it points
 *				to a set of signals to be used to change the currently
 *				blocked set	
 * @param oldset If the argument oset is not a null pointer, the
 *			previous mask is stored in the location pointed to by oset
 *
 * @return Upon successful completion, sigprocmask() returns 0.
 *			Otherwise -1 is returned, errno is set to indicate 
 *			the error and the process' signal mask will be unchanged
 */
int sigprocmask( int how, const sigset_t *set, sigset_t *oldset)
{
	errno = ENOTSUP;
	return -1;
}

#undef sigfillset
/**
 * @brief initialises the signal set pointed to by set,
 *			such that all signals defined in this document are included. 
 *
 * @param set initialise and fill a signal set 
 *
 * @return Upon successful completion, sigfillset() returns 0.
 * 			Otherwise, it returns -1 and sets errno to indicate the error 
 */
int sigfillset(sigset_t *set)
{
	errno = ENOTSUP;
	return -1;
}

/**
 * @brief initializes the filelib_init for the NEWLIB_SUPPORT
 *
 * @param tls_lib_addr Configuration parameter for the NEWLIB_SUPPORT 
 *
 * @return SW_OK on success, Otherwise, it returns -1 and and
 *				sets errno to indicate the error
 */
int filelib_init(sw_tls *tls_lib_addr) {
    if(tls_lib_addr == NULL) {
        sw_printf("Error in _newlib_init\n");
        errno= EBADF;
        return (-1);
    }

    tls_lib_addr->file_data=sw_malloc(sizeof(sw_tls_file));
    if(tls_lib_addr->file_data == NULL) {
        sw_printf("Error in _newlib_init\n");
        errno= ENOMEM;
        return (-1);
    }

    ((sw_tls_file *)tls_lib_addr->file_data)->file_lib_pointers =
        sw_malloc(MAX_OPEN_FILES*sizeof(FILE));
    if(((sw_tls_file *)tls_lib_addr->file_data)->file_lib_pointers == NULL) {
        sw_free(tls_lib_addr->file_data);
        sw_printf("Error in _newlib_init\n");
        errno= ENOMEM;
        return (-1);
    }

	sw_memset(((sw_tls_file *)tls_lib_addr->file_data)->file_lib_pointers,
            						0x0,MAX_OPEN_FILES*sizeof(FILE));
    return SW_OK;
}

/**
 * @brief function gets called before the task deletion
 * 			and frees the allocated memory for data
 *
 * @param tls_lib_addr tls data which need to be freed
 *
 * @return SW_OK on success, Otherwise, it returns -1 and and
 *              sets errno to indicate the error 
 */
int filelib_exit(sw_tls *tls_lib_addr) {
	if(tls_lib_addr == NULL) {
		sw_printf("_newlib_exit\n");
		errno= EBADF;
		return (-1);
	}

	if(((sw_tls_file *)tls_lib_addr->file_data)->file_lib_pointers)
		sw_free(((sw_tls_file *)tls_lib_addr->file_data)->file_lib_pointers);

	if(tls_lib_addr->file_data)
		sw_free(tls_lib_addr->file_data);

	return SW_OK;
}

/**
 * @brief Upstart process management daemon
 *			the  parent of all processes on the system,
 * 			it is executed by the kernel and is responsible
 *      	for starting all other processes
 */
void _init()
{
}

/**
 * @brief responsible for finish all other processes 
 */
void _fini()
{
}
/** @} */ // end of OS_UserSyscalls 
