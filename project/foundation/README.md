# Cocapn / ActiveLog

**Voice is the interface. The log is the product. The boat becomes a robot one wire at a time.**

Cocapn is a hands-free helm assistant and always-on timestamped logger for working
vessels. A captain talks; the system steers the autopilot, transcribes the day, and —
as a byproduct of ordinary work — produces labeled training data that teaches cameras,
sounders, and future automation how *that* captain fishes.

ActiveLog is the substrate underneath: an open, append-only, timestamped event-log
format that every part of the system reads and writes. Cocapn is the first
application built on it.

---

## The repo constellation

Separate repos, one direction of dependency: everything depends on the spec,
nothing depends on an app.

| Repo | What it is | Depends on |
|---|---|---|
| **activelog-spec** | The event-log format: JSON schema, media anchoring, merge rules. The heart. | nothing |
| **cocapn-app** | Phone app (iOS + Android). Wake word, STT, command grammar, autopilot GUI, dictation mode, GPS stamping. | activelog-spec |
| **cocapn-desktop** | Laptop app. Same brain, small local STT model, bigger screen for review/annotation. | activelog-spec |
| **cocapn-helm-firmware** | ESP32 firmware: secure BLE+WiFi link, profile-driven autopilot remote emulation, hardware safety watchdog. | autopilot-profiles |
| **autopilot-profiles** | Data files (not code) describing each autopilot's remote interface: ComNav P-series / 2001 / 1001, Simrad, Furuno, Wood Freeman. Community-contributed. | activelog-spec (event names) |
| **cocapn-provisioner** | The dockside "post-coding-era" installer: picks/generates a profile, flashes firmware, walks the wiring with a meter, runs the sea-trial checklist. | firmware, profiles |
| **activelog-sync** | Optional offline-first sync: BYO Cloudflare (Workers + R2 + D1) reference implementation for fleet sharing. | activelog-spec |

## Principles (non-negotiable)

1. **Offline-first, always.** A boat with zero connectivity gets the full core
   experience. Cloud is additive, never required.
2. **The log is append-only truth.** Every subsystem emits timestamped events;
   nothing edits history. Corrections are new events.
3. **Profiles, not forks.** New hardware support = a data file, not a firmware fork.
   This is what lets an AI write support for a new device from a conversation.
4. **Safety by command class.** Voice can trim a course; voice cannot latch a rudder
   or touch propulsion without escalating confirmation. See `SAFETY.md`.
5. **Two setup modes.** *Dockside setup* assumes a medium-skilled technician with a
   multimeter. *Sea setup* assumes a captain who wants it to just work.
6. **Open source, funded by hardware + hosting.** Code free forever; money from
   ESP32 Helm units (preloaded, affiliate/maker channel) and later a nominal
   hosted-service tier.

## Start here

- `VISION.md` — the 3–5 year picture, ecosystem, and moat
- `ARCHITECTURE.md` — the grand architecture, layer by layer
- `SAFETY.md` — watchdogs, overrides, dead-man logic
- `ROADMAP.md` — phased plan from MVP to Vessel-as-a-Robot
- `repos/` — scaffold + key code for each repo

## Prior art

`github.com/superinstance/sonarvision` — early exploration of camera-calibrated
depthsounder analysis; its ideas fold into the Phase 3 perception work.

## Domains held

cocapn.com · activelog.ai · activeledger.ai · fishinglog.ai · makerlog.ai ·
personallog.ai · businesslog.ai · studylog.ai · reallog.ai · playerlog.ai
