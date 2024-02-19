#!/bin/sh

busybox echo "Seting up user space"
for i in $(/bin/busybox --list)
 do
   /bin/busybox ln -s busybox bin/$i
 done

echo "mount proc"
mount -t proc proc /proc
echo "mount sys"
mount -t sysfs sysfs /sys
echo "mount dev"
mount -t devtmpfs devtmpfs /dev
echo "Mounts done"