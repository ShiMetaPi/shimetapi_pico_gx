# make distclean
make CROSS_COMPILE=aarch64-gcc12.2.0-linux- PLAT=xmfalcon DEBUG=1 NEED_BL32=no SPD=none bl31
