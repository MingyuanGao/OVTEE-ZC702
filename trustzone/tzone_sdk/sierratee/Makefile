ifndef SDK_PATH
$(error SDK_PATH must be defined before this file is included)
endif

-include $(CONFIG_PACKAGE_FILE)

export $BOARD
export $ARM_CPU
export $ARM_ARCH
export $ENABLE_LIBCRYPT

ifneq ($(CONFIG_NEWLIB), y)
ifeq ($(CONFIG_CRYPTO), y)
	export CONFIG_NEWLIB=y
endif
ifeq ($(CONFIG_FFMPEG), y)
	export CONFIG_NEWLIB=y
endif
endif

.PHONY: all
all: 
	if [ ! -d src/package ]; then \
	mkdir -p src/package/src; \
	cp -R $(SDK_PATH)/../package/src src/package/; \
	fi

ifeq ($(CONFIG_CRYPTO), y)
	$(MAKE) -f Makefile.in modules_crypto
endif	
ifeq ($(CONFIG_FFMPEG), y)
	$(MAKE) -f Makefile.in modules_ffmpeg
endif	

	$(MAKE) -f Makefile.in modules

	$(MAKE) -f Makefile.in clean

	$(MAKE) -f Makefile.in
	$(MAKE) -f Makefile.boot
	
	rm -r -f src/package

boot:
	$(MAKE) -f Makefile.boot

.PHONY: clean
clean:
	$(MAKE) -f Makefile.in clean
	$(MAKE) -f Makefile.boot clean
	
	rm -r -f src/package


