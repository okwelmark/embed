# Smart Control and Monitoring System for CoCIS

## System Operation Report

**Institution:** Makerere University, College of Computing and Information Sciences (CoCIS)  
**Programmes:** BSE III and CS II  
**Academic year:** 2026/2027  
**Submission date:** [Insert submission date]

## 1. Introduction

The College of Computing and Information Sciences requires improved control of access to
valuable equipment and more efficient use of electricity in lecture rooms. This project
implements a pilot Smart Control and Monitoring System for the main entrance and two
lecture rooms.

The design combines an ESP32 microcontroller, PIR motion sensors, room-light outputs,
entrance and exit door servo connections, an SPI microSD card, and a serial management
interface. The system records building entry and exit events, estimates room occupancy,
switches room lights according to occupancy, and stores event records on the microSD card.

The current implementation uses one ESP32 with three logical terminals: an entrance
terminal, a Lecture Room 1 terminal, and a Lecture Room 2 terminal. The design can be
extended to separate ESP32 devices if the pilot is later distributed across multiple
physical locations.

## 2. Objectives

The project objectives are to:

1. Monitor entry to and exit from the building.
2. Maintain the number of people currently inside the building.
3. Monitor movement through the two lecture rooms.
4. Estimate room occupancy using separate entry and exit PIR sensors.
5. Switch each room light on while the room is occupied and off when it becomes empty.
6. Store access, occupancy, and lighting events on an SD card.
7. Provide a serial interface for testing and viewing system status.
8. Use synchronization mechanisms to protect shared state while concurrent FreeRTOS
   tasks are running.

## 3. System Architecture

The system is implemented as several FreeRTOS tasks running on an ESP32:

- **Entrance task:** Processes simulated RFID tap commands, updates the building
  occupancy count, and creates entry or exit log records.
- **Room 1 task:** Monitors the Lecture Room 1 entry and exit PIR sensors, updates
  occupancy counters, and controls the Room 1 light.
- **Room 2 task:** Performs the equivalent work for Lecture Room 2.
- **Logger task:** Mounts the microSD card and is the only task that writes to the log
  file.
- **CLI task:** Reads commands from the UART serial interface and sends entrance events
  to the entrance task or prints the current status.

The tasks communicate through FreeRTOS queues. A mutex protects the shared state
structure, which contains building occupancy, room counters, and light states. This
prevents simultaneous tasks from updating shared values inconsistently.

### 3.1 Data flow

1. A serial command is entered through the ESP32 UART.
2. The CLI task parses the command.
3. An entrance tap is placed in the entrance queue.
4. The entrance task updates the building occupancy under the shared-state mutex.
5. A formatted event is placed in the logging queue.
6. The logger task writes the event to `/sdcard/log.csv` and also reports it through
   the serial log.

For room monitoring, the room task polls its two PIR inputs every 50 milliseconds. A
low-to-high transition is treated as a sensor event. The task then updates the room
counters, determines the desired light state, changes the output if required, and sends
the event to the logger queue.

## 4. Hardware Components and Pin Assignment

The circuit is defined in `diagram.json`. The firmware pin constants are defined in
`src/main.c`.

| Component or function | ESP32 GPIO |
|---|---:|
| Lecture Room 1 entry PIR | 32 |
| Lecture Room 1 exit PIR | 33 |
| Lecture Room 2 entry PIR | 34 |
| Lecture Room 2 exit PIR | 35 |
| Lecture Room 1 light | 27 |
| Lecture Room 2 light | 14 |
| Entrance door servo | 25 |
| Exit door servo | 26 |
| SD card chip select | 5 |
| SD card MOSI/DI | 23 |
| SD card MISO/DO | 19 |
| SD card clock | 18 |

The PIR sensors are powered from the ESP32 5 V and ground connections. The room
lights are represented by LEDs in the simulation. The microSD card uses the SPI bus.
The two servos are connected in the diagram and are reserved for entrance and exit
door control.

