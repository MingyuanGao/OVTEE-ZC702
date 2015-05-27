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
 * memory management implementation
 */

#include <sw_buddy.h>
#include <sw_types.h>
#include <sw_debug.h>
#include <libc_sos.h>
#include <sw_mem_functions.h>
#include <otz_tee_mem_mgmt.h>
#include <otz_tee_common.h>
#include <sw_types.h>

/** @ingroup GP_IntAPI GP Internal API
 *  GP Internal API for Sierra OS User tasks
 *  @{
 */
/*
   static void *Data;

   TEE_Result TEE_CheckMemoryAccessRights(	sw_uint accessFlags,
   								void* buffer, size_t size) {
   return TEE_SUCCESS;
   }

   void TEE_SetInstanceData( void* instanceData ) {
   Data = instanceData;
   }

   void* TEE_GetInstanceData( void ) {

   return Data;
   }
   */
void* TEE_Malloc( size_t size, sw_uint hint ) {
	void* buffer;
	if((buffer = sw_malloc((sw_uint)size))==NULL) {
		sw_printf("Memory Allocation Failed\n");
		return NULL;
	}

	if(hint == 0x0) {
		sw_memset(buffer, 0, size);
	}
	/*else {
	  hint in the range [0x00000001, 0xFFFFFFFF] can be used in future versions.
	  }*/
	return buffer;
}

/**
 * @brief 
 *
 * @param buffer
 * @param newSize
 *
 * @return 
 */
void* TEE_Realloc( void* buffer, sw_uint newSize ) {
	return(sw_realloc(buffer,newSize));
}

/**
 * @brief 
 *
 * @param buffer
 */
void TEE_Free( void *buffer ) {
	sw_free(buffer);
}

/**
 * @brief 
 *
 * @param dest
 * @param src
 * @param count
 */
void  TEE_MemMove( void* dest, void* src, sw_uint count ) {
	sw_memmove(dest, src, count);
}

/**
 * @brief 
 *
 * @param src
 * @param dest
 * @param length
 *
 * @return 
 */
int TEE_MemCompare( void* src, void* dest, sw_uint length) {
	return(sw_memcmp(src, dest, length));
}

/**
 * @brief 
 *
 * @param buffer
 * @param x
 * @param size
 */
void  TEE_MemFill( void* buffer, sw_uint x, sw_uint size) {
	sw_memset(buffer, x, size);
}

/**
 * @brief 
 *
 * @param dest
 * @param src
 * @param count
 */
void  TEE_MemCpy( void* dest, void* src, sw_uint count ) {
	sw_memcpy(dest, src, count);
}

/**
 * @brief 
 *
 * @param dest
 * @param src
 * @param count
 * @param dstLen
 * @param srcLen
 */
void  TEE_MemnCpy(void* dest, void* src, sw_uint count,
		sw_uint dstLen,sw_uint srcLen) {
	sw_memncpy(dest, src, count,dstLen,srcLen);
}
/** @} */ // end of GP_IntAPI 
