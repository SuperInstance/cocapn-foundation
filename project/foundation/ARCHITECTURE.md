# ARCHITECTURE — the grand shape

Five layers. Each works without the layers above it.

```
┌─────────────────────────────────────────────────────────────────┐
│ 5 · INTELLIGENCE   BYOK LLM chat · RL/vision training · fleet   │
│                    analytics · conversational provisioner        │
├─────────────────────────────────────────────────────────────────┤
│ 4 · SYNC (optional) BYO Cloudflare Workers+R2+D1 · append-only  │
│                    log exchange · fleet sharing                  │
├─────────────────────────────────────────────────────────────────┤
│ 3 · ACTIVELOG      the substrate: append-only timestamped        │
│                    events · media anchors · GPS · merge rules    │
├─────────────────────────────────────────────────────────────────┤
│ 2 · BRAIN          captain's phone / laptop: wake word · STT ·   │
│                    command grammar · autopilot GUI · dictation   │
├─────────────────────────────────────────────────────────────────┤
│ 1 · EDGE           ESP32 Helm units · cameras · GPS · sounder    │
│                    dumb, safe, profile-driven                    │
└─────────────────────────────────────────────────────────────────┘
```

**Rule of the stack:** Layer 1–3 must be fully functional with no internet, ever.
Layer 4–5 are additive. A captain with only a phone gets layers 2–3 (fishinglog.ai
positioning). A captain with a Helm unit gets 1–3 (cocapn.com positioning).

---

## Layer 1 — Edge: the Helm unit

An ESP32 that plugs into the autopilot's **remote port** (SPU / distribution box),
exactly where a jog lever or remote would plug in. It *emulates a remote* — the
autopilot doesn't know it isn't one. This is the key trick: remote ports are the
manufacturer-sanctioned, electrically simple, fail-safe entry point on every brand.

- **Wired in parallel, never in series.** The physical helm controls and any
  existing remote keep working with the Helm unit unpowered, dead, or unplugged.
- **Profile-driven.** The firmware is a *profile interpreter* (see
  `repos/cocapn-helm-firmware/`). A profile is a JSON file describing the
  electrical interface: contact closures, resistor ladders, NMEA 0183/2000
  sentences, PWM. ComNav P-series, 2001, 1001, Simrad, Furuno, Wood Freeman are
  each a profile, not a fork.
- **Dual-link.** BLE for pairing + close-range; WiFi (unit as AP, or joining boat
  LAN) for range and throughput. Same authenticated message protocol on both.
- **Momentary-only actuation** with TTL, backed by a hardware watchdog
  (see `SAFETY.md`).
- Beyond autopilot: same board + different profile = throttle, lights, pumps,
  gauge monitoring, relay anything. The provisioner makes each new role a
  conversation, not a project.

## Layer 2 — Brain: phone and laptop

All compute local. Phone (iOS + Android, one cross-platform codebase) and desktop
(laptop at the helm) share the same core logic.

**Voice pipeline** (the heart of the app — full state machine in
`repos/cocapn-app/`):

```
mic ──▶ wake word ("cocapn") ──▶ COMMAND mode: small fixed grammar,
 │                               fuzzy match, confirm-by-class, execute
 └────▶ DICTATION mode: continuous STT, VAD pause segmentation,
        each segment = timestamped ActiveLog event (+ GPS fix)
```

- Wake word: on-device (openWakeOrd/Porcupine-class), custom-trained on "cocapn" —
  a word that exists nowhere else in speech, which is why the name works.
- Command STT: the platform's native STT (phone) or a small local model like
  whisper.cpp tiny/Vosk (laptop). The command set is small and closed; we match
  against a grammar, not open vocabulary — this is what makes it reliable in wind
  and engine noise.
- Dictation STT: native dictation (phone) or whisper.cpp base (laptop). Accuracy
  matters less than timestamps; the audio segment is kept (configurable) so future
  models can re-transcribe.
- **Autopilot GUI mirrors the physical unit.** A ComNav captain sees ComNav-shaped
  controls with ComNav words. The GUI is generated from the same profile the
  firmware runs — one file defines the buttons, the voice commands, and the wires.
- Dictation view: split screen or separate window; usable alone with no ESP32
  (the fishinglog.ai mode).

## Layer 3 — ActiveLog: the substrate

Append-only JSONL streams, one per device per day. Full spec in
`repos/activelog-spec/`. Key decisions:

- **Events, never state.** `helm.command`, `speech.segment`, `fix.gps`,
  `media.frame`, `catch.assertion`, `chat.exchange` — everything is an event with
  `ts` (UTC ms), `mono` (monotonic ms), `dev` (device id), `seq` (per-device
  counter).
- **Media anchoring by time, not by reference.** A camera frame and a spoken
  sentence are linked because their timestamps overlap — nothing has to know about
  anything else at capture time. "I'm putting lingcod in the port tote" + deck-cam
  footage at the same instant = a training label, resolved later, offline.
- **Merge = set union.** Append-only + unique (dev, seq) means fleet merge is
  trivial and conflict-free. No CRDT machinery needed.
- **Corrections are events.** `correction.retract` / `correction.amend` point at
  (dev, seq). History is never rewritten — vital for training data provenance and
  for trust.

## Layer 4 — Sync (optional)

Reference implementation on Cloudflare (Workers + R2 for media + D1 for indexes),
BYO account. Devices upload their own streams; peers pull streams they don't have.
Because merge is union, sync is resumable, orderless, and tolerant of
weeks-offline vessels. Fleet = a shared bucket + an access list. We host nothing
required; later we offer hosted convenience (banner, nominal fee to remove).

## Layer 5 — Intelligence

- **BYOK chat.** Captain's own API key (DeepSeek/OpenAI/Anthropic/local). Wake
  ("cocapn, ask…") or tap. Every exchange is an ActiveLog event, so the
  conversation itself is timestamped context for images, GPS, and catches around
  it. The chatbot can *query the log* ("when did we hit that pinnacle?").
- **Supervised-learning byproduct.** Narration + time-anchored media = labels.
  Species ID, size/weight estimation (ruler/scale in camera view), hazard
  teaching, sounder calibration against confirmed catches (sonarvision lineage).
  Training runs elsewhere; ActiveLog is the dataset format.
- **Conversational provisioner.** The endgame of profiles-as-data: captain
  describes a new role for a Helm unit; the system drafts the profile + wiring
  card; the installer verifies with a meter using the guided flow. The AI writes
  device support because device support is data.

---

## Connectivity tiers (fleet reality: "varies by vessel")

| Tier | Boat | What works |
|---|---|---|
| T0 | No connectivity, ever | Everything in layers 1–3. Export by USB/SD/AirDrop at the dock. |
| T1 | Dock WiFi only | + sync burps at the dock; fleet data arrives days late and that's fine. |
| T2 | Intermittent cell/Starlink | + opportunistic sync, BYOK chat at sea. |
| T3 | Always on | + live fleet overlay. |

Design target is T0; everything above is progressive enhancement.

## Security model (summary — details in SAFETY.md)

- Pairing: QR code on the Helm unit carries the unit's public key; phone and unit
  do an authenticated key exchange; all traffic (BLE and WiFi alike) is inside an
  encrypted, replay-protected session (nonce + counter). WiFi never depends on
  WPA alone.
- One *helm token* at a time: exactly one paired brain may hold actuation rights;
  transfer is explicit. Dictation/telemetry may be multi-listener.
- Logs are the captain's property. Sync is BYO storage. Sharing is opt-in per
  stream.
