# master make file for Trustzone sdk kit

###
#
#CONFIG_NEON_SUPPORT:
#	Enables Neon support for executing neon based intrinsics and the instructions.
#
#TIMER_NOT_DEFINED
#	Disables the secure timer support.
#
#CONFIG_SW_MULTICORE:
#	Enables support for booting SMP kernels(Linux SMP for example)
#	in non secure world. 
#	Number of cores to be enabled is selected by MAX_CORES macro.
#
#CONFIG_SW_NOBOOTLOADER:
#	Enables support for directly running the secure world OS on
#	target without running a bootloader. A typical example is
#	fastmodel emulation targets. Currently this is used to handle
#	the multicore booting.
#
#CONFIG_CACHE_L2X0
#	Enables the L2 cache controller support in Secure OS. 
#	Currently it is to co-operate to work along with guest. 
#	for e.g. Few PL310 controller operations are supported 
#	from secure world.
#
#CONFIG_CACHE_PL310
#	Enables the PL310 L2 Cache controller support.
#
###

SIERRA_PRODUCT = Sierra TEE
SIERRA_MAJOR_VERSION = 1
SIERRA_MINOR_VERSION = 7
SIERRA_PATCH_LEVEL = 2
SIERRA_BUILD_NO = 3


export OTZONE_BUILD:= y
export CONFIG_MULTI_GUESTS_SUPPORT:=n
export CONFIG_ANDROID_GUEST:=n
export NON_SECURE_BOOTWRAPPER_SUPPORT:=y
export CONFIG_NEON_SUPPORT :=n
export CONFIG_KIM :=n

export SDK_PATH:= $(CURDIR)
export PACKAGE_DIR:= $(SDK_PATH)/../package
export PACKAGE_PATCHES_DIR:=$(PACKAGE_DIR)/patches
export PACKAGE_STORAGE_DIR:=$(PACKAGE_DIR)/storage
export PACKAGE_BUILD_DIR:=$(PACKAGE_DIR)/build
export CONFIG_PACKAGE_FILE:=$(PACKAGE_DIR)/config.package
export ANDROID_PATH:= /home/pub/ov_android/android
VERSION_FILE=$(SDK_PATH)/include/version.h

ifeq ($(CONFIG_ANDROID_GUEST),y)
export KDIR := $(ANDROID_PATH)/kernel
else	
export KDIR := $(SDK_PATH)/../../kernel/first/linux-xlnx
endif
ifeq ($(CONFIG_MULTI_GUESTS_SUPPORT),y)
export KDIR2 := $(SDK_PATH)/../../kernel/second/linux-xlnx
endif

ifeq ($(CONFIG_ANDROID_GUEST),y)
export KERNEL_BUILT_DIR := $(ANDROID_PATH)/out/target/product/vexpress_rtsm/obj/kernel
else	
export KERNEL_BUILT_DIR := $(SDK_PATH)/../../kernel/first/linux-xlnx
endif
ifeq ($(CONFIG_MULTI_GUESTS_SUPPORT),y)
export KERNEL_BUILT_DIR_2 := $(SDK_PATH)/../../kernel/second/linux-xlnx
endif

ifeq ($(CONFIG_ANDROID_GUEST),y)
NS_IMAGE := $(ANDROID_PATH)/out/target/product/vexpress_rtsm/boot/linux-system-A15.axf
else	
NS_IMAGE := $(SDK_PATH)/../../bootwrapper/first/normal.elf
endif
ifeq ($(CONFIG_MULTI_GUESTS_SUPPORT),y)
NS_IMAGE_2 := $(SDK_PATH)/../../bootwrapper/second/normal.elf
INITRD_IMAGE := $(SDK_PATH)/../otz_linux/linux2_initrd.bin.gz
endif

ifneq ($(BOARD), zynq7)
ROOT_FILE_SYSTEM_IMAGE := $(SDK_PATH)/../otz_linux/armv5t_min_EB_V6_V7.image 
endif

-include $(CONFIG_PACKAGE_FILE)

ifeq ($(CONFIG_NEWLIB), y)
ifndef CROSS_COMPILE_NEWLIB
$(error Experimental support for openssl is enabled. CROSS_COMPILE_NEWLIB \
		must be defined and point to a toolchain with \
		newlib support. For example, in bash do --\
		export CROSS_COMPILE_NEWLIB=arm-none-eabi-)
endif
endif

export BOARD:= zynq7
export ARM_CPU:= CORTEX_A9
export ARM_ARCH:= ARMV7

ifeq ($(ARM_CPU), CORTEX_A15)
ifneq ($(CONFIG_ANDROID_GUEST), y)
export OTZONE_ASYNC_NOTIFY_SUPPORT:= y
endif
endif


