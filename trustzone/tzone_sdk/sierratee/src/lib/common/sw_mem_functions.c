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
 * Memory management functions
 */

#include <sw_debug.h>
#include <sw_mem_functions.h>

/** @ingroup OS_CommonApi Common API for Secure OS
 *  API for Sierra OS User and Kernel tasks
 *  @{
 */
/**
 * @brief 
 *
 * memory move
 *
 * @param dest
 * @param src
 * @param count
 *
 * @return 
 */
void * sw_memmove(void * dest,const void *src, sw_uint count)
{
	char *tmp, *s;

	if (dest <= src)
	{
		tmp = (char *) dest;
		s = (char *) src;
		while (count--)
		{
			*tmp++ = *s++;
		}
	}
	else
	{
		tmp = (char *) dest + count;
		s = (char *) src + count;
		while (count--)
		{
			*--tmp = *--s;
		}
	}
	return dest;
}

/**
 * @brief 
 * memory set
 *
 * @param dest
 * @param c
 * @param count
 *
 * @return 
 */
void * sw_memset(void * dest, sw_uint c, sw_uint count)
{
	/* Standard bytewise memset */
	char* d;
	d = (char*) dest;

	while(count--)
	{
		*d = c;
		d++;
	}

	return dest;
}


/**
 * @brief 
 * This version of memcpy assumes disjoint ptrs src, dst 
 *
 * @param dst
 * @param src
 * @param count
 *
 * @return 
 */
void *sw_memcpy(void *dst, const void *src, sw_uint count)
{
	int i;
	char *dst_tmp = dst;
	const char *src_tmp = src;

	if (!((unsigned int)src & 0x3) && !((unsigned int)dst & 0x3))
	{
		/* Word aligned so we can safely do word copies */
		for (i=0; i < count; i+=4)
		{
			if (i + 3 > count - 1)
				break; /* Don't copy too much */

			*(sw_uint *)dst_tmp = *(sw_uint *)src_tmp;
			dst_tmp += 4;
			src_tmp += 4;
		}
		if (i <= count - 1)
		{
			for (; i < count; i++)
			{
				*dst_tmp = *src_tmp;
				dst_tmp++;
				src_tmp++;
			}
		}
	}
	else
	{
		/* Generic version */
		for (i=0; i < count; i++)
			dst_tmp[i] = src_tmp[i];
	}
	return dst;
}


/**
 * @brief custom version of memcmp function
 */
int sw_memcmp(void *src, void *dest, sw_uint length)
{
	sw_uint counter = -1;
	sw_short_int *local_src = (sw_short_int*)src, *local_dest = (sw_short_int*)dest;
	while((++counter <length) && (local_src[counter] - local_dest[counter] == 0));
	if(counter == (int)length) {
		return(local_src[counter-1] - local_dest[counter-1]);
	}
	return(local_src[counter] - local_dest[counter]);
}



/**
 * @brief
 * Copies the values of Num bytes from the location pointed by source directly 
 * to the memory block pointed by destination By checking the values of 
 * Num bytes with destination and source Maximum Length
 *
 * @param dst - Pointer to the destination array where the content is to be
 *  			   copied
 * @param src - Pointer to the source of data to be copied
 * @param count - Number of bytes to copy
 * @param dst_len - destination Maximum Length
 * @param src_len - source Maximum Length
 *
 * @return 
 */
void *sw_memncpy(void *dst, const void *src, sw_uint count,sw_uint dst_len,sw_uint src_len)
{
	if(count<=dst_len || count<=src_len) {
		return sw_memcpy(dst,src,count);		
	}
	else {
#ifdef DEBUG_MEMORY_FUNCTIONS
		sw_printf("Memory Overflow Error\n");
#endif
		return NULL;
	}
}
/** @} */ // end of OS_CommonApi
