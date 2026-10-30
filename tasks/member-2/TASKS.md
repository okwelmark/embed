# Member 2 — Room occupancy and lighting

## Ownership

- `src/rooms.c`
- `src/include/rooms.h` (shared start API)

Do not edit coordinator or other member modules. Raise API changes before changing shared headers.

## Work

- Implement independent PIR sampling and light control for both rooms.
- Track occupancy separately from the light's hold-on timeout.
- Publish room state transitions through the coordinator/event bus.
- Measure cumulative light-on time for the energy-saving demonstration if the core behavior is complete.

## Interfaces

Provides `rooms_start()` from `src/include/rooms.h`.

Depends on `coordinator.h` and `system_types.h`. Shared room state is read from the coordinator snapshot, not from module globals.

## Definition of done and Wokwi test

- Each PIR controls only its room's light.
- Motion turns a light on; continuous motion refreshes the hold time; after motion stops the light turns off at the agreed timeout.
- Occupancy reporting reflects sensor policy rather than merely copying the light state.
- Room transitions and any energy metric appear in the system snapshot/log.

## Video explanation

Demonstrate the two PIR/light channels, task timing, hold timeout, and the difference between occupancy and light state.

## Report section

Write a short section describing room sensing, lighting behavior, tests performed, and any occupancy/energy limitations. Give it to Member 5 for the final report.
