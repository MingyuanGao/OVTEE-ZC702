# Remove old images
echo Removing boot.bin
sudo rm -f /media/me/boot/boot.bin

echo Removing ovtee.bin
sudo rm -f /media/me/boot/ovtee.bin

echo Removing otzclient  
sudo rm -rf /media/me/rootfs/home/linaro/otzclient

echo Removing old kernel modules 
sudo rm -rf /media/me/rootfs/lib/modules

# Copy new images
echo Copying boot.bin
cp boot.bin /media/me/boot

echo Copying ovtee.bin
cp ovtee.bin /media/me/boot

echo Copying dir otzclient
cp otzclient -r /media/me/rootfs/home/linaro/

echo Copying kernel modules
sudo cp modules/lib/modules -r /media/me/rootfs/lib/

# Flush the buffer
sync
