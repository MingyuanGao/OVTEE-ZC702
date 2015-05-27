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
#include <sw_addr_config.h>
#include <sw_syscall.h>
void sw_seterrno(sw_int errno)
{
	sw_tls *tls;
	tls = __sw_get_tls();
	tls->task_errno = errno;
	__sw_set_tls((void*)tls);

}

sw_int  sw_geterrno(void)
{
	sw_tls *tls;
	tls = __sw_get_tls();
	return tls->task_errno;

}
