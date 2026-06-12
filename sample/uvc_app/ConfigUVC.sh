#!/bin/sh

export LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH

#insmod libcomposite.ko
#insmod usb_f_uvc.ko
#insmod u_audio.ko
#insmod usb_f_uac1.ko
#insmod usb_f_uac2.ko
#insmod usb_f_fs.ko
#insmod usbmon.ko

UVC_ENABLE="true"
UVC1_ENABLE="false"
UAC_ENABLE="true"

UVC_ENABLE_YUYV="true"
UVC_ENABLE_NV21="true"
UVC_ENABLE_MJPEG="true"
UVC_ENABLE_H264="true"
UVC_ENABLE_H265="false"

# UVC data endpoint transmission mode: iso or bulk
UVC_TRANSFER_MODE="isoc"

# USB speed.
# ss: super-speed; hs: high-speed; fs: full-speed.
USB_SPEED="hs"

export VID="0x3337"
export PID="0x4321"
export MANUFACTURER="XMedia"
export PRODUCT="HD USB Camera"
export SERIALNUMBER="20251215"

export YUYV="640x360@30 1280x720@13"
export NV21="640x360@30 1280x720@13"
export MJPEG="640x360@30 1280x720@30 1920x1080@30"
export H264="640x360@30 1280x720@30 1920x1080@30"
export H265="640x360@30 1280x720@30 1920x1080@30"

export YUYV_GUID="YUY2\x00\x00\x10\x00\x80\x00\x00\xaa\x00\x38\x9b\x71"
export YUYV_BITS="16"
export NV21_GUID="NV21\x00\x00\x10\x00\x80\x00\x00\xaa\x00\x38\x9b\x71"
export NV21_BITS="12"
export H264_GUID="H264\x00\x00\x10\x00\x80\x00\x00\xaa\x00\x38\x9b\x71"
export H265_GUID="H265\x00\x00\x10\x00\x80\x00\x00\xaa\x00\x38\x9b\x71"

export FUNCTION_UVC="uvc.usb0"
export FUNCTION_UVC1="uvc1.usb0"
export FUNCTION_UAC1="uac1.usb0"

#-USB Device descriptor.
function gen_dev_desc() {
	echo "0x01" > bDeviceProtocol
	echo "0x02" > bDeviceSubClass
	echo "0xEF" > bDeviceClass
	echo "0x200" > bcdUSB
	echo $VID > idVendor
	echo $PID > idProduct
	echo "0x100" > bcdDevice
	mkdir -p strings/0x409
	echo $MANUFACTURER > strings/0x409/manufacturer
	echo $PRODUCT > strings/0x409/product
	echo $SERIALNUMBER > strings/0x409/serialnumber
}

#-Payload Format Descriptors
function gen_payload_format_desc() {
	local PAYLOAD=$1
	local FORMAT=$2
	local GUID=$3
	local BITS=$4
	local RESOLUTIONS=$5
	local CR=1

	mkdir -p streaming/$PAYLOAD/$FORMAT/

	if [ $GUID != "null" ];then
		echo -n -e $GUID > streaming/$PAYLOAD/$FORMAT/guidFormat
	fi

	if [ $BITS -ne 0 ];then
		echo -n -e $BITS > streaming/$PAYLOAD/$FORMAT/bBitsPerPixel
	fi

	if [ $FORMAT = "mjpeg" ];then
		# The compression ratio is calculated at 10:1.
		CR=10
	elif [ $FORMAT = "h264" ];then
		# The compression ratio is calculated at 50:1.
		CR=50
	elif [ $FORMAT = "h265" ];then
		# The compression ratio is calculated at 80:1.
		CR=100
	fi

	for str in $RESOLUTIONS; do
		# Width x Height
		local RES=$(echo "$str" | awk -F '@' '{print $1}')
		# Interval
		local I=$(echo "$str" | awk -F '@' '{print $2}')
		# Width
		local W=$(echo "$RES" | awk -F 'x' '{print $1}')
		# Height
		local H=$(echo "$RES" | awk -F 'x' '{print $2}')

		RES="${RES}p"

		# microsecond
		local TIME=10000000
		# Interval
		local INTERVAL=$((TIME / I))
		# Video Frame Buffer Size
		local FRAMESIZE=$((W * H))
		if [ $BITS -eq 16 ]; then
			FRAMESIZE=$((FRAMESIZE * 2))
		else
			# The data before encoding is YUV420
			FRAMESIZE=$((FRAMESIZE * 3))
			FRAMESIZE=$((FRAMESIZE / 2))
		fi

		FRAMESIZE=$((FRAMESIZE / CR))

		# Max Bits Rate
		local MAXBITRATE=$((FRAMESIZE * 8))
		MAXBITRATE=$((MAXBITRATE * I))
		# Min Bits Rate
		local MINBITRATE=$MAXBITRATE

		mkdir -p streaming/$PAYLOAD/$FORMAT/$RES/
		echo "${W}" > streaming/$PAYLOAD/$FORMAT/$RES/wWidth
		echo "${H}" > streaming/$PAYLOAD/$FORMAT/$RES/wHeight
		echo "${INTERVAL}" > streaming/$PAYLOAD/$FORMAT/$RES/dwFrameInterval
		echo "${INTERVAL}" > streaming/$PAYLOAD/$FORMAT/$RES/dwDefaultFrameInterval
		echo "${MAXBITRATE}" > streaming/$PAYLOAD/$FORMAT/$RES/dwMaxBitRate
		echo "${MINBITRATE}" > streaming/$PAYLOAD/$FORMAT/$RES/dwMinBitRate

		if [ $PAYLOAD != "framebased" ];then
			echo "${FRAMESIZE}" > streaming/$PAYLOAD/$FORMAT/$RES/dwMaxVideoFrameBufferSize
		fi

		#echo "$FORMAT $W x $H @ $INTERVAL $FRAMESIZE $MAXBITRATE"
	done
}

