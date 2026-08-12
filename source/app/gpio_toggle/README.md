```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""GPIO1_6 翻转测试(板端 Python3,GPIO chardev v1 ABI + /dev/mem 改 pad mux)。

用法: sudo python3 gpio_toggle.py
模块: from gpio_toggle import GPIOClient, set_pad_func
"""
import fcntl
import mmap
import os
import struct
import time

# struct gpiohandle_request (364B): lineoffsets[64]@0 | flags@256 |
#   default_values[64]@260 | consumer_label[32]@324 | lines@356 | fd@360(kernel)
_HREQ_SIZE = 364
# struct gpiohandle_data: values[64] (64B)
_HDATA_SIZE = 64

_INPUT, _OUTPUT = 1 << 0, 1 << 1


def _ioc(d, t, nr, size):
    """_IOC 宏:((dir)<<30)|((size)<<16)|((type)<<8)|nr"""
    return ((d & 3) << 30) | ((size & 0x3FFF) << 16) | ((t & 0xFF) << 8) | (nr & 0xFF)

# _IOWR = 3, ioctl magic = 0xB4
_GET_HANDLE = _ioc(3, 0xB4, 0x03, _HREQ_SIZE)
_GET_VALUES = _ioc(3, 0xB4, 0x08, _HDATA_SIZE)
_SET_VALUES = _ioc(3, 0xB4, 0x09, _HDATA_SIZE)


def set_pad_func(phys_addr, func_val):
    """mmap /dev/mem 写 iocfg 寄存器,把 pad 切到目标功能。"""
    PAGE = 0x1000
    fd = os.open("/dev/mem", os.O_RDWR)
    try:
        m = mmap.mmap(fd, PAGE, mmap.MAP_SHARED,
                      mmap.PROT_READ | mmap.PROT_WRITE,
                      offset=phys_addr & ~(PAGE - 1))
        off = phys_addr & (PAGE - 1)
        m[off:off + 4] = struct.pack('<I', func_val & 0xFFFFFFFF)
        m.close()
    finally:
        os.close(fd)


class GPIOClient:
    """单 GPIO line 的 chardev 句柄(chip=/dev/gpiochipN, line=line 偏移)。"""

    def __init__(self, chip, line, output=True, default=0, label="python-gpio"):
        self._chip_fd = os.open(chip, os.O_RDONLY)
        try:
            self._line_fd = self._request(line, output, default, label)
        except BaseException:
            self.close()
            raise

    def set_value(self, value):
        data = bytearray(_HDATA_SIZE)
        data[0] = 1 if value else 0
        fcntl.ioctl(self._line_fd, _SET_VALUES, data)

    def get_value(self):
        data = bytearray(_HDATA_SIZE)
        fcntl.ioctl(self._line_fd, _GET_VALUES, data)
        return data[0]

    def _request(self, line, output, default, label):
        req = bytearray(_HREQ_SIZE)
        struct.pack_into('<I', req, 0, line)                    # lineoffsets[0]
        struct.pack_into('<I', req, 256, _OUTPUT if output else _INPUT)
        req[260] = 1 if default else 0                          # default_values[0]
        lbl = label.encode('ascii', 'replace')[:31]
        req[324:324 + len(lbl)] = lbl                           # consumer_label
        struct.pack_into('<I', req, 356, 1)                     # lines
        fcntl.ioctl(self._chip_fd, _GET_HANDLE, req)
        fd = struct.unpack_from('<i', req, 360)[0]              # kernel 填回
        if fd <= 0:
            raise OSError(f"GPIO_GET_LINEHANDLE_IOCTL 返回无效 fd={fd}")
        return fd

    def close(self):
        for attr in ("_line_fd", "_chip_fd"):
            fd = getattr(self, attr, -1)
            if fd >= 0:
                os.close(fd)
                setattr(self, attr, -1)

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()


# GPIO1_6 = SFC_MISO_IO1: iocfg_reg29 @0x11980018, func0=GPIO, 0x1000=输入使能|func
_GPIO1_6_PAD = 0x11980018
_GPIO1_6_FUNC = 0x1000


def main():
    set_pad_func(_GPIO1_6_PAD, _GPIO1_6_FUNC)
    with GPIOClient("/dev/gpiochip1", 6, output=True, default=0,
                    label="pin_GPIO1_6") as gpio:
        print("GPIO1_6 每 1s 翻转,Ctrl+C 退出", flush=True)
        v = 0
        try:
            while True:
                gpio.set_value(v)
                print(f"GPIO1_6 = {v}", flush=True)
                v ^= 1
                time.sleep(1)
        except KeyboardInterrupt:
            print("\n退出", flush=True)


if __name__ == "__main__":
    main()
```
