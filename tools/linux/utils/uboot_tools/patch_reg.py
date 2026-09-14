#!/usr/bin/env python3
"""给 regbin 生成的 .reg 板级寄存器表打补丁（改值 / 增记录）。

用法:  patch_reg.py <.reg 文件> <补丁文件>

.reg 格式（regbin 产物）:
    0x00..0x7F  头部: "BRHD" + 版本 + 生成时间 + 源 xlsm 名
    之后        每条 16 字节: [地址][值][保留][掩码=0xff]
    末尾        一条全零记录作终止符

补丁文件每行:  <addr> <value>    # 注释
    - 地址已存在 -> 原地改值（保持原有顺序）
    - 地址不存在 -> 插到终止符之前
    幂等: 重复执行结果一致。

回退: 删掉对应的补丁文件即可, Makefile 钩子会跳过, .reg 恢复 regbin 原样。
"""
import os
import struct
import sys

HDR_SIZE = 0x80
REC_SIZE = 16


def main():
    if len(sys.argv) != 3:
        sys.exit("用法: patch_reg.py <.reg 文件> <补丁文件>")
    reg_path, patch_path = sys.argv[1], sys.argv[2]

    data = open(reg_path, "rb").read()
    if data[:4] != b"BRHD":
        sys.exit("[reg-patch] %s 不是 BRHD 格式的 reg 文件" % reg_path)
    if len(data) < HDR_SIZE or (len(data) - HDR_SIZE) % REC_SIZE:
        sys.exit("[reg-patch] %s 大小异常: %d" % (reg_path, len(data)))

    hdr, body = data[:HDR_SIZE], data[HDR_SIZE:]
    recs = [list(struct.unpack_from("<IIII", body, i * REC_SIZE))
            for i in range(len(body) // REC_SIZE)]

    patches = []
    with open(patch_path) as fp:
        for lineno, raw in enumerate(fp, 1):
            line = raw.split("#")[0].strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) != 2:
                sys.exit("[reg-patch] %s:%d 格式应为 <addr> <value>" % (patch_path, lineno))
            try:
                patches.append((int(parts[0], 0), int(parts[1], 0)))
            except ValueError:
                sys.exit("[reg-patch] %s:%d 数值解析失败: %s" % (patch_path, lineno, line))

    # 终止符 = 末尾的全零记录, 新记录要插在它前面
    end = len(recs)
    while end > 0 and recs[end - 1][0] == 0 and recs[end - 1][1] == 0:
        end -= 1

    changed = 0
    for addr, val in patches:
        idx = next((i for i in range(end) if recs[i][0] == addr), None)
        if idx is None:
            recs.insert(end, [addr, val, 0, 0xFF])
            end += 1
            changed += 1
            print("  [reg-patch] +  %#010x = %#x  (新增)" % (addr, val))
        elif recs[idx][1] != val:
            print("  [reg-patch] ~  %#010x = %#x  (原值 %#x)" % (addr, val, recs[idx][1]))
            recs[idx][1] = val
            changed += 1
        else:
            print("  [reg-patch] =  %#010x = %#x  (已一致, 跳过)" % (addr, val))

    open(reg_path, "wb").write(hdr + b"".join(struct.pack("<IIII", *r) for r in recs))
    print("  [reg-patch] %s: 共 %d 条记录, 本次改动 %d 条 (补丁: %s)"
          % (os.path.basename(reg_path), len(recs), changed, os.path.basename(patch_path)))


main()
