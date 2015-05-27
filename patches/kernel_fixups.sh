# Patch the xilinx kernel

PWD=`pwd`
TOP=$PWD/../

#Multi guest not supported at the moment.
MULTI_GUEST="n"


echo "PATCHING LINUX 1....."
cd $TOP/kernel/first/linux-xlnx

echo "Patching zynq_base_trd patch for linux ...."
git apply --stat $TOP/patches/zynq_base_trd_14_5.patch
git apply --check $TOP/patches/zynq_base_trd_14_5.patch
git am $TOP/patches/zynq_base_trd_14_5.patch

echo "Patching otz patch for linux ...."
git apply --stat $TOP/patches/0001-zynq-enable-TEE-extensions.patch
git apply --check $TOP/patches/0001-zynq-enable-TEE-extensions.patch
git am $TOP/patches/0001-zynq-enable-TEE-extensions.patch

echo "Patching otz patch for smp support in linux ...."
git apply --stat $TOP/patches/0002-zynq-smp-support-for-TEE-extended-linux.patch
git apply --check $TOP/patches/0002-zynq-smp-support-for-TEE-extended-linux.patch
git am $TOP/patches/0002-zynq-smp-support-for-TEE-extended-linux.patch

git apply --stat $TOP/patches/0004-zynq-config-file-for-SMP-enabled-kernel.patch
git apply --check $TOP/patches/0004-zynq-config-file-for-SMP-enabled-kernel.patch
git am $TOP/patches/0004-zynq-config-file-for-SMP-enabled-kernel.patch

# This is not supported at the moment
if [ "$MULTI_GUEST" = "y" ]
then
echo "Patching disable uart in linux 1...."
git apply --stat $TOP/patches/0003-zynq-disable-UART-in-a-TEE-extended-linux.patch
git apply --check $TOP/patches/0003-zynq-disable-UART-in-a-TEE-extended-linux.patch
git am $TOP/patches/0003-zynq-disable-UART-in-a-TEE-extended-linux.patch

echo "PATCHING LINUX 2......"
#cd $TOP/kernel/second/linux-xlnx

echo "Patching otz patch for linux ...."
#patch -p1 < $TOP/patches/kernel_second.patch
fi

cd $TOP
