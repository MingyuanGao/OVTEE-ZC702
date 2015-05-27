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
*/

/* Sierraware Trustzone API interface driver. */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <asm/cacheflush.h>
#ifdef CONFIG_KIM
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/stat.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include "md5.h"
#endif
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <otz_client.h>
#include <otz_common.h>
#include <otz_id.h>
#include <smc_id.h>
#include <sw_config.h>


#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
#include <linux/smp.h>
#endif

#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

#define OTZ_MEM_SERVICE_RW (0x2)

#ifdef CONFIG_KIM
#define KERN_TEXT_START 0xc0030000
#define KERN_TEXT_SIZE 0x3361f0

#define KERN_TEXT_END (KERN_TEXT_START + KERN_TEXT_SIZE -1)
/**
 * @brief 
 */
typedef struct file_struct {
	char	 	   hash[33];
	int              file_reported;
	char             fullpath[256];
	/* 
	   dev_t            dev;
	   ino_t            ino;
	   mode_t           mode;
	   nlink_t          hardlinks;
	   uid_t            owner;
	   gid_t            group;
	   dev_t            rdev;
	   off_t            size;
	   unsigned long    blksize;
	   unsigned long    blocks;
	   struct timespec  atime;
	   struct timespec  mtime;
	   struct timespec  ctime;*/
} file_type;

#endif

static struct class *driver_class;
static dev_t otz_client_device_no;
static struct cdev otz_client_cdev;

static u32 cacheline_size;
static u32 device_file_cnt = 0;

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
static struct otzc_notify_data *notify_data = NULL;
static int current_guest_id;
#endif

static struct otz_smc_cdata otz_smc_cd[NR_CPUS];


/**
 * @brief 
 *
 * @param in_lock
 */
static DEFINE_MUTEX(in_lock);

/**
 * @brief 
 *
 * @param send_cmd_lock
 */
static DEFINE_MUTEX(send_cmd_lock);

/**
 * @brief 
 *
 * @param smc_lock
 */
static DEFINE_MUTEX(smc_lock);

/**
 * @brief 
 *
 * @param encode_cmd_lock
 */
static DEFINE_MUTEX(encode_cmd_lock);

/**
 * @brief 
 *
 * @param im_check_lock
 */
static DEFINE_MUTEX(im_check_lock);

/**
 * @brief 
 *
 * @param decode_cmd_lock
 */
static DEFINE_MUTEX(decode_cmd_lock);

/**
 * @brief 
 *
 * @param ses_open_lock
 */
static DEFINE_MUTEX(ses_open_lock);

/**
 * @brief 
 *
 * @param ses_close_lock
 */
static DEFINE_MUTEX(ses_close_lock);

/**
 * @brief 
 *
 * @param mem_free_lock
 */
static DEFINE_MUTEX(mem_free_lock);

/**
 * @brief 
 *
 * @param mem_alloc_lock
 */
static DEFINE_MUTEX(mem_alloc_lock);

/**
 * @brief 
 */
static struct otz_dev_file_head{
	u32 dev_file_cnt;
	struct list_head dev_file_list;
} otzc_dev_file_head;

/**
 * @brief 
 */
typedef struct otzc_shrd_mem_head{

	int shared_mem_cnt;
	struct list_head shared_mem_list;
} otzc_shared_mem_head;

/**
 * @brief 
 */
typedef struct otz_dev_file{
	struct list_head head;
	u32 dev_file_id;
	u32 service_cnt;
	struct list_head services_list;
	otzc_shared_mem_head dev_shared_mem_head;
} otzc_dev_file;

/**
 * @brief 
 */
typedef struct otzc_service{
	struct list_head head;
	u32 service_id;
	struct list_head sessions_list;
} otzc_service;


/**
 * @brief 
 */
typedef struct otzc_session{
	struct list_head head;
	int session_id;

	struct list_head encode_list;
	struct list_head shared_mem_list;
} otzc_session;

/**
 * @brief 
 */
struct otz_wait_data {
	wait_queue_head_t send_cmd_wq;
	int               send_wait_flag;
};

/**
 * @brief 
 */
typedef struct otzc_encode{

	struct list_head head;

	int encode_id;

	void* ker_req_data_addr;
	void* ker_res_data_addr;

	u32  enc_req_offset;
	u32  enc_res_offset;
	u32  enc_req_pos;
	u32  enc_res_pos;
	u32  dec_res_pos;

	u32  dec_offset;

	struct otz_wait_data wait_data;

	struct otzc_encode_meta *meta;
} otzc_encode;



/**
 * @brief 
 */
typedef struct otzc_shared_mem{

	struct list_head head;
	struct list_head s_head;

	void* index;

	void* k_addr;
	void* u_addr;
	u32  len;
} otzc_shared_mem;

static int otz_client_prepare_encode(void* private_data,
		struct otz_client_encode_cmd *enc,
		otzc_encode **penc_context,
		otzc_session **psession);

/**
 * @brief 
 *
 * @param cmd_addr
 *
 * @return 
 */
static u32 _otz_smc(u32 cmd_addr)
{
	register u32 r0 asm("r0") = CALL_TRUSTZONE_API;
	register u32 r1 asm("r1") = cmd_addr;
	register u32 r2 asm("r2") = OTZ_CMD_TYPE_NS_TO_SECURE;


	do {
		asm volatile(
#if USE_ARCH_EXTENSION_SEC
				".arch_extension sec\n\t"
#endif
				__asmeq("%0", "r0")
				__asmeq("%1", "r0")
				__asmeq("%2", "r1")
				__asmeq("%3", "r2")
				"smc    #0  @ switch to secure world\n"
				: "=r" (r0)
				: "r" (r0), "r" (r1), "r" (r2));
	} while (0);

	return r0;
}

/**
 * @brief 
 *
 * @param otz_smc handler for secondary cores
 *
 * @return 
 */
static void secondary_otz_smc_handler(void *info)
{
	struct otz_smc_cdata *cd = (struct otz_smc_cdata *)info;

	rmb();

	TDEBUG("secondary otz smc handler...");

	cd->ret_val = _otz_smc(cd->cmd_addr);
	wmb();

	TDEBUG("done smc on primary \n");
}

/**
 * @brief This function takes care of posting the smc to the 
 *        primary core
 *
 * @param cpu_id
 * @param cmd_addr
 *
 * @return 
 */
static u32 post_otz_smc(int cpu_id, u32 cmd_addr)
{
	struct otz_smc_cdata *cd = &otz_smc_cd[cpu_id];

	TDEBUG("Post from secondary ...");

	cd->cmd_addr = cmd_addr;
	cd->ret_val  = 0;
	wmb();

	smp_call_function_single(0, secondary_otz_smc_handler, 
			(void *)cd, 1);
	rmb();

	TDEBUG("completed smc on secondary \n");

	return cd->ret_val;
}

/**
 * @brief otz_smc wrapper to handle the multi core case
 *
 * @param cmd_addr
 *
 * @return 
 */
static u32 otz_smc(u32 cmd_addr)
{
	int cpu_id = smp_processor_id();

	if (cpu_id != 0) {
		mb();
		return post_otz_smc(cpu_id, cmd_addr); /* post it to primary */
	} else {
		return _otz_smc(cmd_addr); /* called directly on primary core */
	}
}

/**
 * @brief Call SMC
 *
 * When the processor executes the Secure Monitor Call (SMC),
 * the core enters Secure Monitor mode to execute the Secure Monitor code
 *
 * @param svc_id  - service identifier
 * @param cmd_id  - command identifier
 * @param context - session context
 * @param enc_id - encoder identifier
 * @param cmd_buf - command buffer 
 * @param cmd_len - command buffer length
 * @param resp_buf - response buffer
 * @param resp_len - response buffer length
 * @param meta_data
 * @param ret_resp_len
 *
 * @return 
 */
static int otz_smc_call(u32 dev_file_id, u32 svc_id, u32 cmd_id,
		u32 context, u32 enc_id, const void *cmd_buf,
		size_t cmd_len, void *resp_buf, size_t resp_len, 
		const void *meta_data, int *ret_resp_len, 
		struct otz_wait_data* wq, void* arg_lock)
{
	int ret;
	u32 smc_cmd_phys;

	static struct otz_smc_cmd *smc_cmd;

	smc_cmd = (struct otz_smc_cmd*)kmalloc(sizeof(struct otz_smc_cmd), 
			GFP_KERNEL);
	if(!smc_cmd){
		TERR("kmalloc failed for smc command\n");
		ret = -ENOMEM;
		goto out;
	}

	if(ret_resp_len)
		*ret_resp_len = 0;

	smc_cmd->src_id = (svc_id << 10) | cmd_id;
	smc_cmd->src_context = task_tgid_vnr(current);

	smc_cmd->id = (svc_id << 10) | cmd_id;
	smc_cmd->context = context;
	smc_cmd->enc_id = enc_id;
	smc_cmd->dev_file_id = dev_file_id;
	smc_cmd->req_buf_len = cmd_len;
	smc_cmd->resp_buf_len = resp_len;
	smc_cmd->ret_resp_buf_len = 0;

	if(cmd_buf)
		smc_cmd->req_buf_phys = virt_to_phys((void*)cmd_buf);
	else
		smc_cmd->req_buf_phys = 0;

	if(resp_buf)
		smc_cmd->resp_buf_phys = virt_to_phys((void*)resp_buf);
	else
		smc_cmd->resp_buf_phys = 0;

	if(meta_data)
		smc_cmd->meta_data_phys = virt_to_phys(meta_data);
	else
		smc_cmd->meta_data_phys = 0;        

	smc_cmd_phys = virt_to_phys((void*)smc_cmd);

