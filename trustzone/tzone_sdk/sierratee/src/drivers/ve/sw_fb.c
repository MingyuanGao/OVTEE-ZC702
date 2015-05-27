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
 * Secure Frame_base buffer info initialization
 */

#include <sw_board.h>
#include <sw_fb.h>
#include <sw_debug.h>
#include <sw_mem_functions.h>
#include <fb.h>

#ifdef CONFIG_CACHE_L2X0
#include <sw_l2_pl310.h>
#endif


static sw_int fb_open = 0;

static struct sw_fb_info g_fb_info;

/**
* @brief 
*
* @param clcd_base
* @param width
* @param height
* @param frame_base
*/
static void init_pl111( sw_uint clcd_base,
		sw_int           width,
		sw_int           height,
		sw_uint			     frame_base )
{
	volatile sw_uint*  clcd  = (volatile sw_uint*) clcd_base;

	/* PL111 register offsets (32-bit words) */
	const sw_int PL111_TIM0 = (0x00/4);
	const sw_int PL111_TIM1 = (0x04/4);
	const sw_int PL111_TIM2 = (0x08/4);
	const sw_int PL111_TIM3 = (0x0C/4);
	const sw_int PL111_UBAS = (0x10/4);
	const sw_int PL111_LBAS = (0x14/4);
	const sw_int PL111_CNTL = (0x18/4);
	const sw_int PL111_IENB = (0x1C/4);

	/*	Timing number for an 8.4" LCD screen for use on a VGA screen */
	sw_uint TIM0_VAL = 
		( (((width/16)-1)<<2) | (63<<8) | (31<<16) | (63<<8) );
	sw_uint TIM1_VAL = ( (height - 1) | (24<<10) | (11<<16) | (9<<24) );
	sw_uint TIM2_VAL = ( (0x7<<11) | ((width - 1)<<16) | (1<<26) );

	/* Statically allocate memory for screen buffer */
	g_fb_info.fb_buffer = (sw_uint*) frame_base;

	/* Program the CLCD controller registers and start the CLCD */
	clcd[ PL111_TIM0 ] = TIM0_VAL;
	clcd[ PL111_TIM1 ] = TIM1_VAL;
	clcd[ PL111_TIM2 ] = TIM2_VAL;
	clcd[ PL111_TIM3 ] = 0; 
	clcd[ PL111_UBAS ] = VE_FRAME_BASE_PA;
	clcd[ PL111_LBAS ] = 0;
	clcd[ PL111_IENB ] = 0;

	/* Set the control register: 16BPP, Power OFF */
	if(g_fb_info.format == RGB565_FORMAT) {
		/* RGB 565 mode */
		clcd[ PL111_CNTL ] = (1<<0) | (4<<0) | (1<<5) | (0<<8) | (4 << 1);
	}
	else {
		/* RGB 444 mode */
		clcd[ PL111_CNTL ] = (1<<0) | (1<<5) | (4<<1);
	}

	/* Power ON */
	clcd[ PL111_CNTL ] |= (1<<11);
	fb_open = 1;
}


/**
* @brief 
*
* @return 
*/
static sw_int init_lcd(void)
{
	const sw_uint dvi_mux = 0;

	/* VE System Register 32-bit word offsets */
/*	const sw_int VE_SYS_ID       = (0x00/4); */
	const sw_int VE_SYS_CFG_DATA = (0xA0/4);
	const sw_int VE_SYS_CFG_CTRL = (0xA4/4);
/*	const sw_int VE_SYS_ID_HBI = 0x225; */

	volatile sw_uint*  ve_sysreg = (volatile sw_uint*) VE_SYSTEM_REGS;

	/* Set CLCD clock
	 *   SYS_CFG_DATA sets oscillator rate value as 5.4MHz
	 *   SYS_CFG_CTRL
	 *      ( start=1 | write=1 | function=1 | site=0 | position=0 | device=1 )
	 */
	ve_sysreg[ VE_SYS_CFG_DATA ] = 5400000;
	ve_sysreg[ VE_SYS_CFG_CTRL ] = 0x80000000 | 
		(1<<30) | (1<<20) | (0<<16) | (0<<12) | (1<<0);

	/* 	Set DVI mux for correct MMB
		SYS_CFG_CTRL ( start=1 | write=1 | 
		function=7 | site=0 | position=0 | device=0 )
		*/
	ve_sysreg[ VE_SYS_CFG_DATA ] = dvi_mux;
	ve_sysreg[ VE_SYS_CFG_CTRL ] = 0x80000000 | 
		(1<<30) | (7<<20) | (0<<16) | (0<<12) | (0<<0);

	init_pl111(VE_CLCD_BASE, g_fb_info.xres, g_fb_info.yres, VE_FRAME_BASE);
	return SW_OK;
}

