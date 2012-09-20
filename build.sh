#!/bin/bash

HWVER=rev_a
CLEAN=""


WORK_PATH=`pwd`
#IMAGE_PART1='../../../../../android/images/p970'
IMAGE_PART1='./'


for argument in "$@"
do
	case "$argument" in
    hw*)
      HWVER=${argument:2} 
      ;;
	"clean")
	  CLEAN=clean 
	  make clean
	  exit 0
	  ;;
	esac
done


echo "##############building kernel HWVER = ${HWVER} ####################"

#make ${CLEAN} ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- sniper_${HWVER}_emmc_defconfig
#make ${CLEAN} ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- android_zoom3_defconfig
make ${CLEAN} ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- black_p970_defconfig
make ${CLEAN} -j8 ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- uImage
make ${CLEAN} ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- modules

#if [ ! -d ${IMAGE_PART1} ]
#then
#	mkdir ${IMAGE_PART1}
#fi

#cp -vf arch/arm/boot/uImage ${IMAGE_PART1}
