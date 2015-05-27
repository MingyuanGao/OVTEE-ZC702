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
 *  Division routines
 */

#include "sw_debug.h"
#include "sw_types.h"
#include <sw_math.h>

/**
 * @brief 
 * Unsigned int division.
 *
 * @param dividend
 * @param divisor
 *
 * @return 
 */
#if !defined(NEWLIB_SUPPORT) || defined(SW_KERNEL)
sw_uint __aeabi_uidiv(sw_uint dividend, sw_uint divisor)
{
   	
   	sw_uint tempdivisor, quotient = 1;

   	if (divisor == 0)
		tee_panic("Divide by 0");
	if (divisor == dividend)
		return 1;
	else if (dividend < divisor)
		return 0;
	if (is_pow_of_2(divisor))
	{
		return dividend >> log_of_2(divisor);
	}
	tempdivisor=divisor;
	while (divisor<<1 <= dividend) {
		divisor = divisor << 1;
		quotient = quotient << 1;
	} 
	quotient = quotient + __aeabi_uidiv(dividend-divisor,tempdivisor);
	return quotient;
}
#endif

typedef struct
{
	sw_big_ulong quot;
	sw_big_ulong rem;
} uldivmod_t;

/**
 * @brief 
 * 64-bit unsigned int division
 *
 * @param dividend
 * @param divisor
 *
 * @return 
 */
#if !defined(NEWLIB_SUPPORT) || defined(SW_KERNEL) 
uldivmod_t __aeabi_uldivmod(sw_big_ulong dividend, sw_big_ulong divisor)
{
	uldivmod_t ret,temp;
	unsigned long long tempdivisor;
	ret.quot = 1;
	if (divisor == 0)
		tee_panic("Divide by 0");
	if (divisor == dividend) {
		ret.quot = 1;
		ret.rem = 0;
		return ret;
	}
	else if (dividend < divisor) {
			ret.quot = 0;
			ret.rem = dividend;
			return ret;
	}
	if(is_pow_of_2(divisor)) {
		ret.quot = dividend >> log_of_2(divisor);
		ret.rem = (dividend & (divisor - 1));
		return ret;
	}
	tempdivisor=divisor;
	while (divisor<<1 <= dividend) {
		divisor = divisor << 1;
		ret.quot = ret.quot <<1;
	} 
	temp = __aeabi_uldivmod(dividend-divisor  , tempdivisor);
	ret.quot+=temp.quot;
	ret.rem = temp.rem;
	return ret;
	
}
#endif

/**
 * @brief Signed modulo division
 *
 * @param dividend
 * @param divisor
 *
 * @return 
 */
 
#if !defined(NEWLIB_SUPPORT) || defined(SW_KERNEL)
sw_uint __aeabi_idivmod(sw_uint dividend, sw_uint divisor) {
	sw_int remainder=0, sign = 1;
	sw_uint udividend = (dividend < 0) ? -dividend : dividend;
	sw_uint udivisor = (divisor < 0) ? -divisor : divisor;
	
	if (divisor == 0)
		tee_panic("Divide by 0");
	if ((dividend>0 &&divisor<0)||(dividend<0 && divisor>0))
		sign = -1;
	if (udivisor == udividend) {
		remainder = 0;
		return remainder;
	}
	else if (udividend < udivisor) {
		if(dividend < 0) {
			remainder = udividend*sign;
			return remainder;
		}
		else {
			remainder = udividend;
			return remainder;
		}
	}
	if(is_pow_of_2(divisor)) {
		return(udividend & (udivisor - 1))*sign;
	}
	while (udivisor<<1 <= udividend) {
		udivisor = udivisor << 1;
	} 
	if(dividend < 0)
		remainder = __aeabi_idivmod(-(udividend-udivisor)  , divisor);
	else
		remainder = __aeabi_idivmod(udividend-udivisor  , divisor);
	return remainder;
}
#endif
