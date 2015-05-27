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
 * This Header file for NEWLIB_SUPPORT functions that needed by apps 
 */
#define FILE_READ                0x01
#define FILE_WRITE               0X02
#define FILE_CREATE_NEW          0x04
#define FILE_CREATE_ALWAYS       0x08
#define FILE_APPEND              0x10  


#define   FILE_CNTR_START_INDEX   3
#define   MAX_OPEN_FILES   		 (32+FILE_CNTR_START_INDEX)
#define   MAX_FILENAME_LENGTH     255
#define	  __MYPID 				  1

/**
 * @brief NEWLIB_SUPPORT FILE structure
 * 		for maintain file descriptor value
 *		and FILE_OPEN index no information
 */
typedef struct
{
	int fd;
	unsigned char *filename;
}FILE;

/**
 * @brief initializes the filelib_init for the NEWLIB_SUPPORT
 *
 * @param tls_lib_addr Configuration parameter for the NEWLIB_SUPPORT 
 *
 * @return SW_OK on success, Otherwise, it returns -1 and and
 *              sets errno to indicate the error
 */
int filelib_init(sw_tls *tls_lib_addr);

/**
 * @brief function gets called before the task deletion
 *          and frees the allocated memory for data
 *
 * @param tls_lib_addr tls data which need to be freed
 *
 * @return SW_OK on success, Otherwise, it returns -1 and and
 *              sets errno to indicate the error 
 */
int filelib_exit(sw_tls *tls_lib_addr);
