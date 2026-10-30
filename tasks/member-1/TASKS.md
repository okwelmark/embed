# Member 1 — Entrance access and door hardware

## Ownership

- `src/access.c`
- `src/include/access.h` (shared start API)
- Entrance/RFID/servo wiring changes in `diagram.json`

Do not edit coordinator, schedule, storage, or CLI implementation files. Raise API changes before changing shared headers.

## Work

- Initialize and read the RFID reader; validate card IDs against the agreed demo allow-list.
- Distinguish entry and exit reader/door paths.
- Use `schedule_allows()` for both directions and submit the result through `coordinator_access_commit()`.
- Actuate a door only when the coordinator returns a granted decision; report denied scans for audit.
- Publish sequence-numbered access outcomes and surface reader, coordinator, and queue errors.

## Interfaces

Provides `access_start()` from `src/include/access.h`.

Depends on `schedule.h`, `coordinator.h`, `storage.h`, and `shared_spi.h`. The coordinator owns the authoritative inside-UID ledger; this module must not update a second occupancy count.

## Definition of done and Wokwi test

- A known UID is accepted only when policy permits; an unknown UID is denied.
- Entry and exit drive only their corresponding servo.
- Outside the configured window, both entry and exit are denied, including for a UID currently inside.
- Denied or failed scans do not change the inside ledger; access outcomes are auditable.
- Verify RFID/servo parts and pin assignments in Wokwi; document any part or multi-board simulation limitation.

## Video explanation

Show how a scanned UID and direction reach the policy/coordinator decision, why the door is not actuated on denial, and how the RFID/SPI and servo PWM interfaces work.

## Report section

Write a short section describing the entrance hardware, access decision flow, tests performed, and any RFID/Wokwi limitations. Give it to Member 5 for the final report.