.PHONY: all

ifeq ($(CONFIG_ANDROID_GUEST),y)
all:	$(VERSION_FILE)
else
ifneq ($(BOARD), zynq7)
all:	$(ROOT_FILE_SYSTEM_IMAGE)	$(VERSION_FILE)
else
all:	$(VERSION_FILE)
endif
endif
ifeq ($(NON_SECURE_BOOTWRAPPER_SUPPORT),y)
	cp -f $(NS_IMAGE) $(SDK_PATH)/bin/normal.elf;
else
	cp -f $(NS_IMAGE) $(SDK_PATH)/bin/normal.bin;
endif
ifeq ($(CONFIG_MULTI_GUESTS_SUPPORT),y)

ifeq ($(NON_SECURE_BOOTWRAPPER_SUPPORT),y)
	cp -f $(NS_IMAGE_2) $(SDK_PATH)/bin/normal_2.elf;
else
	cp -f $(NS_IMAGE_2) $(SDK_PATH)/bin/normal_2.bin;
endif
ifneq ($(BOARD), zynq7)
	cp -f $(INITRD_IMAGE) $(SDK_PATH)/sierratee/linux2_initrd.bin.gz;
endif
endif

# Mingyuan's Change Location
ifneq ($(CONFIG_ANDROID_GUEST),y)
	$(MAKE) -C $(SDK_PATH)/otz_api CROSS_COMPILE=$(CROSS_COMPILE_LINARO)
	#$(MAKE) -C $(SDK_PATH)/ns_client_apps
endif
	$(MAKE) -C $(SDK_PATH)/otz_driver CROSS_COMPILE=$(CROSS_COMPILE_LINARO)
	$(MAKE) -C $(SDK_PATH)/../package CROSS_COMPILE=$(CROSS_COMPILE_LINARO)
# Mingyuan's Change Location

