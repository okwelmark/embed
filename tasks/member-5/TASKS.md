# Member 5 — Serial CLI, staff authorization, and deliverables

## Ownership

- `src/cli.c`
- `src/auth.c`
- `src/include/cli.h`
- `src/include/auth.h`
- `README.md`
- `docs/system-operation-report.md` (create after implementation evidence is available)

Do not edit other members' implementation modules. Gather each person's contribution and test evidence for the report rather than claiming their code.

## Work

- Implement a clear UART command menu/parser and reject malformed or unknown commands.
- Provide queries for recent access records, building/room occupancy, lighting, and the current schedule.
- Require staff authentication before schedule changes; audit both successful changes and failed attempts without logging credentials.
- Update README run/test instructions and compile the final operation report from each member's own report section, verified behavior, known limitations, the final Wokwi project link from Member 6, and individual contribution/video details.

## Interfaces

Provides `cli.h` and `auth.h`.

Depends on `coordinator.h`, `storage.h`, and `schedule.h`. Call `auth_verify_staff()` before `schedule_set()`; use coordinator snapshots rather than reading shared globals.

## Definition of done and test

- Documented commands work over the serial monitor; malformed arguments and invalid directions are rejected.
- Unauthenticated schedule changes fail; authenticated changes persist and are auditable.
- Access history, occupancy, room state, lights, and schedule can be queried.
- README/report distinguish implemented features from simulator or hardware limitations; contribution claims match the code each student can explain.
- The final report includes a clearly attributed section from each of the six members, plus verified integration results and the final Wokwi link.

## Video explanation

Demonstrate the menu/parser, a status/history query, rejected unauthenticated update, authorized schedule update, and the audit result.

## Report section

Write the CLI/authentication/audit section and compile the final report using each member's submitted section. Confirm that contribution statements and test claims match evidence and that the Wokwi link supplied by Member 6 opens the final integrated project.
