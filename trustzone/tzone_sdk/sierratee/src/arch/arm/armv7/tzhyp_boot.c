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
 * TEE  based hypervisor boot helper routines
 */

#include <nsk_boot.h>
#include <sw_board.h>
#include <sw_debug.h>
#include <sw_timer.h>
#include <mem_mng.h>
#include <sw_mem_functions.h>
#include <cpu_asm.h>
#include <monitor.h>
#include <tzhyp_global.h>
#include <tzhyp.h>
#include <page_table.h>
#include <task.h>
#include <sw_gic.h>

static struct nsk_boot_info ns_world_binfo[GUESTS_NO];
#ifdef CONFIG_MULTI_GUESTS_SUPPORT
static struct timer_event* tzhypboot_event;
static sw_timeval tzhypboot_time;
#endif

/**
 * @brief 
 */
static void tzhyp_load_guests(void);

/**
 * @brief 
 */
static void tzhyp_guest_context_init(void);

#ifdef CONFIG_MULTI_GUESTS_SUPPORT
/**
 * @brief 
 *
 * @param 
 */
static void tzhyp_bootevent_handler(struct timer_event *);

/**
 * @brief 
 */
static void tzhyp_bootevent(void);
#endif

/**
 * @brief 
 *
 * @return 
 */
int tzhyp_guest_init(void)
{
	int guest;

	/* Initialize Guest 1 */
	guest = 0;
	/* Assuming both start and load address are 1mb aligned*/
	ns_world_binfo[guest].nskbi_srcaddr = (sw_uint)&kernel_start; 
	ns_world_binfo[guest].nskbi_loadaddr = NSK_LOAD_ADDRESS; 
	ns_world_binfo[guest].nskbi_size  = 
		((sw_uint)&kernel_end) - ((sw_uint)&kernel_start);
	ns_world_binfo[guest].nskbi_initrd_flag = 0;


#ifdef CONFIG_MULTI_GUESTS_SUPPORT
	/* Initialize Guest 2 */
	guest = 1;
	/* Assuming both start and load address are 1mb aligned*/
	ns_world_binfo[guest].nskbi_srcaddr = (sw_uint)&kernel_2_start; 
	ns_world_binfo[guest].nskbi_loadaddr = 
		NSK_LOAD_ADDRESS + GUEST_MEM_SIZE; 
	ns_world_binfo[guest].nskbi_size  = 
		((sw_uint)&kernel_2_end) - ((sw_uint)&kernel_2_start);

#ifndef CONFIG_ZYNQ7_BOARD	
	/* Use initrd for the second guest */
	ns_world_binfo[guest].nskbi_initrd_flag = 1;
	ns_world_binfo[guest].nskbi_initrd_sa = (sw_uint)&initrd_image_start;
	ns_world_binfo[guest].nskbi_initrd_la = LINUX_INITRD_ADDR;
	ns_world_binfo[guest].nskbi_initrd_size = 
		((sw_uint)&initrd_image_end) - ((sw_uint)&initrd_image_start);
#endif


#endif

	/* Load all the guests to memory */
	tzhyp_load_guests();

	/* Initialize the register context for all the guests */
	tzhyp_guest_context_init();

	return SW_OK;
}

/**
 * @brief 
 */
static void tzhyp_load_guests(void)
{
	int guest;

	for (guest = 0; guest < GUESTS_NO; guest++)
		nsk_load(&ns_world_binfo[guest]);

#ifdef LINUX_ATAG_BOOT_EN
	nsk_atag_init();
#endif
}

/**
 * @brief 
 */
