#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
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

    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
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
    sudo rm -rf ${OUTDIR}/rootfs
    mkdir -p ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
# May need to change directory first or point to rootfs

cd ${OUTDIR}/rootfs/
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
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
    make ARCH=${ARCH} defconfig
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
    make ARCH=${ARCH} CONFIG_PREFIX=${OUTDIR}/rootfs/bin
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
else
    cd busybox
fi

echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/busybox/busybox | grep "program interpreter" 
${CROSS_COMPILE}readelf -a ${OUTDIR}/busybox/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
# Copy from the cross compile location to lib or lib64
# Probably someone smarter than I am could make this cleaner.

SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)

/usr/bin/cp -ap $(echo ${SYSROOT}/lib/ld-linux-aarch64.so.1) ${OUTDIR}/rootfs/lib
/usr/bin/cp -a $(echo ${SYSROOT}/lib64/*) ${OUTDIR}/rootfs/lib64
/usr/bin/cp -a ${OUTDIR}/busybox/busybox ${OUTDIR}/rootfs/bin/busybox

cd ${OUTDIR}/rootfs/bin
ln -sf busybox sh
ln -sf busybox ash

# TODO: Make device nodes
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/console c 5 1
sudo mknod ${OUTDIR}/rootfs/dev/ram0 b 1 0

# TODO: Clean and build the writer utility

rm -rf ${FINDER_APP_DIR}/*.o ${FINDER_APP_DIR}/*.elf ${FINDER_APP_DIR}/*.bin ${FINDER_APP_DIR}/*.s
${CROSS_COMPILE}gcc ${FINDER_APP_DIR}/writer.c -o ${FINDER_APP_DIR}/writer

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs

mkdir -p ${OUTDIR}/rootfs/etc/rc0.d
/usr/bin/cp -a ${FINDER_APP_DIR}/S0links.sh ${OUTDIR}/rootfs/etc/rc0.d/
chmod +x ${OUTDIR}/rootfs/etc/rc0.d/S0links.sh

/bin/cp ${FINDER_APP_DIR}/writer ${OUTDIR}/rootfs/home/

/bin/cp ${FINDER_APP_DIR}/finder_bb.sh ${OUTDIR}/rootfs/home/finder.sh
chmod +x ${OUTDIR}/rootfs/home/finder.sh

/bin/cp ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home/
/bin/cp ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home/

mkdir -p ${OUTDIR}/rootfs/home/conf/
/bin/cp ${FINDER_APP_DIR}/../conf/* ${OUTDIR}/rootfs/home/conf/

# TODO: Chown the root directory
cd ${OUTDIR}/rootfs/
sudo chown -R root:root *
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}

# TODO: Create initramfs.cpio.gz
gzip -f ${OUTDIR}/initramfs.cpio


