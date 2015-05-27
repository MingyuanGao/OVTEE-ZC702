kernel-test-apps-objs-$(CONFIG_TEST_SUITE)=int_contxt_switch_task.o
kernel-test-apps-objs-$(CONFIG_TEST_SUITE)+=test_suite_task.o

test-apps-objs-$(CONFIG_TEST_TASKS)=heap_test_task.o
test-apps-objs-$(CONFIG_TEST_TASKS)+=test_shm.o
test-apps-objs-$(CONFIG_TEST_TASKS)+=test_suite_user.o