#-For YUV Payload Format Descriptor
function gen_uncompressed_desc() {
	local PAYLOAD="uncompressed"
	local FORMAT=$1
	local GUID=$2
	local BITS=$3
	local RESOLUTIONS=$4

	gen_payload_format_desc ${PAYLOAD} ${FORMAT} ${GUID} ${BITS} "${RESOLUTIONS}"

	ln -s streaming/$PAYLOAD/$FORMAT/ streaming/header/h/
}

#-For FRAMEBASE Payload Format Descriptor
function gen_framebase_desc() {
	local PAYLOAD="framebased"
	local FORMAT=$1
	local GUID=$2
	local BITS=0
	local RESOLUTIONS=$3

	gen_payload_format_desc ${PAYLOAD} ${FORMAT} ${GUID} ${BITS} "${RESOLUTIONS}"

	ln -s streaming/$PAYLOAD/$FORMAT/ streaming/header/h/
}

#-For MJPEG Payload Format Descriptor
function __gen_mjpeg_desc() {
	local PAYLOAD="mjpeg"
	local FORMAT="mjpeg"
	local GUID="null"
	local BITS=0
	local RESOLUTIONS=$1

	gen_payload_format_desc ${PAYLOAD} ${FORMAT} ${GUID} ${BITS} "${RESOLUTIONS}"

	ln -s streaming/$PAYLOAD/$FORMAT/ streaming/header/h/
}

#-YUYV Format
function gen_yuyv_desc() {
	gen_uncompressed_desc "yuyv" "${YUYV_GUID}" "${YUYV_BITS}" "${YUYV}"
}

#-NV21 Format
function gen_nv21_desc() {
	gen_uncompressed_desc "nv21" "${NV21_GUID}" "${NV21_BITS}" "${NV21}"
}

#-H264 Format
function gen_h264_desc() {
	gen_framebase_desc  "h264" "${H264_GUID}" "${H264}"
}

#-H265 Format
function gen_h265_desc() {
	gen_framebase_desc  "h265" "${H265_GUID}" "${H265}"
}

#-MJPEG Format
function gen_mjpeg_desc() {
	__gen_mjpeg_desc "${MJPEG}"
}