## 5. Software Design

### 5.1 Shared state

The shared state contains:

- `inside`: number of people currently inside the building.
- `room_entries[2]`: cumulative entry events for each room.
- `room_exits[2]`: counted exit events for each room.
- `light[2]`: current logical light state for each room.

All updates to this state are protected by `S_mtx`, a FreeRTOS mutex.

### 5.2 Building entry and exit records

The serial interface accepts:

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

An entry increments `inside`. An exit decrements `inside` only when the value is
greater than zero, preventing the building count from becoming negative.

The current implementation treats the UID supplied in the command as a simulated RFID
identifier. It records the UID but does not yet validate it against an authorized-user
database.

### 5.3 Room occupancy and lighting

Each room has independent entry and exit PIR sensors. The task detects rising edges
and increments the appropriate entry or exit counter. An exit is counted only if the
number of exits is less than the number of entries. Therefore, an exit event cannot
produce a negative room occupancy.

The room occupancy calculation is:

```text
occupancy = room_entries - room_exits
```

The light is switched on when occupancy is greater than zero and switched off when
occupancy returns to zero. This reduces unnecessary energy consumption when the room
is empty.

### 5.4 Event logging

The logger task attempts to mount the SD card using the SPI interface. It appends
events to:

```text
/sdcard/log.csv
```

The file is initialized with the header `event` when it is empty. Example records
include:

```text
IN,1234,inside=1
OUT,1234,inside=0
ROOM1,IN,occupancy=1
ROOM1,OUT,occupancy=0
ROOM1,light=1
ROOM1,light=0
```

The logger task is the only task that accesses the SD card. Other tasks send messages
through `log_q`, reducing the risk of concurrent file-access conflicts.

### 5.5 Serial status output

The `status` command displays the building count, both rooms' entry and exit counts,
and their current light states. The serial monitor is configured for 115200 baud.

## 6. Build and Simulation

The project uses PlatformIO with the ESP-IDF framework and targets the `esp32dev`
board. The simulation circuit is described by `diagram.json`, while `wokwi.toml`
points Wokwi to the firmware and ELF files produced under
`.pio/build/esp32dev/`.

Typical commands are:

```text
pio run
pio run --target upload
pio device monitor
```

The Wokwi simulation link is intentionally left for the group to add:

**Wokwi project:** [Insert Wokwi project link]

## 7. Demonstration and Test Procedure

### 7.1 Building entry and exit

1. Start the firmware and open the serial monitor at 115200 baud.
2. Enter `tap in 1234`.
3. Enter `status` and confirm that `inside` has increased.
4. Enter `tap out 1234`.
5. Enter `status` and confirm that `inside` has decreased.
6. Confirm that the corresponding `IN` and `OUT` records appear in the serial log
   and, when the SD card is available, in `/sdcard/log.csv`.

### 7.2 Room occupancy and lighting

1. Trigger the Room 1 entry PIR and confirm a `ROOM1,IN` event.
2. Confirm that the Room 1 light output changes to on.
3. Trigger the Room 1 exit PIR and confirm a `ROOM1,OUT` event.
4. Confirm that the light turns off when the room occupancy reaches zero.
5. Repeat the procedure for Room 2.
6. Trigger an exit before any corresponding entry and confirm that it is recorded as
   `OUT_IGNORED` and does not create negative occupancy.

### 7.3 Status and logging

1. Use `status` after each test sequence.
2. Compare the displayed counts with the event records.
3. Verify that the SD card is mounted and that the logger reports successful writes.
4. If the SD card cannot be mounted, record this as a failed storage test rather than
   assuming that the event was persisted.

## 8. Requirements Compliance