	mutex_lock(&smc_lock);
	ret = otz_smc(smc_cmd_phys);
	mutex_unlock(&smc_lock);

#ifdef  OTZONE_ASYNC_NOTIFY_SUPPORT
	if(ret == SMC_PENDING){
		if(arg_lock)
			mutex_unlock(arg_lock);

		if(wq){
			if(wait_event_interruptible(wq->send_cmd_wq,
						wq->send_wait_flag)) {
				ret = -ERESTARTSYS;
				goto out;
			}
			wq->send_wait_flag = 0;
		}

		if(arg_lock)
			mutex_lock(arg_lock);


		svc_id = OTZ_SVC_GLOBAL;
		cmd_id = OTZ_GLOBAL_CMD_ID_RESUME_ASYNC_TASK;
		smc_cmd->src_id = (svc_id << 10) | cmd_id;
		smc_cmd->id = (svc_id << 10) | cmd_id;

		mutex_lock(&smc_lock);

		ret = otz_smc(smc_cmd_phys);
		mutex_unlock(&smc_lock);

	}
#endif

	if (ret) {
		TERR("smc_call returns error\n");

		goto out;
	}

	if(ret_resp_len) {
		*ret_resp_len = smc_cmd->ret_resp_buf_len;
	}

out:
	if(smc_cmd)
		kfree(smc_cmd);
	return ret;
}

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
static void ipi_secure_notify( struct pt_regs *regs)
{
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	otzc_encode *enc_temp;

	int enc_found = 0;

	if(!notify_data)
		return;

	if(notify_data->guest_no != current_guest_id) {
		TERR("Invalid notification from guest id %d\n", 
				notify_data->guest_no);
	}
	TDEBUG("guest id %d\n", notify_data->guest_no);
	TDEBUG("otz_client pid 0x%x\n", notify_data->client_pid);
	TDEBUG("otz_client_notify_handler service id 0x%x \
			session id 0x%x and encoder id 0x%x\n", 
			notify_data->service_id, notify_data->session_id,
			notify_data->enc_id); 

	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == notify_data->dev_file_id){
			TDEBUG("dev file id %d \n",temp_dev_file->dev_file_id);

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head){
				if(temp_svc->service_id == notify_data->service_id) {
					TDEBUG("send cmd ser id %d \n",temp_svc->service_id);

					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id == notify_data->session_id) {
							TDEBUG("send cmd ses id %d \n",
									temp_ses->session_id);

							list_for_each_entry(enc_temp,&temp_ses->encode_list,
									head) {
								if(enc_temp->encode_id == notify_data->enc_id) {
									TDEBUG("send cmd enc id 0x%x\n",
											enc_temp->encode_id);
									enc_found = 1;
									break;
								}
							}
						}
						break;        
					}
					break;
				}
			}
			break;
		}
	}

	if(enc_found) {
		if(!enc_temp->wait_data.send_wait_flag) {
			enc_temp->wait_data.send_wait_flag = 1;
			wake_up_interruptible(&enc_temp->wait_data.send_cmd_wq);
		}
		else {
			printk("Duplicate  Notifcation data\n");
		}
	}
	else {
		printk("Notifcation data is missing\n");
	}

	return;
}
#endif
/**
 * @brief
 * 
 * Clears session id and closes the session 
 *
 * @param private_data
 * @param temp_svc
 * @param temp_ses
 *
 */
static void otz_client_close_session_for_service(
		void* private_data,
		otzc_service* temp_svc, 
		otzc_session *temp_ses)
{

	otzc_encode *temp_encode, *enc_context;
	otzc_shared_mem *shared_mem, *temp_shared;
	u32 dev_file_id = (u32)private_data;

	if(!temp_svc || !temp_ses)
		return;

	TDEBUG("freeing ses_id %d \n",temp_ses->session_id); 

	otz_smc_call(dev_file_id, OTZ_SVC_GLOBAL, 
			OTZ_GLOBAL_CMD_ID_CLOSE_SESSION, 0, 0,
			&temp_svc->service_id,
			sizeof(temp_svc->service_id),&temp_ses->session_id,
			sizeof(temp_ses->session_id), NULL, NULL, NULL, NULL); 

	list_del(&temp_ses->head);

	if (!list_empty(&temp_ses->encode_list)) {
		list_for_each_entry_safe(enc_context, temp_encode, 
				&temp_ses->encode_list, head) {
			list_del(&enc_context->head);   
			kfree(enc_context);
		}
	}

	if (!list_empty(&temp_ses->shared_mem_list)) {
		list_for_each_entry_safe(shared_mem, temp_shared, 
				&temp_ses->shared_mem_list, s_head) {
			list_del(&shared_mem->s_head);   

			if(shared_mem->k_addr)
				free_pages((u32)shared_mem->k_addr,
						get_order(ROUND_UP(shared_mem->len, SZ_4K)));

			kfree(shared_mem);
		}
	}

	kfree(temp_ses);
}

/**
 * @brief 
 *
 * @param service_id
 *
 * @return 
 */
static int otz_client_service_init(otzc_dev_file* dev_file, int service_id)
{
	int ret_code = 0;
	otzc_service* svc_new;
	otzc_service* temp_pos;
	svc_new = (otzc_service*)kmalloc(sizeof(otzc_service), GFP_KERNEL);
	if(!svc_new){
		TERR("kmalloc failed \n");
		ret_code = -ENOMEM;
		goto clean_prev_malloc;
	}

	svc_new->service_id = service_id;
	dev_file->service_cnt++;
	INIT_LIST_HEAD(&svc_new->sessions_list);
	list_add(&svc_new->head, &dev_file->services_list);
	goto return_func;

clean_prev_malloc:
	if (!list_empty(&dev_file->services_list)) {
		list_for_each_entry_safe(svc_new, temp_pos, 
				&dev_file->services_list, head) {
			list_del(&svc_new->head);   
			kfree(svc_new);
		}
	}

return_func:
	return ret_code;
}


/**
 * @brief 
 *
 * @return 
 */
static int otz_client_service_exit(void* private_data)
{
	otzc_shared_mem* temp_shared_mem;
	otzc_shared_mem  *temp_pos;
	otzc_dev_file *tem_dev_file, *tem_dev_file_pos;
	otzc_session *temp_ses, *temp_ses_pos;
	otzc_service* tmp_svc = NULL, *tmp_pos;
	u32 dev_file_id;


	dev_file_id = (u32)(private_data);
	list_for_each_entry_safe(tem_dev_file, tem_dev_file_pos,
			&otzc_dev_file_head.dev_file_list, head) {
		if(tem_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry_safe(temp_shared_mem, temp_pos, 
					&tem_dev_file->dev_shared_mem_head.shared_mem_list, head){
				list_del(&temp_shared_mem->head);

				if(temp_shared_mem->k_addr)
					free_pages((u32)temp_shared_mem->k_addr,
							get_order(ROUND_UP(temp_shared_mem->len, SZ_4K)));

				if(temp_shared_mem)
					kfree(temp_shared_mem);
			}
			if (!list_empty(&tem_dev_file->services_list)) {

				list_for_each_entry_safe(tmp_svc, tmp_pos,
						&tem_dev_file->services_list, head) {

					list_for_each_entry_safe(temp_ses, temp_ses_pos, 
							&tmp_svc->sessions_list, head) {
						otz_client_close_session_for_service(private_data, 
								tmp_svc, temp_ses);
					}
					list_del(&tmp_svc->head);   
					kfree(tmp_svc);
				}
			}

			list_del(&tem_dev_file->head);   
			kfree(tem_dev_file);
			break;
		}
	}

	return 0;
}



/**
 * @brief 
 *
 * Opens session for the requested service by getting the service ID
 * form the user. After opening the session, the session ID is copied back
 * to the user space.
 *
 * @param private_data - Holds the device file ID
 * @param argp - Contains the Service ID
 *
 * @return 
 */
static int otz_client_session_open(void* private_data, void* argp)
{
	otzc_service* svc;
	otzc_dev_file *temp_dev_file;
	otzc_session* ses_new;
	struct ser_ses_id ses_open;
	int svc_found = 0;
	int ret_val = 0, ret_resp_len;
	u32 dev_file_id = (u32)private_data;

	TDEBUG("inside session open\n");

	if(copy_from_user(&ses_open, argp, sizeof(ses_open))){
		TERR("copy from user failed\n");
		ret_val =  -EFAULT;
		goto return_func;
	}

	TDEBUG("service_id = %d\n",ses_open.service_id);

	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(svc, &temp_dev_file->services_list, head){
				if( svc->service_id == ses_open.service_id){
					svc_found = 1;
					break;
				}
			}
			break;
		}
	}

	if(!svc_found) {
		ret_val =  -EINVAL;
		goto return_func;
	}

	ses_new = (otzc_session*)kmalloc(sizeof(otzc_session), GFP_KERNEL);
	if(!ses_new) {
		TERR("kmalloc failed\n");
		ret_val =  -ENOMEM;
		goto return_func;
	}

	TDEBUG("service id 0x%x\n", ses_open.service_id);

	ret_val = otz_smc_call(dev_file_id, OTZ_SVC_GLOBAL,
			OTZ_GLOBAL_CMD_ID_OPEN_SESSION, 0, 0,
			&ses_open.service_id, sizeof(ses_open.service_id),
			&ses_new->session_id,sizeof(ses_new->session_id), NULL,
			&ret_resp_len, NULL, NULL);      


	if(ret_val != SMC_SUCCESS) {
		goto clean_session;
	}

	if(ses_new->session_id == -1) {
		TERR("invalid session id\n");
		ret_val =  -EINVAL;
		goto clean_session;
	}

	TDEBUG("session id 0x%x for service id 0x%x\n", ses_new->session_id,
			ses_open.service_id);

	ses_open.session_id = ses_new->session_id;

	INIT_LIST_HEAD(&ses_new->encode_list);
	INIT_LIST_HEAD(&ses_new->shared_mem_list);
	list_add_tail(&ses_new->head, &svc->sessions_list);

	if(copy_to_user(argp, &ses_open, sizeof(ses_open))) {
		TERR("copy from user failed\n");
		ret_val =  -EFAULT;
		goto clean_hdr_buf;
	}

	/*   TDEBUG("session created from service \n"); */
	goto return_func;

