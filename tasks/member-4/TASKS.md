# Member 4 — SD logging, schedule, and shared SPI

## Ownership

- `src/storage.c`
- `src/schedule.c`
- `src/include/storage.h`
- `src/include/schedule.h`
- `src/include/shared_spi.h`

Do not edit access, coordinator, or CLI implementation files. Coordinate any SD/RFID pin or driver decision with Member 1 before integrating.

## Work

- Implement a single SD writer task/queue and event CSV append/readback.
- Persist schedule configuration in NVS and validate weekday/minute bounds.
- Implement the schedule query used for entry and exit; invalid wall-clock time must deny access.
- Initialize the shared SPI bus once and document/register the SD and RFID devices without duplicate bus initialization.
- Surface mount, queue-full, read, and write failures; do not silently claim logs were persisted.

## Interfaces

Provides `storage.h`, `schedule.h`, and `shared_spi.h`.

Depends on `system_types.h`. CLI authenticates staff before calling `schedule_set`; access calls `schedule_allows` for both directions. The coordinator/event path supplies log records to storage.

## Definition of done and test

- SD event records can be appended and recent records read back.
- Schedule survives restart and rejects invalid values.
- Test before, at, during, and after the configured window for both entry and exit, including invalid time.
- Verify single SPI host ownership with both card and RFID device configurations; report simulator limits or card failures explicitly.

## Video explanation

Show the SD writer queue, shared SPI ownership, NVS schedule persistence, and an allowed/denied access-window boundary.

## Report section

Write a short section describing SD logging, schedule persistence/enforcement, tests performed, and storage/time-source limitations. Give it to Member 5 for the final report.
