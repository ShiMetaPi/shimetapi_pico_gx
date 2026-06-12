#!/bin/sh

START_SEC=$(date +%s 2>/dev/null)
if [ -z "$START_SEC" ]; then
    START_SEC=0
fi

status="FAIL"
message="sensor video device not found"

for dev in /dev/video0 /dev/video1 /dev/video2; do
    if [ -c "$dev" ]; then
        status="PASS"
        message="video device found: $dev"
        break
    fi
done

END_SEC=$(date +%s 2>/dev/null)
if [ -z "$END_SEC" ]; then
    END_SEC=$START_SEC
fi
DURATION_MS=$(((END_SEC - START_SEC) * 1000))
if [ $DURATION_MS -lt 0 ]; then
    DURATION_MS=0
fi

# 输出JSON格式结果，后端读取stdout输出
printf '{\n'
printf '  "item": "sensor",\n'
printf '  "name": "Sensor 出图",\n'
printf '  "status": "%s",\n' "$status"
printf '  "message": "%s",\n' "$message"
printf '  "duration_ms": %s\n' "$DURATION_MS"
printf '}\n'

[ "$status" = "PASS" ]
