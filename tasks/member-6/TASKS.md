# Member 6 — Node communication and Wokwi integration

## Ownership

- `src/network.c`
- `src/include/network.h`
- Wokwi integration tests and final project link

Coordinate changes to the protocol/interface with Member 3 before implementation. Do not edit coordinator state or the authoritative inside ledger.

## Work

- Implement the node-to-node message protocol for room terminals and the Entrance master.
- Include node identity, message type, sequence number, and event payload; reject malformed messages and deduplicate repeated/out-of-order messages.
- Send periodic heartbeats from Room 1 and Room 2 terminals to the Entrance node; make loss/timeout visible to the system.
- On the Entrance node, deliver each validated, deduplicated remote event to `coordinator_publish(event, timeout)`. Do not bypass the coordinator or modify shared state directly.
- Provide an outbound event API for remote room terminals and report transport errors to the caller.
- Integrate and exercise the final demo in Wokwi. Publish the final project link for Member 5 to include in the report.

## Interfaces

Provides `network.h` with:

- `network_start(node_id_t local_node)` to initialize the node communication service and its heartbeat/receive work.
- `network_send_event(const system_event_t *event, TickType_t timeout)` for sending a local event to the Entrance master.

On the Entrance node, this module calls `coordinator_publish()` from `coordinator.h` only after message validation and deduplication. The coordinator owns all authoritative state. The protocol and heartbeat must fit the shared types in `system_types.h`.

## Definition of done and Wokwi test

- Events identify their source node and sequence; duplicate/replayed messages are ignored and malformed frames are rejected.
- Both room terminals produce heartbeats; the Entrance node can detect and report a missing heartbeat.
- Valid remote room and access events reach the coordinator and update the snapshot once; network code never updates the ledger directly.
- Test message encode/decode, ordering, duplicates, and heartbeat timeout. Run the integrated Wokwi project and confirm serial output and room/access behavior.
- Confirm what multi-board networking Wokwi can demonstrate. If the simulator cannot run the intended multi-node topology, document that limitation, test the protocol with a reproducible single-node/test-harness path, and do not claim untested physical-network behavior.
- Provide Member 5 the working final Wokwi project link and test notes.

## Video explanation

Demonstrate the message fields, sequence-based deduplication, room heartbeat and timeout, and the path from a validated received event into the coordinator.

## Report section

Write a short section describing the node protocol, sequence/deduplication, heartbeat behavior, integration tests, and Wokwi limitations. Give it and the final Wokwi project link to Member 5.