static void tzhyp_guest_context_init(void)
{
	int guest, core;
	struct system_context *core_ns_world, *reference;

	/* Initialize the guest 0 context to a sane state */
	smc_nscpu_context_init();

	tzhyp_device_context_init();

	/* Context base for guest 0 of primary core */
	core_ns_world = reference = 
		(struct system_context *)GET_CORE_CONTEXT_BYID(global_val.tzhyp_val.ns_world, 0); 

	/* Copy the guest 0 context to remaining guests */
	for (guest = 1; guest < GUESTS_NO; guest++)
		sw_memcpy(&core_ns_world[guest], reference, 
				sizeof(struct system_context));

	/* Initialize guest specific parameters */
	for (guest = 0; guest < GUESTS_NO; guest++) {
		core_ns_world[guest].sysctxt_core.lr_mon = 
			ns_world_binfo[guest].nskbi_loadaddr;
		core_ns_world[guest].guest_no = guest;
	}

	/* Initialize the secondary core contexts as well */ 
	for (core = 1; core < MAX_CORES; core++) {
		core_ns_world = 
			(struct system_context *)GET_CORE_CONTEXT_BYID(global_val.tzhyp_val.ns_world, 
					core);
		for (guest = 0; guest < GUESTS_NO; guest++) {
			sw_memcpy(&core_ns_world[guest], reference, 
					sizeof(struct system_context));
			core_ns_world[guest].guest_no = guest;
		}
	}
}

#ifdef CONFIG_MULTI_GUESTS_SUPPORT
/**
 * @brief 
 */
static void tzhyp_bootevent(void)
{

	tzhypboot_event = timer_event_create(&tzhyp_bootevent_handler, 
			(void*)NULL);
	if(!tzhypboot_event){
		sw_printk("xxx : Cannot register Handler\n");
		return;
	}

	/* Time duration = 1s */
	tzhypboot_time.tval.nsec = 0;
	tzhypboot_time.tval.sec = 1;

	timer_event_start(tzhypboot_event, &tzhypboot_time);
}
#endif

#ifdef CONFIG_MULTI_GUESTS_SUPPORT
/**
 * @brief 
 *
 * @param x
 */
static void tzhyp_bootevent_handler(struct timer_event *x)
{

	if (global_val.tzhyp_val.ns_preempt_flag) {
		tzhyp_schedule_guest();
	}  else {
		timer_event_start(tzhypboot_event, &tzhypboot_time);	
	}
}
#endif

/**
 * @brief 
 */
void mon_nscpu_context_init()
{
	struct core_context *core_ctxt;
	struct cp15_context *cp15_ctxt;
#ifdef CONFIG_NEON_SUPPORT
	struct vfp_context *vfp_ctxt;
#endif
	struct system_context *primary_ns_world;

	primary_ns_world = (struct system_context *)
		GET_CORE_CONTEXT_BYID(global_val.tzhyp_val.ns_world, 0);
	core_ctxt = &primary_ns_world->sysctxt_core;
	cp15_ctxt = &primary_ns_world->sysctxt_cp15;
#ifdef CONFIG_NEON_SUPPORT
	vfp_ctxt  = &primary_ns_world->sysctxt_vfp;
#endif
	sw_memset(core_ctxt, 0, sizeof(struct core_context));
#ifdef CONFIG_NEON_SUPPORT
	sw_memset(vfp_ctxt, 0, sizeof(struct vfp_context));
#endif
#ifdef LINUX_ATAG_BOOT_EN
	core_ctxt->r0 = 0;
	core_ctxt->r1 = LINUX_MACHINE_ID;
	core_ctxt->r2 = (sw_uint)NORMAL_WORLD_RAM_START + 0x100;
#endif

	core_ctxt->spsr_mon = CPSR_RESET_VAL;

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	primary_ns_world->notify_data = NULL;
#endif


	/* 
	 * Save cp15 reset state
	 */
	tzhyp_sysregs_save(cp15_ctxt);
}

/**
 * @brief 
 */