clean_hdr_buf:
	list_del(&ses_new->head);

clean_session:
	kfree(ses_new);

return_func:

	return ret_val;
}

/**
 * @brief 
 *
 * Closes the client session by getting the service ID and
 * session ID from user space.
 *
 * @param private_data - Contains the device file ID
 * @param argp - Contains the service ID and Session ID
 *
 * @return 
 */
static int otz_client_session_close(void* private_data, void* argp)
{

	struct ser_ses_id ses_close;
	int ret_val = 0;
	u32 dev_file_id = (u32)private_data;
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;

	TDEBUG("inside session close\n");

	if(copy_from_user(&ses_close, argp, sizeof(ses_close))) {
		TERR("copy from user failed \n");
		ret_val = -EFAULT;
		goto return_func;
	}

	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == ses_close.service_id) {

					list_for_each_entry(temp_ses,
							&temp_svc->sessions_list, head) {
						if(temp_ses->session_id == ses_close.session_id) {
							otz_client_close_session_for_service(private_data, 
									temp_svc, temp_ses);
							break;
						}                      
					}
					break;
				}
			}
			break;
		}
	}

	TDEBUG("return from close\n");

return_func:
	return ret_val;
}


static int otz_client_register_service(void) __attribute__((used));

/**
 * @brief 
 *
 * @return 
 */
static int otz_client_register_service(void)
{
	/* Query secure and find out */
	return 0;
}

static int otz_client_unregister_service(void) __attribute__((used));
/**
 * @brief 
 *
 * @return 
 */
static int otz_client_unregister_service(void)
{
	/*query secure and do*/
	return 0;
}

/**
 * @brief 
 * 
 * Creates shared memory between non secure world applicaion
 * and non secure world kernel.
 * 
 * @param filp
 * @param vma
 *
 * @return 
 */
static int otz_client_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;
	otzc_shared_mem *mem_new;
	u32*  alloc_addr;
	long length = vma->vm_end - vma->vm_start;
	otzc_dev_file *temp_dev_file;

	TDEBUG("Inside otz_client mmap\n");

	alloc_addr =  (void*) __get_free_pages(GFP_KERNEL,
			get_order(ROUND_UP(length, SZ_4K)));
	if(!alloc_addr) {
		TERR("get free pages failed \n");
		ret = -ENOMEM;
		goto return_func;
	}

	TDEBUG("mmap k_addr %p \n",alloc_addr);

	if (remap_pfn_range(vma,
				vma->vm_start,
				((virt_to_phys(alloc_addr)) >> PAGE_SHIFT),
				length,
				vma->vm_page_prot)) {
		ret = -EAGAIN;
		goto return_func;
	}

	mem_new = kmalloc(sizeof(otzc_shared_mem), GFP_KERNEL); 
	if(!mem_new) {
		TERR("kmalloc failed\n");
		ret = -ENOMEM;
		goto return_func;
	}

	mem_new->k_addr = alloc_addr;
	mem_new->len = length;
	mem_new->u_addr = (void*)vma->vm_start;
	mem_new->index = mem_new->u_addr;
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == (u32)filp->private_data){
			break;
		}
	}
	temp_dev_file->dev_shared_mem_head.shared_mem_cnt++;
	list_add_tail( &mem_new->head ,
			&temp_dev_file->dev_shared_mem_head.shared_mem_list);

return_func:
	return ret;
}

/**
 * @brief 
 *
 * Sends a command from the non secure world Kernel to the 
 * secure world.
 *
 * @param private_data - Contains device file ID
 * @param argp
 *
 * @return 
 */
static int otz_client_kernel_send_cmd(void *private_data, void *argp)
{

	int ret = 0;
	int ret_resp_len = 0;
	int dev_file_id;
	struct otz_client_encode_cmd *enc = (struct otz_client_encode_cmd *)argp;
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	otzc_encode *enc_temp;

	int enc_found = 0;
	dev_file_id = (u32)private_data;

	TDEBUG("inside send cmd \n");

	TDEBUG("enc id %d\n",enc->encode_id);
	TDEBUG("dev file id %d\n",dev_file_id);
	TDEBUG("ser id %d\n",enc->service_id);
	TDEBUG("ses id %d\n",enc->session_id);

	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head){
				if(temp_svc->service_id == enc->service_id) {
					TDEBUG("send cmd ser id %d \n",temp_svc->service_id);

					list_for_each_entry(temp_ses, &temp_svc->sessions_list, 
							head) {
						if(temp_ses->session_id
								== enc->session_id) {
							TDEBUG("send cmd ses id %d \n",
									temp_ses->session_id);

							if(enc->encode_id != -1) {
								list_for_each_entry(enc_temp, 
										&temp_ses->encode_list, head) {
									if(enc_temp->encode_id == enc->encode_id){
										TDEBUG("send cmd enc id 0x%x\n",
												enc_temp->encode_id);
										enc_found = 1;
										break;
									}
								}
							}
							else {
								ret = otz_client_prepare_encode(
										private_data,
										enc, &enc_temp, &temp_ses);
								if(!ret) {
									enc_found = 1;
								}
								break;
							}
						}
						break;        
					}
					break;
				}
			}
			break;
		}
	}

	if(!enc_found){
		ret = -EINVAL;
		goto return_func;
	}


	ret = otz_smc_call(dev_file_id, enc->service_id, enc->cmd_id,
			enc->session_id, 
			enc->encode_id,
			enc_temp->ker_req_data_addr, enc_temp->enc_req_offset, 
			enc_temp->ker_res_data_addr, enc_temp->enc_res_offset, 
			enc_temp->meta, &ret_resp_len, &enc_temp->wait_data ,
			&send_cmd_lock);

	if(ret != SMC_SUCCESS) {
		TERR("send cmd secure call failed \n");
		goto return_func;
	}

	TDEBUG("smc_success\n");


return_func:
	return ret; 
}

/**
 * @brief 
 *
 * Sends a command from the non secure application
 * to the secure world.
 *
 * @param private_data
 * @param argp
 *
 * @return 
 */

static int otz_client_send_cmd(void* private_data, void* argp)
{
	struct otz_client_encode_cmd enc;
	int ret = 0;
	if(copy_from_user(&enc, argp, sizeof(enc))) {
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_kernel_send_cmd(private_data,&enc);
	if (ret == 0){
		if(copy_to_user(argp, &enc, sizeof(enc))) {
			TERR("copy to user failed \n");
			ret = -EFAULT;
			goto return_func;
		}
	}
return_func:
	return ret;

}

/**
 * @brief 
 *
 * Frees the encode context associated with a particular device and session
 *
 * @param private_data - device file ID
 * @param argp - Encode structure
 *
 * @return 
 */
static int otz_client_kernel_operation_release(void *private_data, void *argp)
{

	otzc_encode *enc_context = NULL;
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	u32 dev_file_id = (u32)private_data;
	int  session_found = 0, enc_found = 0;
	int ret =0;
	struct otz_client_encode_cmd *enc = (struct otz_client_encode_cmd *)argp;

	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == enc->service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id == enc->session_id) {
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}

	if(!session_found) {
		ret = -EINVAL;
		goto return_func;
	}

	if(enc->encode_id != -1) {
		list_for_each_entry(enc_context,&temp_ses->encode_list, head) {
			if(enc_context->encode_id == enc->encode_id) {
				enc_found = 1;
				break;        
			}
		}
	}
	if(enc_found && enc_context) {
		if(enc_context->ker_req_data_addr) 
			kfree(enc_context->ker_req_data_addr);

		if(enc_context->ker_res_data_addr) 
			kfree(enc_context->ker_res_data_addr);

		list_del(&enc_context->head);

		kfree(enc_context->meta);
		kfree(enc_context);
	}
return_func: 
	return ret;
}
/**
 * @brief 
 *
 * Same as otz_client_kernel_operation_release() but this is for a 
 * non-secure user application. So copy_from_user() is used.
 *
 * @param private_data - device file ID
 * @param argp - Encode structure
 *
 * @return 
 */
static int otz_client_operation_release(void* private_data, void *argp)
{
	struct otz_client_encode_cmd enc;
	int ret =0;

	if(copy_from_user(&enc, argp, sizeof(enc))) {
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}
	ret = otz_client_kernel_operation_release(private_data,&enc);
	if (ret != 0){
		TERR("Error in release\n");
	}
return_func: 
	return ret;
}

/**
 * @brief 
 *
 * Prepares and initializes the encode context.
 *
 * @param private_data
 * @param enc
 * @param penc_context
 * @param psession
 *
 * @return 
 */
