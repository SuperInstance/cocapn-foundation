# cocapn-helm-firmware

ESP32 firmware for the **Helm unit** — a profile-driven remote-control emulator
that plugs into an autopilot's remote port (or later: throttle, lights, pumps,
gauges).

## The one big idea: the firmware is a profile *interpreter*

There is no ComNav code, Simrad code, or Furuno code in this repo. There is one
engine that executes **profiles** — JSON files (see `autopilot-profiles/`)
describing:

1. **Electrical interface** — what the device's remote port electrically is:
   contact closures, a resistor ladder, NMEA 0183 sentences, NMEA 2000 PGNs, PWM.
2. **Actions** — named operations ("port_10", "mode_auto") mapped to output
   recipes (pulse pin 2 for 300 ms; present 2.2 kΩ between pins A and B; send
   `$xxAPB…`).
3. **Command grammar** — the spoken phrases, their action mapping, and their
   safety class (C0–C3).
4. **GUI layout** — the buttons the app renders, so the phone shows a control head
   shaped like the captain's real autopilot.

One file defines the wires, the words, and the screen. This is what makes
"an AI writes support for a new device from a conversation with the captain"
possible and *safe to review* — a profile is auditable data, not code.

## Hardware reference design

- ESP32-S3 module, 12 V marine supply (buck + protection: reverse polarity, load
  dump, ESD on all I/O)
- 4–8 output channels: optocoupled contact closures rated for autopilot remote
  inputs; digitally-switched resistor ladder option for analog remotes
- 2 opto-isolated inputs: override detect, physical C3-enable switch
- RS-422/NMEA 0183 transceiver + CAN (NMEA 2000) footprint, populated per role
- Status LED + QR pairing sticker (device pubkey) on the enclosure

Fail-safe posture: all outputs normally-open, pull-downs on every driver, outputs
open during boot/brownout/crash. See `foundation/SAFETY.md`.

## Link layer

BLE (pairing + close range) and WiFi (unit as AP or joins boat LAN) carry the same
protocol: encrypted session established from QR-pairing keys, per-message
monotonic counter (replay protection), single helm-token master, 500 ms command
TTL refreshed by heartbeat.

## Key sources here

- `snippets/autopilot_hal.h` — the interpreter's core interfaces
- `snippets/watchdog.cpp` — TTL + override + hardware watchdog logic
- Profile example: `../autopilot-profiles/comnav-p-series.json`

## Status of electrical details

Pin functions for the ComNav P-series remote port are **written as a hypothesis**
and marked `"verified": false` until confirmed with a meter on the real SPU
(Phase 1). The provisioner's discovery flow exists precisely because remote-port
details vary by unit and installation — the profile format records *how* each
fact was verified.
