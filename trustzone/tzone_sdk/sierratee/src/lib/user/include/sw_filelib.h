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
 * This Header file for libc functions that needed by storage-api apps 
 */
#ifndef _SW_FILELIB_H
#define _SW_FILELIB_H

#include "sw_types.h"
#include "tls.h"

#define EOF                 (-1)
#define SW_FILE_INDEX        3
#define SW_MAX_OPEN         20
#define SW_SIZE_BUF         256

#define SEEK_SET    0    /* Seek from beginning of file.  */
#define SEEK_CUR    1    /* Seek from current position.  */
#define SEEK_END    2    /* Seek from end of file.  */


/**
 * @brief sw_file_flag_Constants
 */
enum sw_file_flag_constants {
	SW_READ_FLAG          =	0x00000001,
	SW_WRITE_FLAG         =	0x00000002,
	SW_CREATE_NEW_FLAG    = 0x00000004,
	SW_CREATE_ALWAYS_FLAG = 0x00000008,
	SW_APPEND_FLAG        = 0x00000010,
	SW_EOF_FLAG           =	0x00000020,
	SW_ERR_FLAG	          =	0x00000040,
};

/**
 * @brief sw_file_cont
 */
struct sw_file_cont {
	int	sw_fdesp;
	int	sw_var_cnt;
	int sw_buf_size;
	int	sw_flags;
	unsigned char *sw_buf_ptr;
	unsigned char *sw_var_buf;
};

#ifndef _STDIO_H_
/**
 * @brief 
 */
typedef struct sw_file_cont FILE;

/**
 * @brief 
 *
 * @param tls_file_addr Intializes and allocates memory for
 * 	FILE structure upto SW_MAX_OPEN.Using tls->file_data
 *	reference pointer points the corresponding
 *	newlib enable/disable FILE structure
 *
 * @return 
 */
int filelib_init(sw_tls *tls_file_addr);

/**
 * @brief 
 * fopen-function shall open the file whose pathname is the string pointed to by
 *      filename, and associates a stream with it
 *
 * @param name-string containing the name of the file to be opened
 * @param mode-string containing a file access mode
 *
 * @return -Upon successful completion, fopen() shall return a pointer to the 
 *         object controlling the stream. Otherwise, a null pointer shall be 
 *         returned
 */
FILE *fopen(const unsigned char *name,const unsigned char *mode);

/**
 * @brief 
 * fread - Reads an array of count elements from an inputstream 
 *
 * @param outbuf - Pointer to a block of memory with a size of at least 
 *                 (size*count) bytes
 * @param size -   Size in bytes, of each element to be read
 * @param count -  Number of elements, each one with a size of size bytes
 * @param instream-Pointer to a FILE object that specifies an input stream
 *
 * @return -       The total number of elements successfully read is returned
 *                 if this number differs from the count parameter, either a 
 *                 reading error occurred or the end-of-file was reached while 
 *                 reading. In both cases, -1 is returned
 */
size_t fread(void *out_buf,size_t size,size_t count,FILE *inputstream);

/**
 * @brief 
 * fwrite - Writes an array of count elements to an outputstream
 *
 * @param input_buf  - Pointer to the array of elements to be written
 * @param size       - Size in bytes of each element to be written
 * @param num_items  - Number of elements, each one with a size of size bytes
 * @param out_stream - Pointer to a FILE object that specifies an output stream
 *
 * @return - The total number of elements successfully written is returned.
 *           If this number differs from the count parameter -1 is returned
 */
size_t fwrite(const void *input_buf, size_t size, size_t num_items,
		FILE* out_stream);

/**
 * @brief 
 * fprintf - Writes data in a specified format into the file 
 *
 * @param out_stream - Pointer to a FILE object that specifies an output stream
 * @param data_fmt   - Specifies format of data to be written
 * @param ...        - Variable number of arguments
 *
 * @return - On success returns the number of characters written otherwise EOF
 * is returned
 */
int fprintf(FILE *out_stream, const char *data_fmt, ...);

/**
 * @brief
 * fseek - Sets the position indicator associated with the stream to a new 
 *        position 
 *
 * @param seekstream - Pointer to a FILE object that identifies the stream
 * @param offset - Number of bytes to offset from origin
 * @param whence - Position used as reference for the offset for reference 
 *                sw_filelibc.h
 *
 * @return - If successful, the function returns zero.
 *          Otherwise, it returns non-zero value
 */
int fseek(FILE *seekstream, long offset, int whence);

/**
 * @brief 
 * feof - Checks whether the end-of-File indicator associated with stream is set
 *
 * @param file_stream - Pointer to a FILE object that identifies the stream 
 *
 * @return - A non-zero value is returned in the case that the end-of-file 
 *           indicator associated with the stream is set.Otherwise, zero is 
 *          returned
 */
int feof(FILE *file_stream);

/**
 * @brief 
 * fclose  - function  will  flushes  the stream pointed to by filepointer 
 *
 * @param file_stream - Pointer to a FILE object that specifies the
 * 						stream to be closed
 *
 * @return - If the stream is successfully closed, a zero value is returned.
 *           On failure, EOF is returned
 */
int fclose(FILE* file_stream);

/**
 * @brief 
 * remove - Deletes the file whose name is specified in filename
 *
 * @param r_pth - string containing the name of the file to be deleted
 *
 * @return - if the file is successfully deleted, a zero value is returned.
 *           On failure, a non-zero value is returned
 */
int remove(const char *r_pth);

/**
 * @brief 
 *	fgets - reads  the  next character from stream
 *
 * @param dest_buf - read bytes from stream into the array pointed to by s,
 *					 until n-1 bytes are read
 * @param input_cnt - Input count to read
 * @param source_stream - Input FILE stream contains data in terms of bytes
 *
 * @return -	returns it as an unsigned char cast to an int, or
 * 				EOF on end of file or error
 */
char *fgets(char *dest_buf, int input_cnt, FILE *source_stream);

/**
 * @brief 
 *
 * @param tls_file_addr free the allocates memory for
 *  FILE structure upto SW_MAX_OPEN.Using tls->file_data
 *  reference pointer points the corresponding
 *  newlib enable/disable FILE structure
 *
 * @return 
 */
int filelib_exit(sw_tls *tls_file_addr);

#endif
#endif
