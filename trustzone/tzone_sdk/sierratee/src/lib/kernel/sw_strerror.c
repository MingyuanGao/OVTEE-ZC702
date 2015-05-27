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

#include <sw_errno.h>

struct errno_str_struct
{
	int errnum;
	const char *str;
};

static const struct errno_str_struct errno[10] =
{
/*	{ SW_EFAIL,					SW_EFAIL_STR			},
	{ SW_ENOMEM,				SW_ENOMEM_STR			},
	{ SW_ILLEGAL_ARGUMENT,		SW_ILLEGAL_ARGUMENT_STR	},
	{ SW_EINVALID,				SW_EINVALID_STR			}*/
};

const char *sw_str_for_errno(int errnum)
{
	int no_of_errstr = (sizeof(errno) / sizeof(struct errno_str_struct));
	int low = 0;
	int hi  = no_of_errstr - 1;
	int mid;

	do
	{
		mid = (low + hi) >> 1;
		if (errnum > errno[mid].errnum)
		{
			low = mid + 1;
		}
		else if (errnum < errno[mid].errnum)
		{
			hi = mid - 1;
		}
		else
		{
			return errno[mid].str;
		}
	}
	while (low <= hi);
	return "Unknown error";
}
