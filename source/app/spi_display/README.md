```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""SPI 屏显示(ST7789 240x240 RGB565,板端 Python3,/dev/spidev2.0 + chardev GPIO + /dev/mem 改 pad mux)。

用法: sudo python3 spi_display.py
模块: from spi_display import ST7789, set_pad_func

注: 本平台 PL022 TX-only 会 overrun,故 SPI 用 SPI_IOC_MESSAGE 全双工(rx dummy);
    TX FIFO <128,flush 分片 16B/片。Python 逐片 ioctl,整屏刷新较慢(~1-2s)。
    板端 Python 无 _ctypes(ctypes 不可用),spi_ioc_transfer 的指针用 array.buffer_info() 取。
"""
import array
import fcntl
import mmap
import os
import struct
import time


def _ioc(d, t, nr, size):
    return ((d & 3) << 30) | ((size & 0x3FFF) << 16) | ((t & 0xFF) << 8) | (nr & 0xFF)


# ---- GPIO chardev v1 ABI(DC/RES/CS 用)----
_GPIO_GET_HANDLE = _ioc(3, 0xB4, 0x03, 364)   # struct gpiohandle_request 364B
_GPIO_SET_VALUES = _ioc(3, 0xB4, 0x09, 64)    # struct gpiohandle_data 64B
_GREQ, _GDATA = 364, 64
_GPIO_OUT = 1 << 1

# ---- SPI spidev(MODE3/8bit/24MHz)----
SPI_IOC_WR_MODE = _ioc(1, 0x6B, 1, 1)         # 0x40106B01
SPI_IOC_WR_BITS = _ioc(1, 0x6B, 3, 1)         # 0x40106B03
SPI_IOC_WR_SPEED = _ioc(1, 0x6B, 4, 4)        # 0x40046B04
SPI_IOC_MESSAGE_1 = _ioc(1, 0x6B, 0, 32)      # 0x40206B00(一条 spi_ioc_transfer=32B)
SPI_SPEED = 24000000

# pad 复用(查 7206V11A PIN_OUT 表3,每脚 func 不同)
_PADS = (
    (0x100C0028, 0x1004),  # SCK  GPIO4_7 func4=SPI2_SCLK
    (0x100C002C, 0x1004),  # MOSI GPIO5_0 func4=SPI2_SDO
    (0x100C0030, 0x1000),  # CS   GPIO5_1 func0=GPIO(手动 framing)
    (0x100C0020, 0x1005),  # DC   GPIO4_5 func5=GPIO(默认 JTAG_TDO!)
    (0x100C001C, 0x1005),  # RES  GPIO4_4 func5=GPIO(默认 JTAG_TDI!)
)


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
    """单 GPIO line 的 chardev 句柄(驱动 DC/RES/CS)。"""

    def __init__(self, chip, line, default=0, label="spi-gpio"):
        self._chip_fd = os.open(chip, os.O_RDONLY)
        try:
            self._line_fd = self._req(chip, line, default, label)
        except BaseException:
            self.close()
            raise

    def set_value(self, v):
        data = bytearray(_GDATA)
        data[0] = 1 if v else 0
        fcntl.ioctl(self._line_fd, _GPIO_SET_VALUES, data)

    def _req(self, chip, line, default, label):
        r = bytearray(_GREQ)
        struct.pack_into('<I', r, 0, line)                 # lineoffsets[0]
        struct.pack_into('<I', r, 256, _GPIO_OUT)          # flags=OUTPUT
        r[260] = 1 if default else 0                       # default_values[0]
        lbl = label.encode('ascii', 'replace')[:31]
        r[324:324 + len(lbl)] = lbl                        # consumer_label
        struct.pack_into('<I', r, 356, 1)                  # lines=1
        fcntl.ioctl(self._chip_fd, _GPIO_GET_HANDLE, r)
        fd = struct.unpack_from('<i', r, 360)[0]           # kernel 填回
        if fd <= 0:
            raise OSError(f"GPIO 申请失败 chip={chip} line={line}")
        return fd

    def close(self):
        for attr in ("_line_fd", "_chip_fd"):
            fd = getattr(self, attr, -1)
            if fd >= 0:
                os.close(fd)
                setattr(self, attr, -1)


# 8x16 ASCII 点阵(0x20~0x7E 共 95 字符;由 font8x16.h 生成,与 oled_display 同源)
_FONT = bytes.fromhex(
    "0000000000000000000000000000000000000010101010101000100000000000"
    "0000002828280000000000000000000000000014247e2828fc48500000000000"
    "0000103c505038141478101000000000000000e0a0e41820dc141c0000000000"
    "000000382020305a4a443e000000000000000010101000000000000000000000"
    "0000102020202020202020100000000000002020101010101010202000000000"
    "000000105438385410000000000000000000000010107c101000000000000000"
    "0000000000101020000000000000000000000000003800000000000000000000"
    "0000000000001010000000000000000000000408081010102020400000000000"
    "0000003c66424a4242663c0000000000000000701010101010107c0000000000"
    "0000003c4202060c18207e00000000000000003c42023c0602423c0000000000"
    "0000000c0c1424647e040400000000000000007c40407c0602027c0000000000"
    "0000001e20405c6242423c00000000000000007e040408081010200000000000"
    "0000003c42423c4242423c00000000000000003c4242423e0204780000000000"
    "0000000010100000101000000000000000000010100000101020000000000000"
    "00000000021c6038060000000000000000000000fc00fc000000000000000000"
    "000000004038061c600000000000000000000038040c18101000100000000000"
    "00001c26424e52524e60201c0000000000000018181824243c42420000000000"
    "0000007c42427c4242427c00000000000000001c2240404040221c0000000000"
    "000000784442424242447800000000000000007e40407e4040407e0000000000"
    "0000007e40407e4040404000000000000000001c2240404642221c0000000000"
    "0000004242427e4242424200000000000000007c1010101010107c0000000000"
    "0000001c04040404044438000000000000000044485060504844420000000000"
    "000000404040404040407e00000000000000004266665a5a4242420000000000"
    "000000426252524a4a464200000000000000003c6642424242663c0000000000"
    "0000007c4242427c404040000000000000003c6642424242663c060000000000"
    "0000007c4242427c44424100000000000000003c4240780602423c0000000000"
    "000000fe101010101010100000000000000000424242424242423c0000000000"
    "00000042422424241818180000000000000000829292aa6c6c44440000000000"
    "00000042242418182424420000000000000000c6442838101010100000000000"
    "0000007e0404081030207e000000000000003020202020202020203000000000"
    "0000402020101010080804000000000000003010101010101010103000000000"
    "0000003048840000000000000000000000000000000000fe0000000000000000"
    "000010080000000000000000000000000000000078043c44443c000000000000"
    "00004040407844444444780000000000000000003c604040603c000000000000"
    "00000404043c444444443c00000000000000000038447c40403c000000000000"
    "00000c10107c101010101000000000000000003c444444443c04380000000000"
    "00004040405864444444440000000000000010000070101010107c0000000000"
    "0010000070101010101010600000000000004040404850605048440000000000"
    "0000e020202020202020180000000000000000007c5454545454000000000000"
    "0000000058644444444400000000000000000000384444444438000000000000"
    "000000784444444478404000000000000000003c444444443c04040000000000"
    "000000003c2420202020000000000000000000003c40700c0478000000000000"
    "0000002020f8202020203800000000000000000044444444443c000000000000"
    "00000000444428282810000000000000000000000404a8a85050000000000000"
    "000000006c281010286c00000000000000000044482828301020600000000000"
    "000000007c081830207c00000000000000001c10101060101010101c00000000"
    "001010101010101010101010000000000000701010100c101010107000000000"
    "0000000000700e000000000000000000"
)

# ST7789 gamma 校正(正/负),照搬 st7789.c
_GMP = bytes([0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54,
              0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23])
_GMN = bytes([0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44,
              0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23])

# RGB565 颜色
BLACK, WHITE = 0x0000, 0xFFFF
RED, GREEN, BLUE = 0xF800, 0x07E0, 0x001F
YELLOW, CYAN = 0xFFE0, 0x07FF


class ST7789:
    """240x240 RGB565 TFT 驱动(/dev/spidev2.0 全双工 + chardev GPIO 控 DC/RES/CS)。

    帧缓冲按 wire 序(高字节先)存,flush 直接整屏送;绘图文 (x,y)->fb[(y*240+x)*2..+1]。
    """

    W = H = 240

    def __init__(self):
        for pad, func in _PADS:
            set_pad_func(pad, func)
        self._dc = GPIOClient("/dev/gpiochip4", 5, default=0, label="tft-dc")
        self._res = GPIOClient("/dev/gpiochip4", 4, default=1, label="tft-res")
        self._cs = GPIOClient("/dev/gpiochip5", 1, default=1, label="tft-cs")
        self._fd = os.open("/dev/spidev2.0", os.O_RDWR)
        fcntl.ioctl(self._fd, SPI_IOC_WR_MODE, bytearray([3]))       # MODE3
        fcntl.ioctl(self._fd, SPI_IOC_WR_BITS, bytearray([8]))
        fcntl.ioctl(self._fd, SPI_IOC_WR_SPEED, struct.pack('<I', SPI_SPEED))
        self._fb = bytearray(self.W * self.H * 2)
        self._reset()
        self._init()

    def _xfer(self, data):
        n = len(data)
        tx = array.array('B', data)                 # tx 缓冲
        rx = array.array('B', bytes(n))             # rx dummy(全双工,防 overrun)
        tr = struct.pack('<QQIIHBBBB',              # spi_ioc_transfer(30B)
                         tx.buffer_info()[0], rx.buffer_info()[0],
                         n, SPI_SPEED, 0, 8, 0, 0, 0) + b'\x00\x00'   # +2B pad=32B
        self._cs.set_value(0)
        fcntl.ioctl(self._fd, SPI_IOC_MESSAGE_1, tr)
        self._cs.set_value(1)

    def _cmd(self, c, data=b""):
        self._dc.set_value(0)                       # DC=0 命令
        self._xfer(bytes([c]))
        if data:
            self._dc.set_value(1)                   # DC=1 数据
            self._xfer(bytes(data))

    def _reset(self):
        for v in (1, 0, 1):
            self._res.set_value(v)
            time.sleep(0.01)

    def _init(self):
        self._cmd(0x11)                              # SLPOUT
        time.sleep(0.12)                            # datasheet: 须等 120ms
        for c, d in ((0x36, b"\x00"), (0x3A, b"\x05"),               # MADCTL/COLMOD(16bit)
                     (0xB2, b"\x0c\x0c\x00\x33\x33"), (0xB7, b"\x35"),
                     (0xBB, b"\x19"), (0xC0, b"\x2C"), (0xC2, b"\x01"),
                     (0xC3, b"\x0B"), (0xC4, b"\x20"), (0xC6, b"\x0F"),
                     (0xD0, b"\xA4\xA1"), (0xE0, _GMP), (0xE1, _GMN)):
            self._cmd(c, d)
        self._cmd(0x21)                             # INVON(多数模组需要,否则偏暗)
        self._cmd(0x29)                             # DISPON
        time.sleep(0.02)
        self.clear()
        self.flush()

    def on(self, v=True):
        self._cmd(0x29 if v else 0x28)

    def clear(self):
        self._fb[:] = b"\x00" * len(self._fb)

    def pixel(self, x, y, color=WHITE):
        if 0 <= x < self.W and 0 <= y < self.H:
            i = (y * self.W + x) * 2
            self._fb[i] = color >> 8
            self._fb[i + 1] = color & 0xFF

    def fill_rect(self, x, y, w, h, color):
        hi, lo = color >> 8, color & 0xFF
        for yy in range(y, y + h):
            if not (0 <= yy < self.H):
                continue
            base = (yy * self.W + x) * 2
            for k in range(w * 2):
                self._fb[base + k] = hi if k % 2 == 0 else lo

    def char(self, x, y, ch, fg=WHITE, bg=BLACK):
        o = (ord(ch) - 0x20) * 16 if 0x20 <= ord(ch) <= 0x7E else 0
        for r in range(16):
            line = _FONT[o + r]
            for c in range(8):
                self.pixel(x + c, y + r, fg if line & (0x80 >> c) else bg)

    def text(self, x, y, s, fg=WHITE, bg=BLACK):
        cx, cy = x, y
        for ch in s:
            if ch == "\n" or cx + 8 > self.W:
                cx, cy = x, cy + 16
                if ch == "\n":
                    continue
            self.char(cx, cy, ch, fg, bg)
            cx += 8

    def flush(self):
        self._cmd(0x2A, bytes([0, 0, (self.W - 1) >> 8, (self.W - 1) & 0xFF]))  # CASET
        self._cmd(0x2B, bytes([0, 0, (self.H - 1) >> 8, (self.H - 1) & 0xFF]))  # RASET
        self._dc.set_value(0)
        self._xfer(b"\x2C")                         # RAMWR
        self._dc.set_value(1)                       # 之后全数据
        fb = self._fb
        for i in range(0, len(fb), 16):             # 16B/片(TX FIFO<128),片间喘息
            self._xfer(fb[i:i + 16])
            time.sleep(0.0001)

    def close(self):
        if getattr(self, "_fd", -1) >= 0:
            os.close(self._fd)
            self._fd = -1
        for g in (getattr(self, n, None) for n in ("_dc", "_res", "_cs")):
            if g:
                g.close()

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()


def main():
    with ST7789() as tft:
        print("ST7789 初始化完成,显示欢迎画面,Ctrl+C 退出", flush=True)
        tft.fill_rect(16, 16, 48, 48, RED)
        tft.fill_rect(tft.W - 64, 16, 48, 48, GREEN)
        tft.fill_rect(tft.W // 2 - 24, tft.H // 2 - 24, 48, 48, BLUE)
        tft.text(8, tft.H - 40, "ST7789 240x240", WHITE, BLACK)
        tft.text(8, tft.H - 22, "spidev2.0 OK", YELLOW, BLACK)
        tft.flush()
        time.sleep(2)
        try:
            while True:
                up = float(open("/proc/uptime").read().split()[0])
                tft.clear()
                tft.text(0, 0, "SPI  Python", CYAN, BLACK)
                tft.text(0, 24, f"UP {up:.0f}s", WHITE, BLACK)
                tft.fill_rect(0, 60, 120, 120, RED)
                tft.fill_rect(120, 60, 120, 120, GREEN)
                tft.flush()
                time.sleep(1)
        except KeyboardInterrupt:
            tft.clear()
            tft.flush()
            tft.on(False)
            print("\n退出", flush=True)


if __name__ == "__main__":
    main()
```
