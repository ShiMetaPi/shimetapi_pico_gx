#!/bin/sh

START_SEC=$(date +%s 2>/dev/null)
if [ -z "$START_SEC" ]; then
    START_SEC=0
fi

GPIO_NUM="${FACTORY_LED_GPIO:-14}"
GPIO_REGISTER_ADDR="${FACTORY_LED_GPIO_REGISTER_ADDR:-0x11980018}"
GPIO_REGISTER_VALUE="${FACTORY_LED_GPIO_REGISTER_VALUE:-0x00001000}"
BLINK_COUNT="${FACTORY_LED_BLINK_COUNT:-3}"
BLINK_MS="${FACTORY_LED_BLINK_MS:-500}"
GPIO_DIR="/sys/class/gpio/gpio${GPIO_NUM}"
LOG_FILE="/tmp/factory_led_error.log"

status="FAIL"
message="LED GPIO blink failed"

: > "$LOG_FILE"

json_escape() {
    printf '%s' "$1" | tr '\n\r\t\"' '    '
}

write_gpio_file() {
    file="$1"
    value="$2"
    echo "$value" > "$file" 2>>"$LOG_FILE"
}

sleep_ms() {
    ms="$1"
    if command -v usleep >/dev/null 2>&1; then
        usleep $((ms * 1000))
    else
        sleep 1
    fi
}

init_led_register() {
    if [ -z "$GPIO_REGISTER_ADDR" ] || [ -z "$GPIO_REGISTER_VALUE" ]; then
        return 0
    fi

    if ! command -v xmmm >/dev/null 2>&1; then
        echo "xmmm not found, skip register init" >>"$LOG_FILE"
        return 0
    fi

    # xmmm 会向 stdout 打印类似 "0x11980018: ..." 的信息。
    # stdout 必须只输出 JSON，所以这里必须把 xmmm 的 stdout/stderr 全部重定向到日志文件。
    if ! xmmm "$GPIO_REGISTER_ADDR" "$GPIO_REGISTER_VALUE" >>"$LOG_FILE" 2>&1; then
        message="LED register init failed: $(cat "$LOG_FILE" 2>/dev/null)"
        return 1
    fi

    return 0
}

if init_led_register; then
    if [ ! -d /sys/class/gpio ]; then
        message="/sys/class/gpio not found"
    else
        if [ ! -d "$GPIO_DIR" ]; then
            echo "$GPIO_NUM" > /sys/class/gpio/export 2>>"$LOG_FILE"
            sleep 1
        fi

        if [ ! -d "$GPIO_DIR" ]; then
            message="gpio${GPIO_NUM} export failed: $(cat "$LOG_FILE" 2>/dev/null)"
        elif [ ! -w "$GPIO_DIR/direction" ] || [ ! -w "$GPIO_DIR/value" ]; then
            message="gpio${GPIO_NUM} direction/value not writable"
        else
            if write_gpio_file "$GPIO_DIR/direction" "out"; then
                i=0
                ok=1
                while [ $i -lt "$BLINK_COUNT" ]; do
                    if ! write_gpio_file "$GPIO_DIR/value" "1"; then
                        ok=0
                        break
                    fi
                    sleep_ms "$BLINK_MS"

                    if ! write_gpio_file "$GPIO_DIR/value" "0"; then
                        ok=0
                        break
                    fi
                    sleep_ms "$BLINK_MS"
                    i=$((i + 1))
                done

                if [ "$ok" = "1" ]; then
                    status="MANUAL"
                    message="LED GPIO blink command sent: gpio=$GPIO_NUM count=$BLINK_COUNT interval=${BLINK_MS}ms"
                else
                    message="gpio${GPIO_NUM} write failed: $(cat "$LOG_FILE" 2>/dev/null)"
                fi
            else
                message="gpio${GPIO_NUM} set direction failed: $(cat "$LOG_FILE" 2>/dev/null)"
            fi
        fi
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

message_json=$(json_escape "$message")

printf '{\n'
printf '  "item": "led",\n'
printf '  "name": "LED",\n'
printf '  "status": "%s",\n' "$status"
printf '  "message": "%s",\n' "$message_json"
printf '  "duration_ms": %s\n' "$DURATION_MS"
printf '}\n'

[ "$status" = "MANUAL" ]
