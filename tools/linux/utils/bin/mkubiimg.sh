#!/usr/bin/env bash
###############################################################################
#
#  Copyright (c) XMEDIA.
#  All rights reserved.
#  Create By Caizhiyong 2021/3/16
#
###############################################################################
#
# SELF <pagesize> <blocksize> <rootdir> <image>
# mkubiimg.sh 2k 128k rootdir xxxx.ubiimg
#
###############################################################################

function run ()
{
	local cmd=$1
	echo "${cmd}"
	msg=$(eval "${cmd}"); result=$?
	echo ${msg}
	[ ${result} == 0 ] || exit ${result}
}
###############################################################################

function hstrtol ()
{
	local hstr=$1
	local zoom=1
	local result=$(echo "${hstr}" | awk '{printf "%d",$0}')

	if [ "$(echo ${hstr} | grep '[Gg]')" == "${hstr}" ]; then
		zoom=1073741824
	elif [ "$(echo ${hstr} | grep '[Mm]')" == "${hstr}" ]; then
		zoom=1048576
	elif [ "$(echo ${hstr} | grep '[Kk]')" == "${hstr}" ]; then
		zoom=1024
	fi

	echo $((${result} * ${zoom}))
}
###############################################################################

SELF=$(basename ${0})
SELFDIR=$(dirname ${0})
PAGESIZE=$(hstrtol ${1})
BLOCKSIZE=$(hstrtol ${2})
ROOTDIR=$(echo $(echo "${3} " | sed 's/\/ //'))
IMAGE=${4}

if [ ! -d ${ROOTDIR} ]; then
	echo "${SELF}: *** Directory \"${ROOTDIR}\" not exists."
	exit 2
fi

IMAGEDIR=$(dirname ${IMAGE}) 

if [ ! -d ${IMAGEDIR} ]; then
	echo "${SELF}: *** Directory \"${IMAGEDIR}\" not exists."
	exit 2
fi

PARTSIZE=$(hstrtol 64M)
LEB=$((${BLOCKSIZE} - ${PAGESIZE} * 2))
MAX_LEB_CNT=$((${PARTSIZE} / ${BLOCKSIZE}))

###############################################################################

MKUBIFS=${SELFDIR}/mkfs.ubifs
MKUBI=${SELFDIR}/ubinize

if [ ! -f ${MKUBIFS} ]; then
	echo "${SELF}: *** mkfs.ubifs \"${MKUBIFS}\" not exists."
	exit 2
fi

if [ ! -f ${MKUBI} ]; then
	echo "${SELF}: *** ubinize \"${MKUBI}\" not exists."
	exit 2
fi

run "${MKUBIFS} -F -d ${ROOTDIR} -m ${PAGESIZE} -o ${IMAGE}.ubiimg -e ${LEB} -c ${MAX_LEB_CNT}"

cat > ${IMAGE}.ubicfg << __EOF
[ubifs-volumn]
mode=ubi
image=${IMAGE}.ubiimg
vol_id=0
vol_type=dynamic
vol_alignment=1
vol_name=ubifs
vol_size=64MiB
vol_flags=autoresize

__EOF

run "${MKUBI} -o ${IMAGE} -m ${PAGESIZE} -p ${BLOCKSIZE} ${IMAGE}.ubicfg"

rm -f ${IMAGE}.ubicfg ${IMAGE}.ubiimg

