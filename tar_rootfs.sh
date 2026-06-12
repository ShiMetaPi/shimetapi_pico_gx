#!/bin/bash
#
# Tar Rootfs Script
# Version: V1.0.0
#

cd out/xm7206v12a
tar -czf rootfs.tgz rootfs/
mv rootfs.tgz ../../source/rootfs/scripts/