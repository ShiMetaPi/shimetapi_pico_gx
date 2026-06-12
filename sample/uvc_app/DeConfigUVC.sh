#!/bin/sh

echo "" > /sys/kernel/config/usb_gadget/camera/UDC
unlink /sys/kernel/config/usb_gadget/camera/configs/c.1/uvc.usb0
rm -rf /sys/kernel/config/usb_gadget/camera/configs/c.1/ 2>>/dev/null
rm -rf /sys/kernel/config/usb_gadget/camera/functions/uvc.usb0 2>>/dev/null
rm -rf /sys/kernel/config/usb_gadget/camera 2>>/dev/null
umount /sys/kernel/config/
#rmmod usb_f_fs
#rmmod usb_f_uvc
#rmmod libcomposite