#-UVC common
function __gen_uvc_desc() {
	local FUNNAME=$1

	mkdir -p functions/$FUNNAME
	cd functions/$FUNNAME

	# streaming_maxpacket: 1 ~ 3072
	# high-speed and super-speed: (1 ~ 3072); full-speed: (1 ~ 1023)
	if [ $UVC_TRANSFER_MODE = "isoc" ];then
		# USB ISOC transfer mode
		echo 0 > streaming_bulk
		echo 1 > streaming_interval

		if [ $USB_SPEED = "fs" ];then
			# USB full-speed
			echo 0 > streaming_maxburst
			echo 512 > streaming_maxpacket
		elif [ $USB_SPEED = "hs" ];then
			# USB high-speed
			echo 0 > streaming_maxburst
			echo 3072 > streaming_maxpacket
		elif [ $USB_SPEED = "ss" ];then
			# USB super-speed
			echo 9 > streaming_maxburst
			echo 3072 > streaming_maxpacket
		fi
	else
		# USB BULK transfer mode
		echo 1 > streaming_bulk
		echo 0 > streaming_interval

		if [ $USB_SPEED = "fs" ];then
			# USB full-speed
			echo 0 > streaming_maxburst
			echo 64 > streaming_maxpacket
		elif [ $USB_SPEED = "hs" ];then
			# USB high-speed
			echo 0 > streaming_maxburst
			echo 512 > streaming_maxpacket
		elif [ $USB_SPEED = "ss" ];then
			# USB super-speed
			echo 9 > streaming_maxburst
			echo 1024 > streaming_maxpacket
		fi
	fi

	mkdir -p control/header/h/
	mkdir -p streaming/header/h/

	echo "0x0110" > control/header/h/bcdUVC
	echo "96000000" > control/header/h/dwClockFrequency

	ln -s control/header/h/ control/class/fs/
	ln -s control/header/h/ control/class/ss/

	#YUYV
	if [ $UVC_ENABLE_YUYV = "true" ];then
		gen_yuyv_desc
	fi

	#NV21
	if [ $UVC_ENABLE_NV21 = "true" ];then
		gen_nv21_desc
	fi

	#MJPEG
	if [ $UVC_ENABLE_MJPEG = "true" ];then
		gen_mjpeg_desc
	fi

	#H264
	if [ $UVC_ENABLE_H264 = "true" ];then
		gen_h264_desc
	fi

	#H265
	if [ $UVC_ENABLE_H265 = "true" ];then
		gen_h265_desc
	fi

	ln -s streaming/header/h/ streaming/class/fs/
	ln -s streaming/header/h/ streaming/class/hs/
	ln -s streaming/header/h/ streaming/class/ss/

	cd ../../
}

#-UAC common
function __gen_uac_desc() {
	local FUNNAME=$1

	mkdir -p functions/$FUNNAME
	echo "0x01" > functions/$FUNNAME/c_chmask
	echo "16000" > functions/$FUNNAME/c_srate

	echo "0x01" > functions/$FUNNAME/p_chmask
	echo "16000" > functions/$FUNNAME/p_srate
}

#-UVC
function gen_uvc_desc() {
	__gen_uvc_desc "${FUNCTION_UVC}"
}

#-UVC1
function gen_uvc1_desc() {
	__gen_uvc_desc "${FUNCTION_UVC1}"
}

#-UAC1
function gen_uac1_desc() {
	__gen_uac_desc "${FUNCTION_UAC1}"
}

################################################################################################

# Main
APP_PATH=$PWD

# Step 1
mount -t configfs none /sys/kernel/config/
cd /sys/kernel/config/usb_gadget/
mkdir -p camera
cd camera

# Step 2
gen_dev_desc

# Step 3
if [ $UVC_ENABLE = true ];then
	gen_uvc_desc
fi

if [ $UVC1_ENABLE = true ];then
	gen_uvc1_desc
fi

if [ $UAC_ENABLE = true ];then
	gen_uac1_desc
fi

# Step 4
#-Create and setup configuration
mkdir -p configs/c.1/
echo "500" > configs/c.1/MaxPower
echo "0x80" > configs/c.1/bmAttributes
mkdir -p configs/c.1/strings/0x409/
echo "Config 1" > configs/c.1/strings/0x409/configuration

# Step 5
if [ $UVC_ENABLE = true ];then
	ln -s functions/${FUNCTION_UVC}/ configs/c.1/
fi

if [ $UVC1_ENABLE = true ];then
	ln -s functions/${FUNCTION_UVC1}/ configs/c.1/
fi

if [ $UAC_ENABLE = true ];then
	ln -s functions/${FUNCTION_UAC1}/ configs/c.1/
fi

# Step 6
dwc=$(ls /sys/class/udc)
echo "${dwc}" > UDC

# Step 7
#cd $APP_PATH
#./uvc_app