| Coursework requirement | Current implementation |
|---|---|
| ESP32 and ESP-IDF | Implemented using PlatformIO and ESP-IDF |
| Building entry and exit monitoring | Implemented through serial tap commands |
| RFID identification | Simulated by the UID in a tap command; hardware validation is not implemented |
| Door activation | Servo connections are present, but servo movement is marked TODO |
| Two-room occupancy monitoring | Implemented with four PIR inputs |
| Automatic lighting | Implemented with two GPIO light outputs |
| SD-card data storage | Implemented through an SPI microSD logger |
| Serial management interface | Implemented with `tap in`, `tap out`, and `status` |
| Resource synchronization | Implemented with a mutex and queues |
| Configurable access schedule | Not yet implemented |
| Authenticated schedule configuration | Not yet implemented |
| Multiple physical ESP32 terminals | Current version uses one ESP32 with three logical terminals |

The report deliberately identifies incomplete features. In particular, RFID validation,
door servo actuation, configurable access schedules, authenticated configuration, and
multi-device communication remain extension points for a complete production system.

## 9. Limitations and Recommended Future Work

The following improvements would bring the pilot closer to the full coursework
specification:

1. Add a real RFID reader and an authorized-user table.
2. Validate every UID before granting access.
3. Add LEDC/PWM servo control to open the correct entrance or exit door.
4. Add a real-time clock or network time source for timestamped records.
5. Implement configurable access start and end times.
6. Restrict schedule changes to authenticated administrators.
7. Enforce the required policy for access outside the configured schedule.
8. Add timestamps, event type, UID, authorization result, and device identity to the
   CSV format.
9. Add queue-full and SD-write error handling that reports failed persistence clearly.
10. If the installation is physically distributed, add communication between the three
    ESP32 terminals and define a synchronized shared-state protocol.
11. Add debouncing or sensor qualification to reduce false counts caused by noisy or
    repeated PIR transitions.
12. Add a command to view recent access records and room events.

## 10. Individual Contributions

Complete this section with the actual work performed by each member:

| Member | Student number | Contribution |
|---|---|---|
| [Insert name] | [Insert ID] | [Insert contribution] |
| [Insert name] | [Insert ID] | [Insert contribution] |
| [Insert name] | [Insert ID] | [Insert contribution] |
| [Insert name] | [Insert ID] | [Insert contribution] |
| [Insert name] | [Insert ID] | [Insert contribution] |

Each contribution should identify the relevant design, code, simulation, testing, or
documentation work and should only claim work the member can explain during the
presentation.

## 11. Group Member Details

| No. | Name | Student number | Programme | Contact or other required detail |
|---:|---|---|---|---|
| 1 | [Insert name] | [Insert ID] | [BSE III/CS II] | [Insert detail] |
| 2 | [Insert name] | [Insert ID] | [BSE III/CS II] | [Insert detail] |
| 3 | [Insert name] | [Insert ID] | [BSE III/CS II] | [Insert detail] |
| 4 | [Insert name] | [Insert ID] | [BSE III/CS II] | [Insert detail] |
| 5 | [Insert name] | [Insert ID] | [BSE III/CS II] | [Insert detail] |

## 12. Video Demonstrations

Add one recording link for each group member. Each video should explain the member's
contribution and demonstrate the corresponding code or system feature.

| Member | Video link |
|---|---|
| [Insert name] | [Insert video link] |
| [Insert name] | [Insert video link] |
| [Insert name] | [Insert video link] |
| [Insert name] | [Insert video link] |
| [Insert name] | [Insert video link] |

## 13. Conclusion

The implemented prototype demonstrates the central monitoring and energy-saving
functions required for the CoCIS pilot. It tracks simulated building entry and exit,
monitors two lecture rooms using directional PIR sensors, controls room lights based
on calculated occupancy, stores events on an SD card, and coordinates concurrent
operations using FreeRTOS queues and a mutex.

The prototype provides a sound foundation for the full smart-control system. The
remaining work is concentrated in production access-control features: real RFID
validation, physical servo control, timestamps, configurable and authenticated access
schedules, and communication between separate physical terminals.