static int otz_client_prepare_encode( void* private_data,
		struct otz_client_encode_cmd *enc,
		otzc_encode **penc_context,
		otzc_session **psession) 
{
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	otzc_encode *enc_context;
	int  session_found = 0, enc_found = 0;
	int ret = 0;
	u32 dev_file_id = (u32)private_data;

	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){


			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == enc->service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id == enc->session_id) {
							TDEBUG("enc cmd ses id %d \n",temp_ses->session_id);
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}

	if(!session_found) {
		TERR("session not found\n");
		ret = -EINVAL;
		goto return_func;
	}

	if(enc->encode_id != -1) {
		list_for_each_entry(enc_context,&temp_ses->encode_list, head) {
			if(enc_context->encode_id == enc->encode_id) {
				enc_found = 1;
				break;        
			}
		}
	}

	if(!enc_found) {
		enc_context = kmalloc(sizeof(otzc_encode), GFP_KERNEL);
		if(!enc_context) {
			TERR("kmalloc failed \n");
			ret = -ENOMEM;
			goto return_func;
		}
		enc_context->meta = kmalloc(sizeof(struct otzc_encode_meta ) * 
				(OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS),
				GFP_KERNEL);
		if(!enc_context->meta) {
			TERR("kmalloc failed \n");
			kfree(enc_context);
			ret = -ENOMEM;
			goto return_func;
		}
		enc_context->encode_id = (int)enc_context;
		enc->encode_id = enc_context->encode_id;
		enc_context->ker_req_data_addr = NULL;
		enc_context->ker_res_data_addr = NULL;
		enc_context->enc_req_offset = 0;
		enc_context->enc_res_offset = 0;
		enc_context->enc_req_pos = 0;
		enc_context->enc_res_pos = OTZ_MAX_REQ_PARAMS;
		enc_context->dec_res_pos = OTZ_MAX_REQ_PARAMS;
		enc_context->dec_offset = 0;
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
		enc_context->wait_data.send_wait_flag = 0; 
		init_waitqueue_head(&enc_context->wait_data.send_cmd_wq);
#endif
		list_add_tail(&enc_context->head, &temp_ses->encode_list);    
	}

	*penc_context = enc_context;
	*psession = temp_ses;

return_func:
	return ret;
}

/**
 * @brief 
 *
 * Funcion to encode uint32
 * 
 * @param private_data
 * @param argp
 * 
 * @return 
 */
static int otz_client_encode_uint32(void* private_data, void* argp)
{
	struct otz_client_encode_cmd enc;
	int ret = 0;
	otzc_session *session;
	otzc_encode *enc_context;


	if(copy_from_user(&enc, argp, sizeof(enc))) {
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_prepare_encode(private_data, &enc, &enc_context, &session);

	if(ret){
		goto return_func;
	}

	if(enc.param_type == OTZC_PARAM_IN) {
		if(!enc_context->ker_req_data_addr) {
			enc_context->ker_req_data_addr = 
				kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_req_data_addr) {
				TERR("kmalloc failed \n");
				ret =  -ENOMEM;
				goto ret_encode_u32;
			}
		}
		if( (enc_context->enc_req_offset + sizeof(u32) <= OTZ_1K_SIZE) &&
				(enc_context->enc_req_pos < OTZ_MAX_REQ_PARAMS)) {
			*(u32*)(enc_context->ker_req_data_addr + 
					enc_context->enc_req_offset) = *((u32*)enc.data);
			enc_context->enc_req_offset += sizeof(u32);
			enc_context->meta[enc_context->enc_req_pos].type 
				= OTZ_ENC_UINT32;
			enc_context->meta[enc_context->enc_req_pos].len = sizeof(u32);
			enc_context->enc_req_pos++;
		}
		else {
			ret =  -ENOMEM;/* Check this */
			goto ret_encode_u32;
		}
	}
	else if(enc.param_type == OTZC_PARAM_OUT) {
		if(!enc_context->ker_res_data_addr) {
			enc_context->ker_res_data_addr = 
				kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_res_data_addr) {
				TERR("kmalloc failed \n");
				ret = -ENOMEM;
				goto ret_encode_u32;
			}
		}
		if( (enc_context->enc_res_offset + sizeof(u32) <= OTZ_1K_SIZE) &&
				(enc_context->enc_res_pos < 
				 (OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS ))) {

			if(enc.data != NULL) {
				enc_context->meta[enc_context->enc_res_pos].usr_addr 
					= (u32)enc.data;
			}
			else {
				enc_context->meta[enc_context->enc_res_pos].usr_addr = 0;
			}
			enc_context->enc_res_offset += sizeof(u32);
			enc_context->meta[enc_context->enc_res_pos].type = OTZ_ENC_UINT32;
			enc_context->meta[enc_context->enc_res_pos].len = sizeof(u32);
			enc_context->enc_res_pos++;
		}
		else {
			ret =  -ENOMEM; /* check this */
			goto ret_encode_u32;
		}
	}


ret_encode_u32:
	if(copy_to_user(argp, &enc, sizeof(enc))){
		TERR("copy from user failed \n");
		return -EFAULT;
	}

return_func:
	return ret;
}

/**
 * @brief 
 *
 * Function to encode an array
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_encode_array(void* private_data, void* argp)
{
	struct otz_client_encode_cmd enc;
	int ret = 0;
	otzc_encode *enc_context;
	otzc_session *session;

	if(copy_from_user(&enc, argp, sizeof(enc))) {
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_prepare_encode(private_data, &enc, &enc_context, &session);

	if(ret){
		goto return_func;
	}
	TDEBUG("enc_id 0x%x\n",enc_context->encode_id);

	if(enc.param_type == OTZC_PARAM_IN) {
		if(!enc_context->ker_req_data_addr) {
			TDEBUG("allocate req data\n");
			enc_context->ker_req_data_addr = kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_req_data_addr) {
				TERR("kmalloc failed \n");
				ret = -ENOMEM;
				goto ret_encode_array;
			}
		}
		TDEBUG("append encode data\n");

		if((enc_context->enc_req_offset + enc.len <= OTZ_1K_SIZE) &&
				(enc_context->enc_req_pos < OTZ_MAX_REQ_PARAMS)) {
			if(copy_from_user(
						enc_context->ker_req_data_addr + enc_context->enc_req_offset, 
						enc.data , 
						enc.len)) {
				TERR("copy from user failed \n");
				ret = -EFAULT;
				goto ret_encode_array;
			}
			enc_context->enc_req_offset += enc.len;

			enc_context->meta[enc_context->enc_req_pos].type = OTZ_ENC_ARRAY;
			enc_context->meta[enc_context->enc_req_pos].len = enc.len;
			enc_context->enc_req_pos++;
		}
		else {
			ret = -ENOMEM; /* Check this */
			goto ret_encode_array;
		}
	}
	else if(enc.param_type == OTZC_PARAM_OUT) {
		if(!enc_context->ker_res_data_addr) {
			enc_context->ker_res_data_addr = kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_res_data_addr) {
				TERR("kmalloc failed \n");
				ret = -ENOMEM;
				goto ret_encode_array;
			}
		}
		if((enc_context->enc_res_offset + enc.len <= OTZ_1K_SIZE) &&
				(enc_context->enc_res_pos < 
				 (OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS ))) {
			if(enc.data != NULL) {
				enc_context->meta[enc_context->enc_res_pos].usr_addr 
					= (u32)enc.data;
			}
			else {
				enc_context->meta[enc_context->enc_res_pos].usr_addr = 0;
			}
			enc_context->enc_res_offset += enc.len;
			enc_context->meta[enc_context->enc_res_pos].type = OTZ_ENC_ARRAY;
			enc_context->meta[enc_context->enc_res_pos].len = enc.len;

			enc_context->enc_res_pos++;
		}
		else {
			ret = -ENOMEM;/* Check this */
			goto ret_encode_array;
		}
	}

ret_encode_array:
	if(copy_to_user(argp, &enc, sizeof(enc))){
		TERR("copy from user failed \n");
		return -EFAULT;
	}

return_func:
	return ret;
}


/**
 * @brief 
 *
 * Function to encode a memory reference (from kernel)
 *
 * @param private_data
 * @param argp
 *
 * @return 
 */
