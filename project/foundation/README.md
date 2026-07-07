# Cocapn / ActiveLog

> **Status: design brief, not a product.** The documents in this folder describe
> a system that is being designed and reviewed, not one that is shipping. Phase 0
> (the free timestamped-logging app) is the first thing to be built; the first
> voice-steered Helm unit is Phase 1 (see `ROADMAP.md`). The safety architecture
> in `SAFETY.md` is specified in enough detail to be argued with and tested
> against *because it has to be* — not because it has been validated in the
> field.

## The problem, in one paragraph

On a working fishing vessel the captain's hands are almost never free. One hand
holds a gaff or a throttle; the other is on the wheel or the radio; the crew is
shouting about gear; the autopilot is doing its best to hold a course through a
tide rip. The captain wants to say "port ten" and have the boat shadow the
trolling wire by ten degrees, or "mark, lingcod, port tote, twelve pounds" and
have it remembered *with the time and the GPS fix* — without ever putting
anything down. Two things have to be true for that to be worth doing at all: the
voice link has to be unable to hurt the boat even when it fails, and the log it
produces has to be honest enough to trust later. Cocapn is the voice interface;
ActiveLog is the log underneath it.

## What each one is

**Cocapn** is a hands-free helm assistant and always-on timestamped logger for
working vessels. A captain talks; the system steers the autopilot within strict
safety limits, transcribes the day, and — as a byproduct of ordinary work —
produces labeled training data that teaches cameras, sounders, and future
automation how *that* captain fishes.

**ActiveLog** is the substrate underneath: an open, **append-only** (nothing
ever edits or deletes a past record; corrections are *new* records that point at
the old one), **timestamped event-log** format that every part of the system
reads and writes. Cocapn is the first application built on it.

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

Two pieces of jargon in that table earn a definition up front, because every
other repo depends on understanding them:

- A **profile** is a data file (JSON), not code. It describes one piece of
  marine hardware's remote-control interface: which wires do what, which spoken
  phrases map to which actions, and how dangerous each action is. The firmware
  is a *profile interpreter* — one program that reads any profile and drives the
  hardware accordingly, instead of separate code for ComNav, Simrad, Furuno, and
  so on. This is what makes "support a new autopilot" a matter of writing a data
  file, not forking the firmware.
- An **append-only log** is a record you can only ever add to, never edit or
  delete. ActiveLog is built this way so that the history of what happened on
  the boat is tamper-evident and conflict-free to merge across devices. (Full
  rationale in `ARCHITECTURE.md`, Layer 3.)

## Principles (non-negotiable)

1. **Offline-first, always.** A boat with zero connectivity gets the full core
   experience. Cloud is additive, never required.
2. **The log is append-only truth.** Every subsystem emits timestamped events;
   nothing edits history. Corrections are new events.
3. **Profiles, not forks.** New hardware support = a data file, not a firmware
   fork. This is what lets an AI write support for a new device from a
   conversation — and what makes that support safe to review, because a profile
   is auditable data, not code.
4. **Safety by command class.** Voice can trim a course; voice cannot latch a
   rudder or touch propulsion without escalating confirmation. See `SAFETY.md`.
5. **Two setup modes.** *Dockside setup* assumes a medium-skilled technician with
   a multimeter. *Sea setup* assumes a captain who wants it to just work.
6. **Open source, funded by hardware + hosting.** Code free forever; money from
   ESP32 Helm units (preloaded, affiliate/maker channel) and later a nominal
   hosted-service tier.

## Start here

- `VISION.md` — the 3–5 year picture, ecosystem, and moat
- `ARCHITECTURE.md` — the grand architecture, layer by layer
- `SAFETY.md` — the five-layer safety architecture: command classes (C0–C3),
  the de-energized-safe / momentary-only / TTL principle (the "dead-man" idea),
  the override lockout, and the sea-trial checklist
- `ROADMAP.md` — phased plan from MVP to Vessel-as-a-Robot
- `repos/` — scaffold + key code for each repo

## Prior art

`github.com/superinstance/sonarvision` — early exploration of camera-calibrated
depthsounder analysis; its ideas fold into the Phase 3 perception work.

## Domains held

cocapn.com · activelog.ai · activeledger.ai · fishinglog.ai · makerlog.ai ·
personallog.ai · businesslog.ai · studylog.ai · reallog.ai · playerlog.ai
