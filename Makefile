PRJ_DIR = $(HOME)/workspace/ovtee_zc702
LINUX_KERNEL_DIR = $(PRJ_DIR)/kernel/first/linux-xlnx
DTS_DIR = $(PRJ_DIR)/dts
BOOTWRAPPER_DIR = $(PRJ_DIR)/bootwrapper/first
TRUSTZONE_DIR = $(PRJ_DIR)/trustzone
TOOLS_DIR = $(PRJ_DIR)/tools
IMAGES_DIR = $(PRJ_DIR)/images


MAKE = make
CP = cp
RM = rm

export CROSS_COMPILE_LINARO= arm-linux-gnueabihf-
export CROSS_COMPILE_SOURCERY= $(TOOLS_DIR)/arm-2013.05/bin/arm-none-linux-gnueabi-
export CROSS_COMPILE_NEWLIB = $(TOOLS_DIR)/newlib/bin/arm-none-gnueabi-


.PHONY: all clean

all: linux_kernel devicetree.dtb normal_linux.elf ovtee otzclient_sdk

# Compile Linux Kernel (including modules) and Copy it to "images" dir
linux_kernel:
	$(MAKE) -C $(LINUX_KERNEL_DIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE_LINARO) xilinx_zynq_base_trd_nosmp_defconfig
	$(MAKE) -C $(LINUX_KERNEL_DIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE_LINARO) -j8 Image modules
	$(CP) $(LINUX_KERNEL_DIR)/arch/arm/boot/Image $(IMAGES_DIR)/linux_image
	$(MAKE) -C $(LINUX_KERNEL_DIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE_LINARO) modules_install INSTALL_MOD_PATH=$(IMAGES_DIR)/modules

devicetree.dtb:$(DTS_DIR)/zynq-zc702-base-trd.dts
	$(TOOLS_DIR)/dtc -I dts -O dtb $(DTS_DIR)/zynq-zc702-base-trd.dts -o $(IMAGES_DIR)/devicetree.dtb 

normal_linux.elf:$(IMAGES_DIR)/linux_image $(IMAGES_DIR)/devicetree.dtb
	$(MAKE) -C $(BOOTWRAPPER_DIR) CROSS_COMPILE=$(CROSS_COMPILE_LINARO) 

ovtee: 
	$(MAKE) -C $(TRUSTZONE_DIR)/tzone_sdk/ 
	$(CP) -f $(TRUSTZONE_DIR)/tzone_sdk/bin/SierraTEE.bin  $(IMAGES_DIR)/ovtee.bin

otzclient_sdk:
	$(CP) -f $(TRUSTZONE_DIR)/tzone_sdk/include -r $(IMAGES_DIR)/otzclient/
	$(CP) -f $(TRUSTZONE_DIR)/tzone_sdk/lib -r $(IMAGES_DIR)/otzclient/
	$(CP) -f $(TRUSTZONE_DIR)/tzone_sdk/ns_client_apps -r $(IMAGES_DIR)/otzclient/
	$(CP) -f $(TRUSTZONE_DIR)/tzone_sdk/bin/otz_client.ko $(IMAGES_DIR)/otzclient/

clean:
	$(RM) -f $(IMAGES_DIR)/linux_image
	$(RM) -f $(IMAGES_DIR)/modules/* -rf
	$(RM) -f $(IMAGES_DIR)/devicetree.dtb
	$(MAKE) -C $(BOOTWRAPPER_DIR) clean
	$(MAKE) -C $(TRUSTZONE_DIR)/tzone_sdk clean
	$(RM) -f $(IMAGES_DIR)/ovtee.bin
	$(RM) -f $(IMAGES_DIR)/otzclient/* -r

distclean:
	$(MAKE) -C $(LINUX_KERNEL_DIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE_LINARO) distclean
	$(MAKE) clean
