cpu-objs-y= cpu_start.o
cpu-objs-$(CONFIG_SW_DEDICATED_TEE)+= dtee_cpu_entry.o
cpu-objs-y+= main_secure.o
cpu-objs-y+= cache.o
cpu-objs-y+= sw_mmu_helper.o
cpu-objs-y+= cpu_data.o
cpu-objs-y+= exception_handlers.o
cpu-objs-y+= page_table.o
cpu-objs-y+= monitor.o
cpu-objs-$(CONFIG_SW_DEDICATED_TEE)+= dtee_monitor.o
cpu-objs-y+= cpu_task.o
cpu-objs-y+= cpu_task_context.o
cpu-objs-y+= smc_wrapper.o
cpu-objs-y+= cpu_api.o
cpu-objs-y+= cpu_common_api.o
cpu-objs-y+= cpu_ipi.o
cpu-objs-y+= sw_semaphores_asm.o
cpu-objs-y+= nsk_loader.o
cpu-objs-y+= tzhyp.o
cpu-objs-y+= tzhyp_boot.o
cpu-objs-y+= tzhyp_sched.o
cpu-objs-y+= tzhyp_switch.o
cpu-objs-y+= tzhyp_devices.o
cpu-objs-y+= sw_cpu_helper.o
cpu-objs-y+= sw_syscall_imp.o
cpu-objs-y+= static_load.o
cpu-objs-y+= mmu.o

ulib-cpu-objs-y = sw_syscalls.o
ulib-cpu-objs-y+= cpu_common_api.o

cpu-svisor-objs-y+= sw_cpu_helper.o
cpu-svisor-objs-y+= cache.o
#cpu-svisor-objs-y+= svisor_sched.o
cpu-svisor-objs-y+= sw_semaphores_asm.o

cpu-boot-objs-y=cpu_boot_entry.o
cpu-boot-objs-y+=secure_boot.o