/**
* @brief 
*/
static void set_fb_default_params(void)
{
	g_fb_info.xres = 1024;
	g_fb_info.yres = 600;
	g_fb_info.bpp = 16;
	g_fb_info.format = RGB565_FORMAT;						
	g_fb_info.fb_buffer = NULL;
}

/**
* @brief 
*/
static void reset_fb_default_params(void)
{
	g_fb_info.xres = 0;
	g_fb_info.yres = 0;
	g_fb_info.bpp = 0;
	g_fb_info.format = 0;		
	g_fb_info.fb_buffer = NULL;					
}

/**
* @brief 
*
* @param offset
* @param color
*/
static void sw_fb_write_pixel(sw_int offset, sw_uint color)
{
	if(offset <= (g_fb_info.yres * g_fb_info.xres + 
				g_fb_info.xres) && (offset >= 0) && (fb_open == 1)) {
		sw_ushort RGB565=((((color >> 16) & 0xF8) << 8) | 
				(((color >> 8) & 0xFC) << 3) | 
				((color & 0xF8) >> 3));

		sw_ushort *tmp = (sw_ushort*)g_fb_info.fb_buffer;
		tmp[offset] = RGB565;
	}
}

/**
* @brief 
*
* @param data
* @param img_size
*/
static void sw_fb_write_window(void *data, sw_int img_size)
{
    sw_memcpy(g_fb_info.fb_buffer, data, img_size);
}

/**
* @brief 
*/
static void sw_flush_fb_cache()
{
#ifdef TOUCH_SUPPORT
	flush_icache_and_dcache();
#ifdef CONFIG_CACHE_L2X0
	sw_flush_l2cache_multi_line(VE_FRAME_BASE, VE_FRAME_SIZE);
#endif
#endif
}

/**
* @brief 
*
* @return 
*/
int sw_fb_open(void)
{
	set_fb_default_params();
	init_lcd();
	return SW_OK;
	
}

/**
* @brief 
*
* @return 
*/
int sw_fb_close(void)
{
	reset_fb_default_params();
	return SW_OK;
}

/**
* @brief 
*
* @param ioctl_id
* @param req_buf
* @param resp_buf
*
* @return 
*/
int sw_fb_ioctl(sw_uint ioctl_id, void* req_buf, void*resp_buf)
{
	sw_uint old_xres, old_yres;
	switch(ioctl_id) {
		case SW_FB_IOCTL_GET_INFO:
			if(resp_buf) {
				sw_memcpy(resp_buf, (void*)&g_fb_info, 
						sizeof(struct sw_fb_info));
				}
				else {
					sw_seterrno(SW_EINVAL);
					return SW_EINVAL;
				}
			break;
		case SW_FB_IOCTL_SET_INFO:
			if(req_buf) {
				old_xres = g_fb_info.xres;
				old_yres = g_fb_info.yres;				
				sw_memcpy((void*)&g_fb_info, req_buf, 
					sizeof(struct sw_fb_info));
				if( (old_xres != g_fb_info.xres) ||
					(old_yres != g_fb_info.yres)) {
					init_lcd();
				}
						
			}
			else {
					sw_seterrno(SW_EINVAL);
					return SW_EINVAL;
			}
			break;
		case SW_FB_IOCTL_WRITE_PIXEL:
			sw_fb_write_pixel((sw_uint)req_buf, (sw_uint)resp_buf);
			break;

		case SW_FB_IOCTL_WRITE_WINDOW:
			sw_fb_write_window(req_buf, (sw_int)resp_buf);
			break;
		case SW_FB_IOCTL_FLUSH_CACHE:
			sw_flush_fb_cache();
			break;

		default:
			sw_seterrno(SW_EINVAL);
			return SW_EINVAL;
		
	}
	return SW_OK;
}
