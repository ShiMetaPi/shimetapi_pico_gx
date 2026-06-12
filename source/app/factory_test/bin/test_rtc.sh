#!/bin/sh

START_SEC=$(date +%s 2>/dev/null)
if [ -z "$START_SEC" ]; then
    START_SEC=0
fi

rtc_dev="/dev/rtc0" 
status="FAIL"
message="RTC device not found"

if [ -e "$rtc_dev" ]; then
    if command -v hwclock >/dev/null 2>&1; then
        rtc_out=$(hwclock -r 2>&1)
        ret=$?
        if [ $ret -eq 0 ]; then
            status="PASS"
            message="RTC read ok: $rtc_out"
        else
            message="hwclock failed: $rtc_out"
        fi
    else
        status="PASS"
        message="RTC device exists: $rtc_dev"
    fi
fi

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
printf '  "item": "rtc",\n'
printf '  "name": "RTC 时钟",\n'
printf '  "status": "%s",\n' "$status"
printf '  "message": "%s",\n' "$message"
printf '  "duration_ms": %s\n' "$DURATION_MS"
printf '}\n'

[ "$status" = "PASS" ]
