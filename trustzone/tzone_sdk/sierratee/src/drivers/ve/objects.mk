drivers-objs-y= pl011_uart.o
drivers-objs-y+= board_mmu.o
drivers-objs-y+= sp804_timer.o
drivers-objs-y+= board.o
drivers-objs-y+= board_config.o
drivers-objs-y+= board_test.o
drivers-objs-$(CONFIG_GUI_SUPPORT)+= sw_fb.o

drivers-svisor-objs-y= pl011_uart.o
drivers-svisor-objs-y+= svisor_board.o


