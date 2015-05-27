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

/* Linker script */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH("arm")

ENTRY(_start)

PHYS_OFFSET = 0xCC000000;

SECTIONS
{
    . = PHYS_OFFSET;
    _SW_CODE_START = .;

    .text : { 
	    build/kernel.o (.text);
	  	build/kernel.o (.rodata .rodata.*)
	}
	. = ALIGN(0x1000);
	_SW_FS_START = .;
	fs_image : {
		*(fs_image);
	}
	. = ALIGN(0x1000);
    _SW_TEXT_END = .;
    _text_size = _SW_TEXT_END - _SW_CODE_START;

    .init : {build/kernel.o (.init) }
    _MOD_INIT_SECTION_START = .;
    .mod_init : {build/kernel.o (.mod_init) }
    _MOD_INIT_SECTION_END = .;
    . = ALIGN(0x1000);
    _MOD_INIT_PADDING_END = .;
    _init_size = _MOD_INIT_PADDING_END - _SW_TEXT_END;
    . = ALIGN(0x1000);

    _SW_LIBC_SECTION_START = .;    
	libc_section :
		{
			libc.o (.text);
			libc.o (.rodata .rodata.*);
			libc.o  (.data);
			libc.o (.bss);
			libc.o (COMMON);
		}
    . = ALIGN(0x1000);
    _SW_LIBC_SECTION_END = .;    
    _libc_size = _SW_LIBC_SECTION_END - _SW_LIBC_SECTION_START;

    _SW_LIBC_SYM_TBL_START = .;    
	libc_sym_section :
		{
			libc.o (.symtab);
		}
    . = ALIGN(0x1000);
    _SW_LIBC_SYM_TBL_END = .;    
    _libc_sym_tbl_size = _SW_LIBC_SYM_TBL_END - _SW_LIBC_SYM_TBL_START;

    _SW_LIBC_STR_TBL_START = .;    
	libc_str_section :
		{
			libc.o (.shstrtab);
			libc.o (.strtab);			
		}
    . = ALIGN(0x1000);
    _SW_LIBC_STR_TBL_END = .;    
    _libc_str_tbl_size = _SW_LIBC_STR_TBL_END - _SW_LIBC_STR_TBL_START;

    _SW_APP_SECTION_START = .;       
	user_app_text :
		{
			user_app.o (.text);
			user_app.o (.rodata .rodata.*)
		}
    . = ALIGN(0x1000);		
    _SW_APP_TEXT_SECTION_END = .;                		
    _app_text_size = _SW_APP_TEXT_SECTION_END - _SW_APP_SECTION_START;
    
	user_app_data :
		{
			user_app.o (.data);
		}
    . = ALIGN(0x1000);		
    _SW_APP_DATA_SECTION_END = .;                		
    _app_data_size = _SW_APP_DATA_SECTION_END - _SW_APP_TEXT_SECTION_END;
    		
	user_app_bss :
		{
			user_app.o (.bss);
			user_app.o (COMMON);
		}
    . = ALIGN(0x1000);
    _SW_APP_SECTION_END = .;                
    _app_bss_size = _SW_APP_SECTION_END - _SW_APP_DATA_SECTION_END;    
	_app_section_size = _SW_APP_SECTION_END - _SW_APP_SECTION_START;    
	_SW_DATA_START = .;
    .data : { build/kernel.o (.data) }
    . = ALIGN(0x1000);
    _SW_DATA_END = .;
    _data_size = _SW_DATA_END - _SW_DATA_START;

/* 
    PROVIDE(_NSADMIN_SECTION_START = .);
        .nsadmin : {
		* (.nsadmin);
		}
    PROVIDE(_NSADMIN_SECTION_END = .);
*/

	normal_image : {
		*(normal_image);
	}
    . = ALIGN(0x1000);

	normal_image_2 : {
		*(normal_image_2);
	}
    . = ALIGN(0x1000);

	initrd_image : {
		*(initrd_image);
	}
    . = ALIGN(0x1000);


    _SW_BSS_START = .;
   .bss : { build/kernel.o (.bss) build/kernel.o (COMMON)}
    . = ALIGN(0x1000);
   _SW_CODE_END = .;
   _bss_size = _SW_CODE_END - _SW_BSS_START;

    _SW_BSS_END = .;   

	_SW_MEM_INFO_START =.;
	mem_info : {
		*(mem_info);
	}
    . = ALIGN(0x1000);	
	_SW_MEM_INFO_END =.;	
   _mem_info_size = _SW_MEM_INFO_END - _SW_MEM_INFO_START;    

    . = ALIGN(0x100000);

	PROVIDE (end = .);
	_total_sw_size = end - _SW_CODE_START;
}

