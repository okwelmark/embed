# Member 3 — Coordinator, authoritative occupancy, and integration

## Ownership

- `src/coordinator.c`
- `src/include/system_types.h`
- `src/include/event_bus.h`
- `src/include/coordinator.h`
- `src/main.c`
- `src/CMakeLists.txt`

The shared headers are the first integration deliverable. Get agreement from all members before freezing them; subsequent changes require review from affected owners.

## Work

- Implement the event queue and single coordinator consumer.
- Keep the authoritative inside-UID ledger and room snapshot in one owner; protect it internally and expose only coordinator APIs.
- Enforce anti-passback: an already-inside UID cannot enter again, and an exit is rejected unless that UID is inside.
- Provide the remote-event ingress used by Member 6: after network validation and deduplication, Member 6 calls `coordinator_publish(event, timeout)` to enqueue the event for coordinator processing.
- Integrate module startup and source registration in `main.c` and `src/CMakeLists.txt`; check every task/queue initialization result.

## Interfaces

Provides `system_types.h`, `event_bus.h`, and `coordinator.h`.

Consumes local events and validated remote events from Member 6. `coordinator_publish(const system_event_t *event, TickType_t timeout)` is the remote-event ingress; Member 6 must call it only after validating the message and applying network sequence-number deduplication. The coordinator is the only event-bus consumer and owns the only mutex for the system snapshot/UID ledger. Never expose the queue handle or mutable state.

## Definition of done and test

- Only the Entrance master changes the authoritative inside ledger.
- Duplicate/replayed access events do not double-count; an exit for a UID not inside is denied.
- Concurrent room and access events leave snapshots internally consistent.
- Queue and task creation failures are surfaced; no door opens without a positive decision.
- Verify remote-event ingress using events supplied by Member 6's integration test; coordinator remains usable through the local single-board Wokwi path.

## Video explanation

Explain the event shape, queue ownership, state mutex/ledger, anti-passback, and remote-event ingress; demonstrate a consistent snapshot.

## Report section

Write a short section describing coordinator/event ownership, the authoritative inside ledger, anti-passback, tests performed, and concurrency limitations. Give it to Member 5 for the final report.
