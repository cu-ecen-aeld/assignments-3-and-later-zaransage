#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
    # TODO: Add your kernel build steps here

    echo "CONFIG_BLK_DEV_RAM=y" >> ./arch/arm64/configs/defconfig
    echo "CONFIG_BLK_DEV_RAM_COUNT=1" >> ./arch/arm64/configs/defconfig
    echo "CONFIG_BLK_DEV_RAM_SIZE=131072" >> ./arch/arm64/configs/defconfig

    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs

fi

echo "Adding the Image in outdir"
mkdir -p ${OUTDIR}/rootfs
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/Image


echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
    mkdir -p ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
# May need to change directory first or point to rootfs

cd ${OUTDIR}/rootfs/
mkdir -p bin dev etc home lib lib64 proc sbon sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log 


cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
    make CONFIG_PREFIX=${OUTDIR}/rootfs
else
    cd busybox
fi

# TODO: Make and install busybox

make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} #install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/busybox/busybox | grep "program interpreter" 
${CROSS_COMPILE}readelf -a ${OUTDIR}/busybox/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
# Copy from the cross compile location to lib or lib64
# Probably someone smarter than I am could make this cleaner.

SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
/usr/bin/cp -va $(echo ${SYSROOT}/lib64/libm.so.6) ${OUTDIR}/rootfs/lib64
/usr/bin/cp -va $(echo ${SYSROOT}/lib64/libresolv.so.2) ${OUTDIR}/rootfs/lib64
/usr/bin/cp -va $(echo ${SYSROOT}/lib64/libc.so.6) ${OUTDIR}/rootfs/lib64
/usr/bin/cp -va $(echo ${SYSROOT}/lib64/ld-2.31.so) ${OUTDIR}/rootfs/lib64
/usr/bin/ln -s $(echo ${SYSROOT}/lib64/ld-2.31.so) ${OUTDIR}/rootfs/lib64/ld-linux-x86-64.so.2
/usr/bin/ln -s $(echo ${SYSROOT}/lib64/ld-2.31.so) ${OUTDIR}/rootfs/lib/ld-linux-x86-64.so.2
/usr/bin/ln -s $(echo ${SYSROOT}/lib64/ld-2.31.so) ${OUTDIR}/rootfs/lib64/linux-aarch64.so.1
/usr/bin/ln -s ${OUTDIR}/rootfs/bin/busybox ${OUTDIR}/rootfs/bin/bash
/usr/bin/cp -va ${OUTDIR}/busybox/busybox ${OUTDIR}/rootfs/bin/busybox

# TODO: Make device nodes
cd ${OUTDIR}/rootfs/
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1
#sudo mknod dev/ram0 b 1 0

# TODO: Clean and build the writer utility

#make clean -f Makefile
#make CROSS_COMPILE -f Makefile

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs

#/bin/cp writer finder.sh finder-test.sh ../conf/assignments.txt ../conf/username.txt ${OUTDIR}/rootfs/home/

# TODO: Chown the root directory
cd ${OUTDIR}/rootfs/
sudo chown -R root:root *
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}

# TODO: Create initramfs.cpio.gz
gzip -f ${OUTDIR}/initramfs.cpio

mkimage -A ${ARCH} -O linux -T ramdisk -d ${OUTDIR}/initramfs.cpio.gz ${OUTDIR}/uRamdisk


