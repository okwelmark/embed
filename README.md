# CoCIS ESP32 Smart Control

ESP32 smart-control demo built with PlatformIO and ESP-IDF. The firmware models an entrance terminal and two rooms using FreeRTOS tasks, directional PIR sensor pairs, room lights, door servos, and an SPI microSD card logger.

## Features

- Tracks people entering and leaving through serial CLI commands.
- Counts room entries and exits from two PIR sensors at each doorway.
- Turns each room light on while its occupancy count is greater than zero.
- Logs entrance and room-light events to `/sdcard/log.csv`.
- Includes a Wokwi circuit diagram for ESP32 simulation.

## Hardware

| Function | GPIO |
| --- | ---: |
| Room 1 entry PIR | 32 |
| Room 1 exit PIR | 33 |
| Room 2 entry PIR | 34 |
| Room 2 exit PIR | 35 |
| Room 1 light | 27 |
| Room 2 light | 14 |
| Door-in servo | 25 |
| Door-out servo | 26 |
| SD card CS | 5 |
| SD card MOSI | 23 |
| SD card MISO | 19 |
| SD card SCK | 18 |

Each room starts with entry and exit counters at zero in RAM. An entry PIR edge increments the entry counter immediately. An exit PIR edge increments the exit counter only when it is lower than the entry counter, so exits can never exceed entries. The light is on exactly while `entries > exits`. Event logs are queued for the SD card without blocking the room tasks. Servo control and RFID validation are marked as TODOs in `src/main.c`.

## Build and upload

Install PlatformIO, connect an ESP32 DevKit board, then run:

```sh
pio run
pio run --target upload
pio device monitor
```

The serial monitor uses `115200` baud.

## Serial commands

```text
tap in <uid>
tap out <uid>
status
```

For example:

```text
tap in 1234
status
tap out 1234
```

## Wokwi

The circuit definition is in `diagram.json`. The PlatformIO build artifacts expected by `wokwi.toml` are generated under `.pio/build/esp32dev/`.
