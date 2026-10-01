# WS2812 LED Blink (ESP32-S3)

ESP-IDF firmware that blinks the on-board WS2812 RGB LED at 1 Hz
(500 ms on, 500 ms off) on an **ESP32-S3 DevKitC**.

## Hardware

- Board: ESP32-S3 DevKitC
- LED: on-board WS2812 addressable RGB LED on **GPIO 48**

## Project layout

```
.
├── CMakeLists.txt             # Top-level ESP-IDF project
├── partitions.csv             # Default partition table
├── main/
│   ├── CMakeLists.txt
│   ├── Kconfig.projbuild      # LED_GPIO, BLINK_INTERVAL_MS
│   ├── main.c                 # app_main: blink loop
│   └── ws2812.h               # RMT-based WS2812 helper (static)
└── test/
    ├── run_tests.sh           # Build + run host-based unit tests
    ├── test_ws2812.c          # Unit tests for ws2812.h
    ├── test_main.c            # Unit tests for main.c (app_main)
    └── mock/                  # Stub ESP/FreeRTOS/RMT headers
```

## Build & flash

```sh
# 1. Set target
idf.py set-target esp32s3

# 2. (Optional) Configure via menuconfig
idf.py menuconfig   # "LED Blink Configuration" -> set GPIO / interval

# 3. Build
idf.py build

# 4. Flash & monitor
idf.py -p PORT flash monitor
```

## Kconfig options

| Option               | Default | Description                                    |
|----------------------|---------|------------------------------------------------|
| `CONFIG_LED_GPIO`    | 48      | GPIO pin for the WS2812 data line.             |
| `CONFIG_BLINK_INTERVAL_MS` | 500 | Half-period of the blink (1 Hz with 500 ms). |

## Unit tests

Host-based unit tests live under `test/`. They use lightweight mock
headers (no ESP-IDF required) to compile `ws2812.h` and `main.c` on the
host and verify:

- GRB byte-order in `ws2812_set_rgb()`
- Encoder callback symbol generation (bit patterns, reset)
- RMT channel idempotent initialization
- Error propagation from every RMT driver call
- `app_main()` blink sequence (ON → OFF) and delay interval
- Startup log banner

```sh
sh test/run_tests.sh
```

## Design notes

- RMT TX channel is created once via `rmt_new_tx_channel()` and kept in
  a `static` handle. No dynamic allocation happens in the blink loop.
- Resolution: 10 MHz (100 ns/tick) → WS2812 timing constants expressed
  in ticks: T0H=4, T0L=5, T1H=4, T1L=5, RESET=2800 (280 µs).
- GRB byte order is enforced in `ws2812_set_rgb()`.
- RMT API used is the modern `rmt_new_*` API (ESP-IDF v5.0+).
