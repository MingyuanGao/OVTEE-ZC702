core-objs-y = task.o
core-objs-y += global.o
core-objs-y += secure_api.o
core-objs-y += mem_mng.o
core-objs-y += sw_modinit.o
core-objs-y += sw_timer.o
core-objs-y += scheduler.o
core-objs-$(CONFIG_SHELL)+= shell_command_manager.o

core-svisor-objs-y = mem_mng.o
core-svisor-objs-y += task_dummy.o