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

#ifndef __SW_ERRNO_H__
#define __SW_ERRNO_H__
/*
 * Error number values and strings are defined on this file.
 */

/**
 * @brief Return the error string for the given error number.
 *
 * @param errnum: Error number
 *
 * @return : 
 * Error string value
 */
const char *sw_str_for_errno(int errnum);

/**
 * @brief Set error number either in kernel or task context.
 *
 * @param errno: Error number
 */
void sw_seterrno(int errno);


/**
 * @brief Get error number based on the execution context 
 *
 * @return: Error number
 */
int sw_geterrno();

/* ERROR CODES */

#define SW_ERROR			   -1
#define SW_OK            	   0

#define SW_EPERM               1
#define SW_EPERM_STR           "Operation not permitted"
#define SW_ENOENT              2
#define SW_ENOENT_STR          "No such file or directory"
#define SW_ESRCH               3
#define SW_ESRCH_STR           "No such process"
#define SW_EIO                 5
#define SW_EIO_STR             "I/O error"
#define SW_ENXIO               6
#define SW_ENXIO_STR           "No such device or address"
#define SW_EBADF               9
#define SW_EBADF_STR           "Bad file number"
#define SW_EAGAIN              11
#define SW_EAGAIN_STR          "Try again"
#define SW_ENOMEM              12
#define SW_ENOMEM_STR          "Out of memory"
#define SW_EACCES              13
#define SW_EACCES_STR          "Permission denied"
#define SW_EFAULT              14
#define SW_EFAULT_STR          "Bad address"
#define SW_EBUSY               16
#define SW_EBUSY_STR           "Device or resource busy"
#define SW_EEXIST              17
#define SW_EEXIST_STR          "File exists"
#define SW_EINVAL              22
#define SW_EINVAL_STR          "Invalid argument"
#define SW_ENOSPC              28
#define SW_ENOSPC_STR          "No space left on device"
#define SW_EROFS               30
#define SW_EROFS_STR           "Read-only file system"
#define SW_ENOSYS              38
#define SW_ENOSYS_STR          "Function not implemented"

#define SW_EBADRQC             56
#define SW_EBADRQC_STR         "Invalid request code"
#define SW_ENODATA             61
#define SW_ENODATA_STR         "No data available"
#define SW_ETIME               62
#define SW_ETIME_STR           "Timer expired"
#define SW_EALREADY            114
#define SW_EALREADY_STR        "Operation already in progress"
#define SW_EINPROGRESS         115
#define SW_EINPROGRESS_STR     "Operation now in progress"

#endif