static int otz_client_kernel_encode_mem_ref(void *private_data, void *argp)
{


	int ret = 0, shared_mem_found = 0;
	otzc_encode *enc_context;
	otzc_session *session;
	otzc_shared_mem* temp_shared_mem;
	struct otz_client_encode_cmd *enc = (struct otz_client_encode_cmd *)argp;

	ret = otz_client_prepare_encode(private_data, enc, &enc_context, &session);

	if(ret){
		goto return_func;
	}
	list_for_each_entry(temp_shared_mem, &session->shared_mem_list,s_head){
		if(temp_shared_mem->index == (u32*)enc->data){
			shared_mem_found = 1;
			break;
		}
	}
	if(!shared_mem_found) {
		otzc_dev_file *temp_dev_file;
		list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
				head) {
			if(temp_dev_file->dev_file_id == (u32)private_data){
				break;
			}
		}
		list_for_each_entry(temp_shared_mem, 
				&temp_dev_file->dev_shared_mem_head.shared_mem_list ,head) {
			TDEBUG("dev id : %d shrd_mem_index : 0x%x\n",
					temp_dev_file->dev_file_id, temp_shared_mem->index);
			if(temp_shared_mem->index == (u32*)enc->data){
				shared_mem_found = 1;
				break;
			}
		}
	}

	if(!shared_mem_found) {

		TERR("shared memory not registered for \
				this session 0x%x\n", session->session_id);
		ret = -EINVAL;
		goto return_func;
	}
	if(enc->param_type == OTZC_PARAM_IN) {
		if(!enc_context->ker_req_data_addr) {
			enc_context->ker_req_data_addr = kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_req_data_addr) {
				TERR("kmalloc failed \n");
				ret = -ENOMEM;
				goto ret_encode_array;
			}
		}

		if((enc_context->enc_req_offset + sizeof(u32) <= 
					OTZ_1K_SIZE) &&
				(enc_context->enc_req_pos < OTZ_MAX_REQ_PARAMS)) {
			*((u32*)enc_context->ker_req_data_addr + 
					enc_context->enc_req_offset) 
				= virt_to_phys(temp_shared_mem->k_addr+enc->offset);
			enc_context->enc_req_offset += sizeof(u32);
			enc_context->meta[enc_context->enc_req_pos].usr_addr
				= (u32)(temp_shared_mem->u_addr + enc->offset);
			enc_context->meta[enc_context->enc_req_pos].type = OTZ_MEM_REF;
			enc_context->meta[enc_context->enc_req_pos].len = enc->len;

			enc_context->enc_req_pos++;
		}
		else {
			ret = -ENOMEM; /* Check this */
			goto ret_encode_array;
		}
	}
	else if(enc->param_type == OTZC_PARAM_OUT) {
		if(!enc_context->ker_res_data_addr) {
			enc_context->ker_res_data_addr = kmalloc(OTZ_1K_SIZE, GFP_KERNEL);
			if(!enc_context->ker_res_data_addr) {
				TERR("kmalloc failed \n");
				ret = -ENOMEM;
				goto ret_encode_array;
			}
		}
		if((enc_context->enc_res_offset + sizeof(u32)
					<= OTZ_1K_SIZE) &&
				(enc_context->enc_res_pos < 
				 (OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS ))) {
			*((u32*)enc_context->ker_res_data_addr + 
					enc_context->enc_res_offset) 
				= virt_to_phys(temp_shared_mem->k_addr + enc->offset);
			enc_context->enc_res_offset += sizeof(u32);
			enc_context->meta[enc_context->enc_res_pos].usr_addr
				= (u32)(temp_shared_mem->u_addr + enc->offset);
			enc_context->meta[enc_context->enc_res_pos].type 
				=  OTZ_MEM_REF;
			enc_context->meta[enc_context->enc_res_pos].len = enc->len;
			enc_context->enc_res_pos++;
		}
		else {
			ret = -ENOMEM; /*Check this */
			goto ret_encode_array;
		}
	}
ret_encode_array:

return_func:
	return ret;
}


/**
 * @brief 
 *
 * Function to encode a memory reference (from user application)
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_encode_mem_ref(void* private_data, void* argp)
{
	struct otz_client_encode_cmd enc;
	int ret = 0;


	if(copy_from_user(&enc, argp, sizeof(enc))) {
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}
	ret = otz_client_kernel_encode_mem_ref(private_data,&enc);
	if(enc.encode_id != -1){
		if(copy_to_user(argp, &enc, sizeof(enc))){
			TERR("copy from user failed \n");
			return -EFAULT;
		}
	}
return_func:
	return ret;
}

/**
 * @brief 
 *
 * This function prepares and initializes the decode context
 *
 * @param dec
 * @param pdec_context
 *
 * @return 
 */
static int otz_client_prepare_decode(void* private_data, 
		struct otz_client_encode_cmd *dec,
		otzc_encode **pdec_context) 
{
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	otzc_encode *dec_context;
	int  session_found = 0, enc_found = 0;
	int ret = 0;
	u32 dev_file_id = (u32)private_data;

	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == dec->service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list, 
							head) {
						if(temp_ses->session_id == dec->session_id) {
							TDEBUG("enc cmd ses id %d \n",temp_ses->session_id);
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}

	if(!session_found) {
		TERR("session not found\n");
		ret = -EINVAL;
		goto return_func;
	}

	if(dec->encode_id != -1) {
		list_for_each_entry(dec_context,&temp_ses->encode_list, head) {
			if(dec_context->encode_id == dec->encode_id){
				enc_found = 1;
				break;        
			}
		}
	}

	if(!enc_found) {
		ret =  -EINVAL;
		goto return_func;
	}

	*pdec_context = dec_context;
return_func:
	return ret;
}

/**
 * @brief 
 *
 * Function to decode uint32
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_decode_uint32(void* private_data, void* argp)
{
	struct otz_client_encode_cmd dec;
	int ret = 0;
	otzc_encode *dec_context;


	if(copy_from_user(&dec, argp, sizeof(dec))) {
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_prepare_decode(private_data, &dec, &dec_context);

	if(ret) {
		goto return_func;
	}

	if((dec_context->dec_res_pos <= dec_context->enc_res_pos) && 
			(dec_context->meta[dec_context->dec_res_pos].type
			 == OTZ_ENC_UINT32)){ 

		if(dec_context->meta[dec_context->dec_res_pos].usr_addr) {
			dec.data = 
				(void*)dec_context->meta[dec_context->dec_res_pos].usr_addr;
		}

		*(u32*)dec.data =  *((u32*)(dec_context->ker_res_data_addr
					+ dec_context->dec_offset));
		dec_context->dec_offset += sizeof(u32);
		dec_context->dec_res_pos++;
	}
	if(copy_to_user(argp, &dec, sizeof(dec))){
		TERR("copy to user failed \n");
		return -EFAULT;
	}

return_func:
	return ret;
}

static int otz_client_kernel_decode_array_space(void *private_data, void *argp) __attribute__((used));
/**
 * @brief 
 *
 * Function to decode an array
 *
 * @param private_data
 * @param argp
 *
 * @return 
 */
static int otz_client_kernel_decode_array_space(void *private_data, void *argp)
{
	struct otz_client_encode_cmd *dec = NULL;
	int ret = 0;
	otzc_encode *dec_context;
	dec = (struct otz_client_encode_cmd *)argp;

	ret = otz_client_prepare_decode(private_data, dec, &dec_context);
	if(ret){
		goto return_func;
	}
	if((dec_context->dec_res_pos <= dec_context->enc_res_pos) && 
			(dec_context->meta[dec_context->dec_res_pos].type 
			 == OTZ_MEM_REF)) {
		if (dec_context->meta[dec_context->dec_res_pos].len >=
				dec_context->meta[dec_context->dec_res_pos].ret_len) {
			dec->data = 
				(void*)dec_context->meta[dec_context->dec_res_pos].usr_addr;
		}
		else {
			TERR("buffer length is small. Length required %d \
					and supplied length %d\n", 
					dec_context->meta[dec_context->dec_res_pos].ret_len,
					dec_context->meta[dec_context->dec_res_pos].len);
			ret = -EFAULT;/* Check this */
			goto return_func;
		}

		dec->len = dec_context->meta[dec_context->dec_res_pos].ret_len;
		dec_context->dec_offset += sizeof(u32);
		dec_context->dec_res_pos++;
	}

	else {
		TERR("invalid data type or decoder at wrong position\n");
		ret = -EINVAL;
		goto return_func;
	}

return_func:
	return ret;


}

/**
 * @brief 
 *
 * Function to decode an array (from user application)
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_decode_array_space(void* private_data, void* argp)
{
	struct otz_client_encode_cmd dec;
	int ret = 0;
	otzc_encode *dec_context;


	if(copy_from_user(&dec, argp, sizeof(dec))) {
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_prepare_decode(private_data, &dec, &dec_context);

	if(ret){
		goto return_func;
	}

	if((dec_context->dec_res_pos <= dec_context->enc_res_pos) && 
			(dec_context->meta[dec_context->dec_res_pos].type 
			 == OTZ_ENC_ARRAY)) {
		if (dec_context->meta[dec_context->dec_res_pos].len >=
				dec_context->meta[dec_context->dec_res_pos].ret_len) {
			if(dec_context->meta[dec_context->dec_res_pos].usr_addr) {
				dec.data = 
					(void*)dec_context->meta[dec_context->dec_res_pos].usr_addr;
			}
			if(copy_to_user(dec.data, 
						dec_context->ker_res_data_addr + dec_context->dec_offset,
						dec_context->meta[dec_context->dec_res_pos].ret_len)){
				TERR("copy from user failed while copying array\n");
				ret = -EFAULT;
				goto return_func;
			} 
		}
		else {
			TERR("buffer length is small. Length required %d \
					and supplied length %d\n", 
					dec_context->meta[dec_context->dec_res_pos].ret_len,
					dec_context->meta[dec_context->dec_res_pos].len);
			ret = -EFAULT; /* check this */
			goto return_func;
		}

		dec.len = dec_context->meta[dec_context->dec_res_pos].ret_len;
		dec_context->dec_offset +=  
			dec_context->meta[dec_context->dec_res_pos].len;
		dec_context->dec_res_pos++;
	}
	else if((dec_context->dec_res_pos <= dec_context->enc_res_pos) && 
			(dec_context->meta[dec_context->dec_res_pos].type 
			 == OTZ_MEM_REF)) {
		if (dec_context->meta[dec_context->dec_res_pos].len >=
				dec_context->meta[dec_context->dec_res_pos].ret_len) {
			dec.data = 
				(void*)dec_context->meta[dec_context->dec_res_pos].usr_addr;
		}
		else {
			TERR("buffer length is small. Length required %d \
					and supplied length %d\n", 
					dec_context->meta[dec_context->dec_res_pos].ret_len,
					dec_context->meta[dec_context->dec_res_pos].len);
			ret = -EFAULT;/* Check this */
			goto return_func;
		}

		dec.len = dec_context->meta[dec_context->dec_res_pos].ret_len;
		dec_context->dec_offset += sizeof(u32);
		dec_context->dec_res_pos++;
	}

	else {
		TERR("invalid data type or decoder at wrong position\n");
		ret = -EINVAL;
		goto return_func;
	}

	if(copy_to_user(argp, &dec, sizeof(dec))){
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

return_func:
	return ret;
}

