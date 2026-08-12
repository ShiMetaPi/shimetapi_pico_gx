#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
8x16 ASCII 点阵字模生成器（编译期辅助工具，板端不依赖 Python/PIL）。

用本机等宽字体把 0x20~0x7E 共 95 个可打印 ASCII 字符渲染成 8 列 x 16 行的
位图，每字符 16 字节，每字节对应一行像素，最高位(MSB)为最左侧像素。
生成的 C 数组写入 font8x16.h，由 ssd1306.c 直接索引使用。

用法：python3 gen_font8x16.py > font8x16.h
"""
import sys

from PIL import Image, ImageDraw, ImageFont

W, H = 8, 16
FIRST = 0x20          # 第一个字符（空格）
LAST = 0x7E           # 最后一个字符（'~'）
COUNT = LAST - FIRST + 1

# 优先用等宽字体，点阵更整齐；找不到则回退到默认字体。
CANDIDATES = [
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
]
font = None
for path in CANDIDATES:
    try:
        font = ImageFont.truetype(path, 11)
        break
    except Exception:
        continue
if font is None:
    font = ImageFont.load_default()


def render(ch):
    """渲染单字符，返回 16 字节（每字节一行，MSB=左像素）。"""
    img = Image.new("1", (W, H), 0)
    d = ImageDraw.Draw(img)
    bbox = d.textbbox((0, 0), ch, font=font)
    tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
    x = (W - tw) // 2 - bbox[0]
    y = (H - th) // 2 - bbox[1] - 1
    d.text((x, y), ch, fill=1, font=font)
    rows = []
    for r in range(H):
        b = 0
        for c in range(W):
            if img.getpixel((c, r)):
                b |= (0x80 >> c)
        rows.append(b)
    return rows


def main():
    out = []
    out.append("/*")
    out.append(" * font8x16.h - 8x16 ASCII 点阵字模（自动生成，请勿手改）")
    out.append(" * 生成工具: gen_font8x16.py  |  字符范围: 0x%02X~0x%02X (%d 个)" % (FIRST, LAST, COUNT))
    out.append(" * 排列: 每字符 %d 字节, 每字节 = 一行 8 像素, bit7(MSB) = 最左像素。" % H)
    out.append(" */")
    out.append("#ifndef FONT8X16_H")
    out.append("#define FONT8X16_H")
    out.append("")
    out.append("#include <stdint.h>")
    out.append("")
    out.append("#define FONT8X16_W       %d" % W)
    out.append("#define FONT8X16_H_      %d" % H)
    out.append("#define FONT8X16_FIRST   0x%02X" % FIRST)
    out.append("#define FONT8X16_COUNT   %d" % COUNT)
    out.append("")
    out.append("static const uint8_t font8x16[FONT8X16_COUNT][FONT8X16_H_] = {")
    for code in range(FIRST, LAST + 1):
        ch = chr(code)
        rows = render(ch)
        hexs = ", ".join("0x%02X" % v for v in rows)
        label = ch if ch.isalnum() or ch == " " else "."
        cmt = " " if ch == " " else ch
        out.append("    { %s }, /* 0x%02X '%s' */" % (hexs, code, cmt))
    out.append("};")
    out.append("")
    out.append("#endif /* FONT8X16_H */")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
