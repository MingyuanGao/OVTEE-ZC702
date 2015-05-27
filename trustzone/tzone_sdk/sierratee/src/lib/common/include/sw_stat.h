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
 * Header file for stat,these functions return information about a file
 */

#ifndef __SW_STAT_H__
#define __SW_STAT_H__

/**
 * @brief 
 * stat_time_date - Structure contains values which indiates time and date
 */
struct stat_time_date {
	long int sw_time_value;
	long int sw_date_value;
};

/**
 * @brief
 */
typedef struct stat_time_date sw_time_date;

/**
 * @brief 
 * sw_stat - Structure contains information about files
 */
struct sw_stat {
	unsigned int sw_file_mode;		/* mode of file, protection bits, etc. */
	long int sw_file_size;			/* size of file */
	sw_time_date sw_access_time;	/* last access time */
	sw_time_date sw_mod_time;		/* last modification time*/
	sw_time_date sw_crt_time;		/* last file status change time */
	long int sw_block_cnt;    	    /* number of 512B blocks allocated */
};

#define SW_IFMT  		0170000		/* type of file */
#define SW_IFREG  		0100000		/* regular */
#define SW_IFDIR 		0040000		/* directory */
#define SW_IFCHR 		0020000		/* character special */

#define SW_ISREG(modes) 	(((modes) & SW_IFMT) == SW_IFREG)
#define SW_ISDIR(modes)     (((modes) & SW_IFMT) == SW_IFDIR)  
#define SW_ISCHR(modes)     (((modes) & SW_IFMT) == SW_IFCHR)

#endif
