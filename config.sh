#!/bin/bash

if [ -z $1 ] 
then
DEFCONFIG=hub_rev_b_defconfig
else
DEFCONFIG=$1_defconfig
fi

export ARCH=arm
make $DEFCONFIG
make CROSS_COMPILE=arm-none-linux-gnueabi- menuconfig
cp .config arch/arm/configs/$DEFCONFIG
rm .config