/**
 * @brief 
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_get_decode_type(void* private_data, void* argp)
{
	struct otz_client_encode_cmd dec;
	int ret = 0;
	otzc_encode *dec_context;


	if(copy_from_user(&dec, argp, sizeof(dec))){
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

	ret = otz_client_prepare_decode(private_data, &dec, &dec_context);

	if(ret){
		goto return_func;
	}

	TDEBUG("decoder pos 0x%x and encoder pos 0x%x\n",
			dec_context->dec_res_pos, dec_context->enc_res_pos);

	if(dec_context->dec_res_pos <= dec_context->enc_res_pos) 
		dec.data = (void*)dec_context->meta[dec_context->dec_res_pos].type;
	else {
		ret = -EINVAL; /* check this */
		goto return_func;
	}

	if(copy_to_user(argp, &dec, sizeof(dec))){
		TERR("copy to user failed \n");
		ret = -EFAULT;
		goto return_func;
	}

return_func:
	return ret;
}

static int otz_client_kernel_shared_mem_alloc(void *private_data, void *argp,
		struct otzc_shared_mem *sh_mem) __attribute__((used));
/**
 * @brief 
 *
 * @param private_data
 * @param argp
 * @param sh_mem
 *
 * @return 
 */
static int otz_client_kernel_shared_mem_alloc(void *private_data, void *argp,
		struct otzc_shared_mem *sh_mem)
{

	struct otz_session_shared_mem_info *mem_info;
	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	otzc_shared_mem* temp_shared_mem = sh_mem;
	int  session_found = 0;
	int ret = 0;
	u32 dev_file_id = (u32)private_data;
	mem_info = (struct otz_session_shared_mem_info *)argp;
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == mem_info->service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id ==
								mem_info->session_id) {
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}

	if(!session_found){
		TERR("Session not found!!\n");
		ret = -1;
		return ret;
	}
	/* If sessions is found..add the shared mem region to the session
	 * specific list */


	list_add_tail( &temp_shared_mem->s_head ,
			&temp_ses->shared_mem_list);
	return ret;

}

/**
 * @brief 
 *
 * Registers the shared memory from the device list to the session list. This is
 * because when we mmap, we cannot specify the session to which the memory has
 * to be mapped because the parameters do not allow us. So during mmap,
 * the memory is mapped to the device. Here, the memory which was mapped to
 * the device is mapped to the session shared memory list.
 *
 * @param argp
 *
 * @return 
 */
static int otz_client_shared_mem_alloc(void* private_data, void* argp)
{
	struct otz_session_shared_mem_info mem_info;
	int ret = 0;

	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;
	otzc_shared_mem* temp_shared_mem;
	int  session_found = 0;
	u32 dev_file_id = (u32)private_data;
	if(copy_from_user(&mem_info, argp, sizeof(mem_info))){
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}    


	TDEBUG("service id 0x%x session id 0x%x user mem addr 0x%x \n", 
			mem_info.service_id,
			mem_info.session_id,
			mem_info.user_mem_addr);



	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == mem_info.service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id ==
								mem_info.session_id) {
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}

	if(!session_found) {
		TERR("session not found\n");
		ret = -EINVAL;
		goto return_func;
	}

	list_for_each_entry(temp_shared_mem,
			&temp_dev_file->dev_shared_mem_head.shared_mem_list , head){
		if(temp_shared_mem->index == (u32*)mem_info.user_mem_addr){
			list_del(&temp_shared_mem->head);
			temp_dev_file->dev_shared_mem_head.shared_mem_cnt--;
			list_add_tail( &temp_shared_mem->s_head ,
					&temp_ses->shared_mem_list);
			break;
		}
	}
return_func:
	return ret;
}

/**
 * @brief 
 *
 * Frees the shared memory for a particular session for the device
 *
 * @param private_data - Contains the device file ID
 * @param argp - Contains shared memory information for the session
 *
 * @return 
 */
static int otz_client_shared_mem_free(void* private_data, void* argp)
{
	otzc_shared_mem* temp_shared_mem;
	struct otz_session_shared_mem_info mem_info;

	otzc_dev_file *temp_dev_file;
	otzc_service *temp_svc;
	otzc_session *temp_ses;

	int  session_found = 0;
	int ret = 0;
	u32 dev_file_id = (u32)private_data;

	if(copy_from_user(&mem_info, argp, sizeof(mem_info))){
		TERR("copy from user failed \n");
		ret = -EFAULT;
		goto return_func;
	}     

	TDEBUG("service id 0x%x session id 0x%x user mem addr 0x%x \n", 
			mem_info.service_id,
			mem_info.session_id,
			mem_info.user_mem_addr);
	list_for_each_entry(temp_dev_file, &otzc_dev_file_head.dev_file_list,
			head) {
		if(temp_dev_file->dev_file_id == dev_file_id){

			list_for_each_entry(temp_svc, &temp_dev_file->services_list, head) {
				if( temp_svc->service_id == mem_info.service_id) {
					list_for_each_entry(temp_ses, &temp_svc->sessions_list,
							head) {
						if(temp_ses->session_id == mem_info.session_id) {
							session_found = 1;
							break;        
						}
					}
					break;
				}
			}
			break;
		}
	}

	if(!session_found) {
		TERR("session not found\n");
		ret = -EINVAL;
		goto return_func;
	}

	list_for_each_entry(temp_shared_mem, &temp_ses->shared_mem_list,s_head){
		if(temp_shared_mem->index == (u32*)mem_info.user_mem_addr){
			list_del(&temp_shared_mem->s_head);

			if(temp_shared_mem->k_addr)
				free_pages((u32)temp_shared_mem->k_addr,
						get_order(ROUND_UP(temp_shared_mem->len, SZ_4K)));

			if(temp_shared_mem)
				kfree(temp_shared_mem);            
			break;
		}
	}
return_func:
	return ret;
}
#ifdef CONFIG_KIM
/**
 * @brief 
 *
 * This function initializes the encode structure specifically
 * for the kernel integrity manager.
 *
 * @param im
 * @param size
 * @param param_type
 * @param flag
 * @param data
 *
 * @return 
 */
struct otz_client_encode_cmd *prep_enc_struct(struct otz_client_im_check *im,
		unsigned int size,unsigned int param_type,unsigned int flag,
		void *data)
{
	struct otz_client_encode_cmd *enc = NULL;
	enc = (struct otz_client_encode_cmd *)kmalloc(sizeof(struct
				otz_client_encode_cmd),GFP_KERNEL);
	if(!enc)
		return NULL;

	enc->encode_id = im->encode_id;
	enc->session_id = im->session_id;
	enc->cmd_id = im->cmd_id;
	enc->service_id = im->service_id;
	enc->data = NULL;
	enc->data = (void*) __get_free_pages(GFP_KERNEL,
			get_order(ROUND_UP(size, SZ_4K)));

	if(!(enc->data)){
		TERR("Unable to allocate space\n");
		return NULL;
	}
	memcpy(enc->data,(void *)data,size);
	enc->flags = flag;
	enc->offset = 0;
	enc->param_type = param_type;
	enc->len = size;

	return enc;

}
/**
 * @brief 
 *
 * @param file
 * @param enc
 *
 * @return 
 */
static int otz_client_enc(struct file *file , struct otz_client_encode_cmd *enc)
{
	int ret;
	/* encode the data */
	mutex_lock(&encode_cmd_lock);
	ret = otz_client_kernel_encode_mem_ref(file->private_data,enc);
	mutex_unlock(&encode_cmd_lock);
	if (ret){
		TDEBUG("failed otz_client_encode_cmd: %d", ret);
		return -1;
	}
	return 0;
}
/**
 * @brief 
 *
 * The Kernel Integriy Manager is not initiated from the non-secure world,
 * so this function helps to create an encode context because we are calling it
 * from the secure world itself.
 *
 * @param file
 * @param im
 * @param size
 * @param flag
 * @param param_type
 * @param data
 *
 * @return 
 */
static struct otz_client_encode_cmd * encode_helper(struct file *file,
		struct otz_client_im_check *im , unsigned int size,
		unsigned int flag,unsigned int param_type,void *data)
{

	otzc_shared_mem *mem_new = NULL;
	struct otz_session_shared_mem_info mem_info;
	struct otz_client_encode_cmd *enc = NULL;
	int ret = 0;
	enc = prep_enc_struct(im,size,flag,param_type,(void *)data);
	TDEBUG("service id is %d and cmd id is"
			"%d session id is"
			"%d\n",enc->service_id,enc->cmd_id,enc->session_id);
	if(!enc){
		TERR("failed to prep_enc_struct\n");
		return NULL;
	}
	mem_new = (otzc_shared_mem
			*)kmalloc(sizeof(otzc_shared_mem),GFP_KERNEL);
	if(!mem_new){
		TERR("Insufficient memory\n");
		return NULL;
	}
	mem_new->k_addr = enc->data;
	mem_new->len = size;
	mem_new->u_addr = enc->data;
	mem_new->index = enc->data;
	/* Add to session shared memory list */

	mem_info.service_id = enc->service_id;
	mem_info.session_id = enc->session_id;
	mem_info.user_mem_addr = (unsigned int)enc->data;
	mutex_lock(&mem_alloc_lock);
	ret = otz_client_kernel_shared_mem_alloc((void *)file->private_data,
			&mem_info, mem_new);
	mutex_unlock(&mem_alloc_lock);

	ret = otz_client_enc(file,enc);
	if(ret == -1){
		TERR("otz_client_enc failed\n");
		return NULL;
	}


	return enc;
}


/**
 * @brief Function to open file.
 *
 * @param path
 * @param flags
 * @param rights
 *
 * @return 
 */
