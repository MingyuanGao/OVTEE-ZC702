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
 * Header for system context declarations
 */

#ifndef SYSTEM_CONTEXT_H
#define SYSTEM_CONTEXT_H

#include <sw_types.h>
#include <sw_board.h>  /* for GIC_ITLINES */
#include <cpu.h>

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
#include <otz_common.h>
#endif

/**
 * @brief 
 * 
 * This has all the core context registers of ARM. 
 * When guests are switched, the switched out guest stores it's
 * register values into the context registers. This is then used to 
 * repopulate the registers when it's switched back in.
 *
 */
struct core_context {
	sw_uint r0;
	sw_uint r1;
	sw_uint r2;
	sw_uint r3;
	sw_uint r4;
	sw_uint r5;
	sw_uint r6;
	sw_uint r7;
	sw_uint r8;
	sw_uint r9;
	sw_uint r10;
	sw_uint r11;
	sw_uint r12;
	sw_uint spsr_mon;
	sw_uint lr_mon;
	sw_uint spsr_svc;
	sw_uint r13_svc;
	sw_uint lr_svc;
	sw_uint r13_sys;
	sw_uint lr_sys;
	sw_uint spsr_abt;
	sw_uint r13_abt;
	sw_uint lr_abt;
	sw_uint spsr_undef;
	sw_uint r13_undef;
	sw_uint lr_undef;
	sw_uint spsr_irq;
	sw_uint r13_irq;
	sw_uint lr_irq;		
};

/**
 * @brief 
 *
 * These are control registers to which the register values are saved
 * when a guet is being switched out.
 *
 */
struct cp15_context { 
	sw_uint c0_CSSELR;      /* Cache Size Selection Register */
	sw_uint c1_SCTLR;       /* System Control Register */
	sw_uint c1_ACTLR;       /* Auxilliary Control Register */
	sw_uint c2_TTBR0;       /* Translation Table Base Register 0 */
	sw_uint c2_TTBR1;       /* Translation Table Base Register 1 */
	sw_uint c2_TTBCR;       /* Translation Table Base Register Control */
	sw_uint c3_DACR;        /* Domain Access Control Register */
	sw_uint c5_DFSR;        /* Data Fault Status Register */
	sw_uint c5_IFSR;        /* Instruction Fault Status Register */
	sw_uint c6_DFAR;        /* Data Fault Address Register */
	sw_uint c6_IFAR;        /* Instruction Fault Address Register */
	sw_uint c7_PAR;         /* Physical Address Register */
	sw_uint c10_PRRR;       /* PRRR */
	sw_uint c10_NMRR;       /* NMRR */
	sw_uint c12_VBAR;       /* VBAR register */
	sw_uint c13_FCSEIDR;    /* FCSE PID Register */
	sw_uint c13_CONTEXTIDR; /* Context ID Register */
	sw_uint c13_TPIDRURW;   /* User Read/Write Thread and Process ID */
	sw_uint c13_TPIDRURO;   /* User Read-only Thread and Process ID */
	sw_uint c13_TPIDRPRW;   /* Privileged only Thread and Process ID */
};


/**
 * @brief 
 *
 * Generic Interrupt Controller context
 *
 */
struct gic_context {
	sw_uint gic_icdiser[GIC_ITLINES+1];
};

#ifdef CONFIG_NEON_SUPPORT
/**
 * @brief 
 *    vfp-neon register bank
 */
struct vfp_context {
	sw_uint FPEXC;              /* Floating point Exception Register*/
	sw_uint FPSCR;	        /* Floating point Status and control Register*/
	sw_uint FPSID;              /* Floating point system ID Register*/
	sw_big_ulong dreg[32];           /* 32 double word D Registers*/
};
#endif
/* 
 * Please do not change the order of the members, until we remove the 
 * below assumption made by cpu context (core registers) switching code in 
 * monitor mode
 *
 * struct system_context x; 
 * "&x.sysctxt_core" is same as "&x"
 */
struct system_context {
	/* CPU context */
	struct core_context sysctxt_core;
	struct cp15_context sysctxt_cp15;
#ifdef CONFIG_NEON_SUPPORT
	struct vfp_context sysctxt_vfp;
#endif
	/* Devices */
	struct gic_context sysctxt_gic;	

	sw_uint guest_no;

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	/*! Shared memory for notification */
	struct otzc_notify_data *notify_data;
	sw_uint pending_notify;
#endif
	
	/* 
	 * to make the size a power of 2, so that multiplication can be acheived 
	 * by logical shift
	 */
#ifndef CONFIG_NEON_SUPPORT  
	sw_uint pad[8];  
#endif
} __attribute__ ((aligned (CACHELINE_SIZE)));


/**
 * @brief 
 *
 * This function restores register values when a guest is swiched in
 *
 * @param 
 * @param 
 */
extern void tzhyp_sysregs_switch(struct cp15_context *, struct cp15_context *);

/**
 * @brief 
 * 
 * cp15 context registers are saved when a guest is 
 * switched out.
 * @param 
 */
extern void tzhyp_sysregs_save(struct cp15_context *);

#endif
