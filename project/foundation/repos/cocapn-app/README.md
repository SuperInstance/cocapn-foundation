# cocapn-app

Phone app, iOS + Android from one codebase (Flutter recommended: first-class
background audio, BLE, and platform STT bridging; single codebase discipline for
a small team).

## The two faces

1. **Log** (fishinglog.ai face) — default tab. Big mic state, live transcript
   segments with times, day timeline, review + export. Works with zero hardware,
   zero accounts. This is the free hook and it must feel finished on its own.
2. **Helm** (cocapn face) — renders the paired unit's profile as a control head
   that looks like the captain's actual autopilot (labels, layout, and words come
   from the profile's `gui` + `grammar` blocks). Until a Helm unit is paired, the
   tab is a quiet explainer: "plug a Helm unit into your autopilot's remote port."

Dictation can run split-screen beside Helm, or alone.

## Voice pipeline (the core asset)

```
                 ┌──────────────────────────────────────────────┐
                 │            ALWAYS-ON (low power)             │
 mic ───────────▶│  wake-word engine: "cocapn" (on-device)      │
                 └───────┬──────────────────────────┬───────────┘
                         │ wake in COMMAND ctx      │ wake in DICTATION toggle ctx
                         ▼                          ▼
              ┌─────────────────────┐    ┌───────────────────────────┐
              │ COMMAND MODE        │    │ DICTATION MODE            │
              │ platform STT, then  │    │ continuous STT + VAD      │
              │ match CLOSED grammar│    │ pause > threshold (2.5 s) │
              │ (profile-defined)   │    │ closes a segment          │
              ├─────────────────────┤    ├───────────────────────────┤
              │ conf ≥ floor:       │    │ each segment →            │
              │  C1: echo + execute │    │  speech.segment event     │
              │  C2: ask + confirm  │    │  (+ ts, mono, GPS fix)    │
              │ conf < floor: ask   │    │ assertion parser →        │
              │ "belay" cancels     │    │  catch.assertion events   │
              └─────────────────────┘    └───────────────────────────┘
                         │                          │
                         └────────► ActiveLog (append-only, local) ─► sync (opt)
```

Separation by construction: **dictation mode has no code path to actuation.**
The command pipeline requires wake word + helm token + profile grammar match.

## Key implementation notes

- **Timestamps first.** Capture `ts` and `mono` at audio-buffer time, not at
  STT-result time (STT latency would smear every downstream label by seconds).
- **Segments keep audio** (opt-in, rolling cap) so better models can re-transcribe
  history later — the log's value compounds retroactively.
- **Assertion parser** is a small deterministic layer (regex/keyword over species
  + container vocab) with `parse_conf`; ambiguity produces no assertion, just the
  raw segment. LLMs can re-parse later; the raw sentence is always preserved.
- **GPS**: subscribe to platform location; attach best-known fix to every event;
  emit `fix.track` breadcrumbs on movement.
- **BYOK chat hook (Phase 0 shape, Phase 2 polish):** a `ChatProvider` interface
  (DeepSeek/OpenAI/Anthropic/local endpoint), key stored in platform keychain,
  every exchange logged as `chat.exchange`. The chat gets *tools* to query the
  local log (`events_between(t0,t1)`, `assertions(species=…)`) — that's what makes
  "when did we hit that pinnacle?" answerable offline-adjacent.
- **Sea setup UX:** pair by pointing camera at the unit's QR. That's it. Every
  further step is dockside/provisioner territory.

## Snippet

`snippets/voice_state_machine.ts` — typed skeleton of the mode/segmentation logic
(portable pseudocode; Dart translation is mechanical).