struct file* my_file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	TDEBUG("File path is: %s\n",path);
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if(IS_ERR(filp)) {
		err = PTR_ERR(filp);
		TERR("************File not able to open!! *************** %d\n",err);
		return NULL;
	}
	return filp;
}

/**
 * @brief Function to seek file (Similar to lseek)
 *
 * @param file
 * @param offset
 * @param origin
 *
 * @return 
 */
int my_file_seek(struct file *file,unsigned long long offset, int origin)
{
	mm_segment_t oldfs;
	int ret;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	ret = vfs_llseek(file,offset,origin);
	vfs_llseek(file,offset,0);
	set_fs(oldfs); 
	return ret;
}
/**
 * @brief Function to read file
 *
 * @param file
 * @param offset
 * @param data
 * @param size
 *
 * @return 
 */
int my_file_read(struct file* file, unsigned long long offset,
		unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	//  set_fs(get_ds());
	set_fs(KERNEL_DS);
	ret = vfs_read(file, data, size, &file->f_pos);
	set_fs(oldfs);
	return ret;
} 

/**
 * @brief Function to close file
 *
 * @param file
 */
void my_file_close(struct file* file) {
	filp_close(file, NULL);
}


/**
 * @brief Function to get file information and return a file_struct structure.
 *
 * @param file
 *
 * @return 
 */
file_type* get_file_info(char* file)
{
	int fd,i,j,t;
	int file_size,stat=0;
	struct kstat buf;
	file_type *my_file = NULL;
	char* file_content = NULL;
	char* hash;

	int offset =0;
	struct file *fp;
	my_file = kmalloc(sizeof(file_type),GFP_KERNEL);
	if(!my_file)
		return NULL;

	fp = my_file_open(file,O_RDONLY,0);
	if(!fp){
		TERR("error in file_open");
		return NULL;
	}
	file_size = my_file_seek(fp,0,2);
	if(file_size<1)
	{
		TERR("File is empty");
		return NULL;
	}
	TDEBUG("file size = %d\n",file_size);
	file_content = (char *)vmalloc((file_size+1));
	if(!file_content){
		TERR("Unable to allocate space\n");
		return NULL;
	}

	if(my_file_read(fp,0,file_content,file_size))
	{
		stat=1;
	}
	if(stat)
	{
		strcpy(my_file->hash,find_md5_hash(file_content));
		my_file->file_reported = 0;
		strcpy(my_file->fullpath,file);
	}
	else
	{
		TERR("Read failed");
		return NULL;
	}
	my_file_close(fp);
	return my_file;
}

static int get_kernel_text_hash(void)
{
	char *kernel_text_start = (char *)KERN_TEXT_START;
	char *buffer = NULL;
	char *hash = NULL;
	buffer = kmalloc(KERN_TEXT_SIZE + 1 , GFP_KERNEL);
	if(!buffer){
		TERR("Unable to contain the kernel's .text section..trying vmalloc()\n");
		/* TODO:vmalloc() is slow, use chunks for kmalloc() allocations instead
		 * */
		buffer = vmalloc(KERN_TEXT_SIZE + 1);
		if(!buffer){
			TERR("kernel .text section allocation failed through vmalloc()\n");
			return NULL;
		}
	}
	memcpy(buffer,kernel_text_start,KERN_TEXT_SIZE);
	buffer[KERN_TEXT_SIZE] = 0x0;
	hash = find_md5_hash(buffer);
	return hash;

}


static int check_kernel_integrity(struct file *file,
		struct otz_client_im_check *im)
{
	char hash[33];
	char *hash_ptr;
	unsigned int size = sizeof(hash);
	int ret = 0;
	struct otz_client_encode_cmd *enc = NULL;
	hash_ptr = get_kernel_text_hash();
	strncpy(hash,hash_ptr,33);	
	enc = encode_helper(file,im,size,0x0,OTZC_PARAM_IN,(void *)hash);
	if(enc == NULL){
		TERR("failed in encode_data\n");
		ret = -1;
		goto ret_func;
	}
	/* Encode  again since we use the same input buffer for both
	 * request and response */
	enc->flags = 1;
	enc->param_type = 1;
	ret = otz_client_enc(file,enc);
	if(ret == -1){
		TERR("failed to encode data again\n");
		ret = -1;
		goto ret_func;
	}
	/* send data to secure world */
	mutex_lock(&send_cmd_lock);
	ret = otz_client_kernel_send_cmd(file->private_data, enc);
	mutex_unlock(&send_cmd_lock);

	if (ret){
		TDEBUG("failed otz_client_send_cmd: %d", ret);
		ret = -1;
		goto ret_func;
	}

	/* Decode the response and  finally call release */
	mutex_lock(&decode_cmd_lock);
	ret = otz_client_kernel_decode_array_space(file->private_data,
			enc);
	mutex_unlock(&decode_cmd_lock);
	if (ret){
		TDEBUG("failed otz_client_decode_cmd: %d", ret);
		ret = -1;
		goto ret_func;
	}

	ret = otz_client_kernel_operation_release(file->private_data,
			enc);
	if (ret){
		TERR("failed operation release: %d", ret);
		ret = -1;
		goto ret_func;
	}

	if(enc){
		kfree(enc);
		enc = NULL;
	}

ret_func:
	return ret;
}

/**
 * @brief The integrity manager helper function.
 *
 * This determines whether the integrity check function was successfully
 * carried out or not.
 *
 * @param file
 * @param argp
 *
 * @return 
 */

static long otz_client_im_helper(struct file *file, void *argp)
{

	struct otz_client_encode_cmd *enc = NULL;
	int ret = 0;
	struct otz_client_im_check *im = (struct otz_client_im_check *)argp;
	ret = check_kernel_integrity(file,im);	
	if(ret == -1){
		TERR("IM command failed\n");
		ret = -1;
		goto ret_func;
	}
ret_func:
	return ret;

}
#endif

/**
 * @brief Function which resolves the ioctl ID's and carries out
 * 			the corressponding task.
 *
 * @param file
 * @param cmd
 * @param arg
 *
 * @return 
 */
static long otz_client_ioctl(struct file *file, unsigned cmd,
		unsigned long arg)
{
	int ret = -EINVAL;
	void *argp = (void __user *) arg;

	switch (cmd) {
		case OTZ_CLIENT_IOCTL_SEND_CMD_REQ: 
			/* Only one client allowed here at a time */
			mutex_lock(&send_cmd_lock);
			ret = otz_client_send_cmd(file->private_data, argp);
			mutex_unlock(&send_cmd_lock);

			if (ret)
				TDEBUG("failed otz_client_send_cmd: %d", ret);
			break;
#ifdef CONFIG_KIM
		case OTZ_CLIENT_IOCTL_IM_CHECK:	{
			mutex_lock(im_check_lock);
			ret = otz_client_im_helper(file,
					argp);
			mutex_unlock(im_check_lock);
			if(ret == -1) {
				TERR("failed IM_CHECK\n");
			}
			return ret;
		}
#endif
		case OTZ_CLIENT_IOCTL_ENC_UINT32: {
			/* Only one client allowed here
			* at a time */
			mutex_lock(&encode_cmd_lock);
			ret = otz_client_encode_uint32(file->private_data, argp);
			mutex_unlock(&encode_cmd_lock);
			if (ret)
			  TDEBUG("failed otz_client_encode_cmd: %d", ret);
			break;
		}

		case OTZ_CLIENT_IOCTL_DEC_UINT32: {
			/* Only one client allowed here
			* at a time */
			mutex_lock(&decode_cmd_lock);
			ret = otz_client_decode_uint32(file->private_data, argp);
			mutex_unlock(&decode_cmd_lock);
			if (ret)
			  TDEBUG("failed otz_client_decode_cmd: %d", ret);
			break;
		}

		case OTZ_CLIENT_IOCTL_ENC_ARRAY: {
			/* Only one client allowed here
			* at a time */
			mutex_lock(&encode_cmd_lock);
			ret = otz_client_encode_array(file->private_data, argp);
			mutex_unlock(&encode_cmd_lock);
			if (ret)
			 TDEBUG("failed otz_client_encode_cmd: %d", ret);
			break;
		}
		case OTZ_CLIENT_IOCTL_DEC_ARRAY_SPACE: {
			/* Only one client allowed
			* here at a time */
			mutex_lock(&decode_cmd_lock);
			ret = 
			   otz_client_decode_array_space(file->private_data, argp);
			mutex_unlock(&decode_cmd_lock);
			if (ret)
			   TDEBUG("failed otz_client_decode_cmd: %d", ret);
			break;
		}
		case OTZ_CLIENT_IOCTL_ENC_MEM_REF: {
			/* Only one client allowed here at a time */
			mutex_lock(&encode_cmd_lock);
			ret = otz_client_encode_mem_ref(file->private_data, argp);
			mutex_unlock(&encode_cmd_lock);
			if (ret)
			   TDEBUG("failed otz_client_encode_cmd: %d", ret);
			break;
		}
		case OTZ_CLIENT_IOCTL_ENC_ARRAY_SPACE: {
			/* Only one client allowed here at a time */
			mutex_lock(&encode_cmd_lock);
			ret = otz_client_encode_mem_ref(file->private_data, argp);
			mutex_unlock(&encode_cmd_lock);
			if (ret)
			   TDEBUG("failed otz_client_encode_cmd: %d", ret);
			break;
		}
		case OTZ_CLIENT_IOCTL_GET_DECODE_TYPE: {
		   /* Only one client allowed here at a time */
		   mutex_lock(&decode_cmd_lock);
		   ret = otz_client_get_decode_type(file->private_data, argp);
		   mutex_unlock(&decode_cmd_lock);
		   if (ret)
			   TDEBUG("failed otz_client_decode_cmd: %d", ret);
		   break;
		}
		case OTZ_CLIENT_IOCTL_SES_OPEN_REQ: {
			/* Only one client allowed here at a time */
			mutex_lock(&ses_open_lock);
			ret = otz_client_session_open(file->private_data, argp);
			mutex_unlock(&ses_open_lock);
			if (ret)
				TERR("failed otz_client_session_open: %d", ret);
			break;
		}
		case OTZ_CLIENT_IOCTL_SES_CLOSE_REQ:{
			/* Only one client allowed here at a time */
			mutex_lock(&ses_close_lock);
			ret = otz_client_session_close(file->private_data, argp);
			mutex_unlock(&ses_close_lock);
			if (ret)
				TDEBUG("failed otz_client_session_close: %d", ret);
			break;
		}
		case OTZ_CLIENT_IOCTL_SHR_MEM_ALLOCATE_REQ: {
			/* Only one client allowed here at a time */
			mutex_lock(&mem_alloc_lock);
			ret = otz_client_shared_mem_alloc(file->private_data, argp);
			mutex_unlock(&mem_alloc_lock);
			if (ret)
				TDEBUG("failed otz_client_shared_mem_alloc: %d", ret);
			break;
		}
		case OTZ_CLIENT_IOCTL_SHR_MEM_FREE_REQ: {
			/* Only one client allowed here at a time */
			mutex_lock(&mem_free_lock);
			ret = otz_client_shared_mem_free(file->private_data, argp);
			mutex_unlock(&mem_free_lock);
			if (ret)
			TDEBUG("failed otz_client_shared_mem_free: %d", ret);
			break;
		}
		case OTZ_CLIENT_IOCTL_OPERATION_RELEASE: {
			ret = otz_client_operation_release(file->private_data, argp);
			if (ret)
			 TDEBUG("failed operation release: %d", ret);
			break;
		}
		default:
			return -EINVAL;
	}
	return ret;
}