void tzhyp_boot_ack_event(void)
{
#ifdef CONFIG_MULTI_GUESTS_SUPPORT
	static int guest_booted = 0;
	
	sw_printk("tzhyp_boot ack event\n");

	guest_booted++;
	if (guest_booted < GUESTS_NO)
		tzhyp_bootevent();
	else if (guest_booted == GUESTS_NO)
		tzhyp_schedevent_init();
	else 
#endif	
		sw_printk("tzhyp: Warning! Ignoring wrong ack event\n");
}

/**
* @brief 
*/

void display_boot_info_nsk(void)
{
	int guest;
	for(guest = 0; guest < GUESTS_NO; guest++) {
		sw_printk("GUEST %d load addr : %x\n",
		       guest,ns_world_binfo[guest].nskbi_loadaddr);
		sw_printk("GUEST %d size : %x\n",
		       guest,ns_world_binfo[guest].nskbi_size);
		if(ns_world_binfo[guest].nskbi_initrd_flag)
		       sw_printk("GUEST %d initrd size : %x\n",
			       guest,ns_world_binfo[guest].nskbi_initrd_size);
	}
	return;
}

sw_int get_current_guest_no(void)
{
	return ns_sys_current->guest_no;
}

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
/**
 * @brief Register the notification shared memory data for notification.
 *
 * This function creates the page table entry for sharing the memory data for
 * notification
 *
 * @param notify_data_phys: Physical address of the shared memory used for
 * Notification data
 *
 * @return otz_return_t:
 * OTZ_OK - Shared memory registration success\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int register_notify_data(sw_phy_addr notify_data_phys)
{
	int ret_val = SW_OK;
	struct otzc_notify_data *data;
	if(map_to_ns(notify_data_phys, (sw_vir_addr*)&data) != 0) {
		ret_val = SW_ERROR;
		goto ret_func;
	}

	ns_sys_current->notify_data = data;
	ns_sys_current->notify_data->guest_no = ns_sys_current->guest_no;
ret_func:
	return ret_val;
}

/**
 * @brief Un-register the shared memory used for the notification
 *
 * This function removed the page table entry created for sharing the memory data
 * of notification
 */
void unregister_notify_data(void)
{ 
	if(ns_sys_current->notify_data)
		unmap_from_ns((sw_vir_addr)ns_sys_current->notify_data);

	ns_sys_current->notify_data = NULL;

}

/**
 * @brief Sets the values of Notification
 *
 * This function sets the notification data which is used by non-secure
 * application upon notification from secure world.
 *
 * @param guest_no: guest_no
 * @param service_id: Service ID
 * @param session_id: Session ID
 * @param enc_id: Encoded context ID
 * @param client_pid: Non-secure application PID
 */
void set_notify_data(int guest_no, int service_id, int session_id, int enc_id, 
		int client_pid, int dev_file_id)
{
	struct system_context *core_ns_world ;

	/* Context base for guest 0 of primary core */
	core_ns_world = 
		(struct system_context *)GET_CORE_CONTEXT_BYID(global_val.tzhyp_val.ns_world, 0); 
	core_ns_world = core_ns_world + guest_no;

	if(core_ns_world->notify_data) {
		if(core_ns_world->notify_data->guest_no != guest_no) {
			sw_printk("wrong notify data. expected guest no %d \
	and actual guest no %d\n", core_ns_world->notify_data->guest_no,
			guest_no);
		}
		else {
			core_ns_world->notify_data->service_id = service_id;
			core_ns_world->notify_data->session_id = session_id;
			core_ns_world->notify_data->enc_id = enc_id;
			core_ns_world->notify_data->client_pid = client_pid;
			core_ns_world->notify_data->dev_file_id = dev_file_id;
			if(core_ns_world != ns_sys_current) {
				core_ns_world->pending_notify = 1;
			}
			else {
				if(!global_val.g_ns_notify_pending)
					sw_generate_soft_int(NS_SGI_NOTIFY_INTERRUPT, SGI_TGT_CPU0);
				else 
					core_ns_world->pending_notify = 1;
			}
		}
	}
}

#endif
