kernel-apps-objs-y=dispatcher_task.o
kernel-apps-objs-y+=linux_task.o
#apps-objs-y+=drm_task.o
#apps-objs-y+=rc4_algorithm.o
#apps-objs-$(CONFIG_NEON_SUPPORT) += neon_app.o
#apps-objs-$(CONFIG_KIM)+=kernel_checker.o
kernel-apps-objs-$(CONFIG_SHELL)+=shell_process_task.o

apps-objs-y+=echo_task.o
apps-objs-$(CONFIG_CRYPTO)+=crypto_task.o
apps-objs-$(CONFIG_CRYPTO)+=crypto_tests.o
apps-objs-$(CONFIG_FFMPEG)+=play_video.o
apps-objs-$(CONFIG_FFMPEG)+=ffmpeg_test_task.o

ifeq ($(CONFIG_CRYPTO), y)
modules-crypto-objs-y+=crypto_task.o
modules-crypto-objs-y+=crypto_tests.o
endif

ifeq ($(CONFIG_FFMPEG), y)
modules-ffmpeg-objs-y+=play_video.o
modules-ffmpeg-objs-y+=ffmpeg_test_task.o
endif

