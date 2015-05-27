drivers-objs-y+= board_mmu.o
drivers-objs-y+= xilinx_uart.o
drivers-objs-y+= xilinx_timer.o
drivers-objs-y+= board.o
drivers-objs-y+= board_test.o
drivers-objs-y+= board_config.o
drivers-objs-$(CONFIG_SW_MULTICORE)+= sec_cpu_entry.o

