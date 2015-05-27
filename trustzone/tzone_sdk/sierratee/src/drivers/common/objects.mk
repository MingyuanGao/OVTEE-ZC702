drivers-common-objs-y= sw_gic.o
drivers-common-objs-y+= uart_driver.o
#drivers-common-objs-y+= sp804_sleep_timer.o
drivers-common-objs-$(CONFIG_CACHE_L2X0) += sw_l2_pl310.o
drivers-common-objs-$(CONFIG_GUI_SUPPORT) += fb_driver.o
drivers-svisor-common-objs-y= gic_svisor.o
drivers-svisor-common-objs-y+= gic_virt.o
