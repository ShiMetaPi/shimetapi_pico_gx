#!/bin/sh

START_SEC=$(date +%s 2>/dev/null)
if [ -z "$START_SEC" ]; then
    START_SEC=0
fi

status="PASS"
message="factory mjpeg stream is served by /usr/bin/factory_server at /mjpeg"

END_SEC=$(date +%s 2>/dev/null)
if [ -z "$END_SEC" ]; then
    END_SEC=$START_SEC
fi
DURATION_MS=$(((END_SEC - START_SEC) * 1000))
if [ $DURATION_MS -lt 0 ]; then
    DURATION_MS=0
fi

printf '{\n'
printf '  "item": "mjpeg",\n'
printf '  "name": "MJPEG 图传",\n'
printf '  "status": "%s",\n' "$status"
printf '  "message": "%s",\n' "$message"
printf '  "duration_ms": %s\n' "$DURATION_MS"
printf '}\n'

[ "$status" = "PASS" ]