ifneq ($(BOARD), zynq7)
ifeq ($(CONFIG_MULTI_GUESTS_SUPPORT),y)
ifneq ($(CONFIG_ANDROID_GUEST),y)
	gunzip $(SDK_PATH)/sierratee/linux2_initrd.bin.gz;
	echo "Mounting filesystem of Second guest and copying file"; \
	sudo mkdir -p /tmp/mnt; \
	sudo mount -o loop $(SDK_PATH)/sierratee/linux2_initrd.bin /tmp/mnt;

	if [ -f ${SDK_PATH}/bin/otzapp.elf ]; then \
	mkdir -p /tmp/mnt/bin; \
	sudo cp ${SDK_PATH}/bin/otzapp.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otzapp.elf;\
	fi

	if [ -f ${SDK_PATH}/bin/otz_tee_app.elf ]; then \
	sudo mkdir -p /tmp/mnt/bin; \
	sudo cp ${SDK_PATH}/bin/otz_tee_app.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_tee_app.elf;\
	fi

	if [ -f ${SDK_PATH}/bin/otz_neon_app.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otz_neon_app.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_neon_app.elf;\
	fi
	
	if [ -f ${SDK_PATH}/bin/otz_test_app.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otz_test_app.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_test_app.elf;\
	fi

	if [ -f ${SDK_PATH}/bin/otz_client_2.ko ]; then \
	sudo cp ${SDK_PATH}/bin/otz_client_2.ko /tmp/mnt/; \
	fi

	if [ -f ${SDK_PATH}/lib/libotzapi.so ]; then \
	sudo mkdir -p /tmp/mnt/usr; \
	sudo mkdir -p /tmp/mnt/usr/lib; \
	sudo cp ${SDK_PATH}/lib/libotzapi.so /tmp/mnt/usr/lib; \
	fi
	
	sleep 1;\
	sudo umount /tmp/mnt; \
	echo "Mounting filesystem of First guest and copying file"; \
	sudo mount -o loop $(ROOT_FILE_SYSTEM_IMAGE) /tmp/mnt;
	
	if [ -f ${SDK_PATH}/bin/otz_boot_ack.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otz_boot_ack.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_boot_ack.elf;\
	fi

	if [ -f ${SDK_PATH}/bin/otz_boot_ack.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otz_boot_ack.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_boot_ack.elf;\
	fi


	sleep 1;\
	sudo umount /tmp/mnt; \
	gzip $(SDK_PATH)/sierratee/linux2_initrd.bin;
endif
endif
endif

ifeq ($(CONFIG_ANDROID_GUEST),y)
ifeq ($(BOARD),VE)
	if [ -f ${SDK_PATH}/bin/otz_client.ko ]; then \
	sudo mkdir -p /tmp/mnt ;\
	sudo tar -xjf $(ANDROID_PATH)/out/target/product/vexpress_rtsm/system.tar.bz2 -C /tmp/mnt; \
	sudo cp ${SDK_PATH}/bin/otz_client.ko /tmp/mnt/system/modules/; \
	sudo cd /tmp/mnt; \
	sudo tar -cjf $(ANDROID_PATH)/out/target/product/vexpress_rtsm/system.tar.bz2 system/; \
	sleep 1;\
	sudo rm -rf system; \
	sudo cd $(SDK_PATH);\
	fi
endif
endif
	
# Mingyuan's Change Location
	$(MAKE) -C $(SDK_PATH)/sierratee CROSS_COMPILE=$(CROSS_COMPILE_SOURCERY) 
# Mingyuan's Change Location

ifneq ($(BOARD), zynq7)
ifneq ($(CONFIG_ANDROID_GUEST),y)
	sudo mkdir -p /tmp/mnt; \
	sudo mount -o loop $(ROOT_FILE_SYSTEM_IMAGE) /tmp/mnt;
	if [ -f ${SDK_PATH}/bin/otzapp.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otzapp.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otzapp.elf;\
	fi

	if [ -f ${SDK_PATH}/bin/otz_tee_app.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otz_tee_app.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_tee_app.elf;\
	fi

	if [ -f ${SDK_PATH}/bin/otz_boot_ack.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otz_boot_ack.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_boot_ack.elf;\
	fi

	if [ -f ${SDK_PATH}/bin/otz_play_media.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otz_play_media.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_play_media.elf;\
	fi

	if [ -f ${SDK_PATH}/bin/otz_neon_app.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otz_neon_app.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_neon_app.elf;\
	fi

	if [ -f ${SDK_PATH}/bin/otz_test_app.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otz_test_app.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_test_app.elf;\
	fi

ifeq ($(CONFIG_KIM),y)
	if [ -f ${SDK_PATH}/bin/kernel_integrity_tee.elf ]; then \
	sudo cp ${SDK_PATH}/bin/kernel_integrity_tee.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/kernel_integrity_tee.elf;\
	fi
endif


	if [ -f ${SDK_PATH}/bin/otz_user_app.elf ]; then \
	sudo cp ${SDK_PATH}/bin/otz_user_app.elf /tmp/mnt/bin; \
	sudo chmod 755 /tmp/mnt/bin/otz_user_app.elf;\
	fi

	if [ -f ${SDK_PATH}/bin/otz_client.ko ]; then \
	sudo cp ${SDK_PATH}/bin/otz_client.ko /tmp/mnt/; \
	fi

	if [ -f ${SDK_PATH}/lib/libotzapi.so ]; then \
	sudo cp ${SDK_PATH}/lib/libotzapi.so /tmp/mnt/usr/lib; \
	fi
	
	sleep 1;\
	sudo umount /tmp/mnt;
endif
endif

boot:
	$(MAKE) -C $(SDK_PATH)/sierratee boot

clean:
	$(MAKE) -C $(SDK_PATH)/../package clean
	$(MAKE) -C $(SDK_PATH)/sierratee clean
ifneq ($(CONFIG_ANDROID_GUEST),y)
	$(MAKE) -C $(SDK_PATH)/ns_client_apps clean
	$(MAKE) -C $(SDK_PATH)/otz_api clean
endif
	$(MAKE) -C $(SDK_PATH)/otz_driver clean

	sudo rm -f bin/*.elf
	sudo rm -f bin/*.bin
	sudo rm -f lib/*.so
	sudo rm -f bin/*.ko
	sudo rm -f $(VERSION_FILE)
 
$(VERSION_FILE):
	@( printf '#define SIERRA_PRODUCT_NAME "%s"\n' \
		'$(SIERRA_PRODUCT)')>>  $@.tmp
	@(printf '#define SIERRA_MAJOR_VERSION "%s"\n' \
		'$(SIERRA_MAJOR_VERSION)')>>  $@.tmp
	@(printf '#define SIERRA_MINOR_VERSION "%s"\n' \
		'$(SIERRA_MINOR_VERSION)')>>  $@.tmp
	@(printf '#define SIERRA_PATCH_LEVEL "%s"\n' \
		'$(SIERRA_PATCH_LEVEL)')>>  $@.tmp
	@( printf '#define SIERRA_BUILD_NO "%s"\n' \
		'$(SIERRA_BUILD_NO)')>>  $@.tmp
	@cmp -s $@ $@.tmp && rm -f $@.tmp || mv -f $@.tmp $@