/**
 * @brief 
 *
 * @param inode
 * @param file
 *
 * @return 
 */
static int otz_client_open(struct inode *inode, struct file *file)
{
	int ret;
	otzc_dev_file *new_dev;

	device_file_cnt++;
	file->private_data = (void*)device_file_cnt;

	new_dev = (otzc_dev_file*)kmalloc(sizeof(otzc_dev_file), GFP_KERNEL);
	if(!new_dev){
		TERR("kmalloc failed for new dev file allocation\n");
		ret = -ENOMEM;
		goto ret_func;
	}
	new_dev->dev_file_id = device_file_cnt;
	new_dev->service_cnt = 0;
	INIT_LIST_HEAD(&new_dev->services_list);

	memset(&new_dev->dev_shared_mem_head, 0, sizeof(otzc_shared_mem_head));
	new_dev->dev_shared_mem_head.shared_mem_cnt = 0;
	INIT_LIST_HEAD(&new_dev->dev_shared_mem_head.shared_mem_list);


	list_add(&new_dev->head, &otzc_dev_file_head.dev_file_list);
	otzc_dev_file_head.dev_file_cnt++;

	if((ret = otz_client_service_init(new_dev, OTZ_SVC_GLOBAL)) != 0) {
		goto ret_func;
	} else if((ret = otz_client_service_init(new_dev, OTZ_SVC_ECHO)) != 0) {
		goto ret_func;
	} else if((ret = otz_client_service_init(new_dev,
					OTZ_SVC_TEST_SUITE_USER)) != 0) {
		goto ret_func;
	} else if((ret = otz_client_service_init(new_dev,
					OTZ_SVC_CRYPT)) != 0) {
		goto ret_func;
	} else if((ret = otz_client_service_init(new_dev,
					OTZ_SVC_MUTEX_TEST)) != 0) {
		goto ret_func;
	} else if((ret = otz_client_service_init(new_dev,
					OTZ_SVC_VIRTUAL_KEYBOARD)) != 0) {
		goto ret_func;
	} else if((ret = otz_client_service_init(new_dev, OTZ_SVC_DRM)) != 0) {
		goto ret_func;
	} else if((ret = otz_client_service_init(new_dev,
					OTZ_SVC_GP_INTERNAL)) != 0) {
		goto ret_func;
	} else if((ret = otz_client_service_init(new_dev,
					OTZ_SVC_TEST_SUITE_KERNEL)) != 0) {
		goto ret_func;
	}
#ifdef CONFIG_KIM
	else if((ret = otz_client_service_init(new_dev,
					OTZ_SVC_KERNEL_INTEGRITY_CHECK)) != 0) {
		goto ret_func;
	}
#endif
#ifdef CONFIG_FFMPEG
	else if((ret = otz_client_service_init(new_dev,
					OTZ_SVC_FFMPEG_TEST)) != 0) {
		goto ret_func;
	}
#endif

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	if(!notify_data){
		notify_data = (struct otzc_notify_data*)kmalloc(
				sizeof(struct otzc_notify_data),
				GFP_KERNEL);
		if(!notify_data){
			TERR("kmalloc failed for notification data\n");
			ret = -ENOMEM;
			goto ret_func;
		}
	}


	ret = otz_smc_call(new_dev->dev_file_id, OTZ_SVC_GLOBAL, 
			OTZ_GLOBAL_CMD_ID_REGISTER_NOTIFY_MEMORY, 
			0, 0,
			notify_data, sizeof(struct otzc_notify_data), NULL, 
			0, NULL, NULL, NULL, NULL);      

	if(ret != SMC_SUCCESS) {
		TERR("Shared memory registration for \
				secure world notification failed\n");
		goto ret_func;
	}
	current_guest_id = notify_data->guest_no;
#endif
ret_func:
	return ret;
}

/**
 * @brief 
 *
 * @param inode
 * @param file
 *
 * @return 
 */
static int otz_client_release(struct inode *inode, struct file *file)
{
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	u32 dev_file_id = (u32)file->private_data;
	int ret;
	ret = otz_smc_call(dev_file_id, OTZ_SVC_GLOBAL, 
			OTZ_GLOBAL_CMD_ID_UNREGISTER_NOTIFY_MEMORY, 
			0, 0,
			NULL, 0, NULL, 
			0, NULL, NULL, NULL, NULL);      

	if(ret != SMC_SUCCESS) {
		TERR("Shared memory un-registration for \
				secure world notification failed\n");
	}

#endif

	TDEBUG("otz_client_release\n");
	otz_client_service_exit(file->private_data);
	if(list_empty(&otzc_dev_file_head.dev_file_list)){
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
		kfree(notify_data);
		notify_data = NULL;
#endif
	}
	return 0;
}

/**
 * @brief 
 *
 * @return 
 */
static int otz_client_smc_init(void)
{
	u32 ctr;

	asm volatile("mrc p15, 0, %0, c0, c0, 1" : "=r" (ctr));
	cacheline_size =  4 << ((ctr >> 16) & 0xf);

	return 0;
}


/**
 * @brief 
 */
static const struct file_operations otz_client_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = otz_client_ioctl,
	.open = otz_client_open,
	.mmap = otz_client_mmap,
	.release = otz_client_release
};

/**
 * @brief 
 *
 * @return 
 */
static int otz_client_init(void)
{
	int ret_code = 0;
	struct device *class_dev;

	TDEBUG("open otzclient init");
	otz_client_smc_init();

	ret_code = alloc_chrdev_region(&otz_client_device_no, 0, 1,
			OTZ_CLIENT_DEV);
	if (ret_code < 0) {
		TERR("alloc_chrdev_region failed %d", ret_code);
		return ret_code;
	}

	driver_class = class_create(THIS_MODULE, OTZ_CLIENT_DEV);
	if (IS_ERR(driver_class)) {
		ret_code = -ENOMEM;
		TERR("class_create failed %d", ret_code);
		goto unregister_chrdev_region;
	}

	class_dev = device_create(driver_class, NULL, otz_client_device_no, NULL,
			OTZ_CLIENT_DEV);
	if (!class_dev) {
		TERR("class_device_create failed %d", ret_code);
		ret_code = -ENOMEM;
		goto class_destroy;
	}

	cdev_init(&otz_client_cdev, &otz_client_fops);
	otz_client_cdev.owner = THIS_MODULE;

	ret_code = cdev_add(&otz_client_cdev,
			MKDEV(MAJOR(otz_client_device_no), 0), 1);
	if (ret_code < 0) {
		TERR("cdev_add failed %d", ret_code);
		goto class_device_destroy;
	}

	/* Initialize structure for services and sessions*/
	TDEBUG("Initializing list for services\n");
	memset(&otzc_dev_file_head, 0, sizeof(otzc_dev_file_head));
	otzc_dev_file_head.dev_file_cnt = 0;
	INIT_LIST_HEAD(&otzc_dev_file_head.dev_file_list);

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	register_secure_notify_handler(ipi_secure_notify);
#endif
	goto return_fn;

class_device_destroy:
	device_destroy(driver_class, otz_client_device_no);
class_destroy:
	class_destroy(driver_class);
unregister_chrdev_region:
	unregister_chrdev_region(otz_client_device_no, 1);
return_fn:
	return ret_code;
}

/**
 * @brief 
 */
static void otz_client_exit(void)
{
	TDEBUG("otz_client exit");

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	unregister_secure_notify_handler();
#endif
	device_destroy(driver_class, otz_client_device_no);
	class_destroy(driver_class);
	unregister_chrdev_region(otz_client_device_no, 1);
}


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Test  <sierraware.org>");
MODULE_DESCRIPTION("Sierraware TrustZone Communicator");
MODULE_VERSION("1.00");

module_init(otz_client_init);

module_exit(otz_client_exit);
