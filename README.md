# cocapn-foundation

> **This is a design brief, not a product.** Nothing in this repository is
> shipping hardware or a runnable application. It is the foundational design
> document set for the **Cocapn / ActiveLog** ecosystem: a hands-free helm voice
> assistant and always-on timestamped logger for working fishing vessels. Read it
> to understand the *plan* and the *safety argument* — not to install anything.

## What this repository is

`cocapn-foundation` is the single source of truth for the design of two things,
and the relationship between them:

- **Cocapn** — a hands-free helm assistant. A captain talks ("cocapn, port ten")
  and the system nudges the autopilot within strict safety limits, transcribes
  the working day, and stamps everything with time and GPS.
- **ActiveLog** — the substrate underneath Cocapn: an open, append-only,
  timestamped event-log format. Every part of the system reads and writes it.

The repository holds **documents and illustrative code snippets**, not a buildable
system. There is no `package.json`, no firmware build, no app binary, and no test
suite. The value here is the *architecture*, the *safety case*, and the *spec* —
written first and published loudest precisely because they are the things that
must be airtight before any of it gets built.

## Status & capabilities (honest markers)

This whole repository sits at the 🔮 / design-spec stage. Concretely:

- ✅ **Design documents that are internally consistent and verified against the
  snippets they reference.** Every number in `SAFETY.md` (500 ms command TTL,
  2000 ms hardware watchdog, 750 ms link-loss grace, 10 000 ms override lockout,
  0.85 confidence floor, 6 000 ms C2 confirm window) was checked against the code
  snippets in `repos/cocapn-helm-firmware/snippets/` and
  `repos/cocapn-app/snippets/` and matches.
- ✅ **A valid JSON Schema for the ActiveLog v1 event envelope**
  (`repos/activelog-spec/schema/event.schema.json`, JSON Schema draft 2020-12),
  and a worked example day of events
  (`repos/activelog-spec/examples/troll-day.jsonl`) that conforms to it.
- ✅ **Real, readable design-level code snippets** — `watchdog.cpp` (the safety
  supervisor), `autopilot_hal.h` (the profile-interpreter interfaces),
  `voice_state_machine.ts` (the wake/command/dictation core). These are
  illustrative fragments (the firmware snippet even calls itself out as relying
  on declarations defined elsewhere), not a complete compilable firmware or app.
- ⚠️ **One device profile (`comnav-p-series.json`) that is explicitly a
  hypothesis.** It carries `"verified": false` and states its electrical facts
  must be confirmed with a multimeter on the real unit during dockside discovery.
- 🔮 **No shipping software.** The phone app, desktop app, ESP32 firmware,
  provisioner, and sync layer are all described as phased future work
  (`ROADMAP.md`). Phase 0 — the free timestamped-logging app — is "the first
  thing to be built." No phase is marked complete.
- 🔮 **No tests.** Verification of this repository is by reading and argument,
  not by running a suite.

## The problem, in one paragraph

On a working fishing vessel the captain's hands are almost never free — one holds
a gaff or throttle, the other the wheel or radio, the crew is shouting about
gear. The captain wants to say "port ten" and have the boat shadow the trolling
wire, or "mark, lingcod, port tote, twelve pounds" and have it remembered *with
the time and GPS* — without putting anything down. Two things must be true for
that to be worth doing at all: the voice link must be **unable to hurt the boat
even when it fails**, and the log it produces must be **honest enough to trust
later**. This repository is where both of those claims are argued in detail
before any hardware is built.

## How to read this repository

The substantive content lives under `project/foundation/`. Start there, in this
order:

| Read this | For |
|---|---|
| [`project/foundation/README.md`](project/foundation/README.md) | The real entry point: what Cocapn and ActiveLog are, the repo constellation, the principles. |
| [`project/foundation/SAFETY.md`](project/foundation/SAFETY.md) | The five-layer safety architecture: command classes (C0–C3), the de-energized-safe / momentary-only / TTL principle, the override lockout, the sea-trial checklist. **The most important document.** |
| [`project/foundation/ARCHITECTURE.md`](project/foundation/ARCHITECTURE.md) | The five-layer stack (Edge → ActiveLog → Intelligence), offline-first rule, connectivity tiers. |
| [`project/foundation/ROADMAP.md`](project/foundation/ROADMAP.md) | Phased plan: Phase 0 (spec + free app) → Phase 4 (Vessel-as-a-Robot). |
| [`project/foundation/VISION.md`](project/foundation/VISION.md) | The 3–5 year picture, ecosystem, and moat. A thinking document. |
| [`project/foundation/repos/`](project/foundation/repos/) | A scaffold README + key snippet for each of the seven planned repos. |

### Repository layout

```
cocapn-foundation/
├── README.md                         this file
├── EDUCATIONAL_NOTES.md              notes from a prior documentation pass
└── project/
    ├── Cocapn Foundation.dc.html     Claude Design visual mockup of the brief (see below)
    ├── support.js                    Claude Design runtime for the mockup
    ├── .thumbnail                    preview image for the mockup
    └── foundation/                   ← the real design content
        ├── README.md · VISION.md · ARCHITECTURE.md · SAFETY.md · ROADMAP.md
        └── repos/
            ├── activelog-spec/         schema/ + example day  (the substrate)
            ├── cocapn-app/             voice_state_machine.ts snippet
            ├── cocapn-helm-firmware/   autopilot_hal.h + watchdog.cpp snippets
            ├── autopilot-profiles/     comnav-p-series.json (hypothesis profile)
            ├── cocapn-desktop/         README only
            ├── cocapn-provisioner/     README only
            └── activelog-sync/         README only
```

## The ecosystem at a glance

Seven planned repos, one direction of dependency — everything depends on the
spec, nothing depends on an app:

| Repo | What it is (planned) |
|---|---|
| **activelog-spec** | The event-log format: JSON schema, media anchoring, merge rules. The heart. |
| **cocapn-app** | Phone app (iOS + Android). Wake word, STT, command grammar, dictation, GPS stamping. |
| **cocapn-desktop** | Laptop app. Same brain, local STT model, review/annotation surface. |
| **cocapn-helm-firmware** | ESP32 firmware: profile interpreter, secure link, hardware safety watchdog. |
| **autopilot-profiles** | Data files (not code) describing each autopilot's remote interface. |
| **cocapn-provisioner** | Dockside installer: flash, guided meter checks, sea-trial checklist. |
| **activelog-sync** | Optional offline-first sync (BYO Cloudflare Workers + R2 + D1 reference). |

> ⚠️ These seven repos are **planned**, described here as design targets. Within
> this foundation repository each is represented only by a README and (for three
> of them) a single illustrative code or data snippet. They are not separate
> built-out codebases in this repo.

The architecture is a five-layer stack with one non-negotiable rule: **layers 1–3
(Edge, Brain, ActiveLog) must work fully with no internet, ever.** Layers 4–5
(Sync, Intelligence) are additive. Full detail in `ARCHITECTURE.md`.

## Where this design has gone since it was written

This brief predates the code that now realizes parts of it. The honest map, as
of 2026-07-09:

- ✅ **[purplepincher/deckboss](https://github.com/purplepincher/deckboss)** —
  a shipped, field-ready voice-first fishing logbook (PWA) that knowingly
  implements a **scoped-down version of this design**. It is an independent
  codebase (no shared git history with this repo), but the lineage is direct
  and on the record: its `log-entry.ts` quotes `activelog-spec`'s "No editing.
  Ever." principle verbatim, and it mirrors this repo's `SAFETY.md` and
  `activelog-spec/` into its own
  `docs/cocapn-foundation-mirror/` so the citation can't go stale. What it
  ships is the ActiveLog *logical model* (append-only, additive corrections,
  read-time fold) in its own wire shape — not the byte-for-byte v1 envelope.
- ✅ **[cocapn-marine](https://github.com/SuperInstance/cocapn-marine)** — a
  real, tested Rust library (NMEA 0183 parsing, PID autopilot, bathymetric
  recording, deadband monitors) built for the sensor-and-control tier this
  architecture describes. It stands alone today; nothing here consumes it yet.
- ⚠️ **The ActiveLog envelope defined here is now the ancestor schema for a
  wider effort.** The schema-reconciliation record lives in
  [`purplepincher/purplepincher` `docs/ACTIVELOG_SCHEMA_FOUNDATION.md`](https://github.com/purplepincher/purplepincher/blob/main/docs/ACTIVELOG_SCHEMA_FOUNDATION.md),
  whose standing rule is taken from this spec: one envelope, many namespaced
  event types — never a second envelope.
- 🔮 Everything else in the seven-repo constellation (helm firmware,
  provisioner, sync, desktop) remains unbuilt, exactly as the sections below
  state.

**Naming note:** the PyPI package
[`activelog-agent`](https://github.com/SuperInstance/activelog-agent) is an
unrelated log-file *monitoring* toolkit — it shares the ActiveLog name but does
not implement or depend on the event-log format specified here.

## What this repository does NOT contain

- 🔮 **No runnable app, firmware, or sync server.** The snippets are design
  illustrations, not a buildable project.
- 🔮 **No validated hardware profile.** The one shipped profile is a hypothesis
  marked `verified: false`.
- 🔮 **No installed Helm unit on any vessel.** The safety architecture is
  specified so it *can* be argued with and tested against on paper — not because
  it has been validated in the field.
- 🔮 **No tests, CI, or conformance vectors** for the spec yet (the
  `activelog-spec` README lists a `conformance/` directory as a target, not a
  present artifact).

## About the design mockup (`project/Cocapn Foundation.dc.html`)

This repository originated as a **handoff bundle from Claude Design**
(claude.ai/design). `project/Cocapn Foundation.dc.html` is an HTML/CSS/JS
prototype — a visual one-page rendering of the foundation brief — that depends on
`project/support.js` (the Claude Design runtime). It is a *design prototype*, not
production code: it exists to communicate look-and-feel and layout, and its links
point at the `foundation/*.md` documents.

> Note for anyone implementing the mockup: the prototype's links and the source
> of truth are the `project/foundation/` Markdown files. Read those for the actual
> architecture, safety, and data-model decisions; treat the HTML as a visual
> reference only.

---

### Appendix: original Claude Design handoff note

This repository was exported as a design handoff bundle. The original wrapper
instructions (preserved for context) were:

> This is a **handoff bundle** from Claude Design (claude.ai/design). A user
> mocked up designs in HTML/CSS/JS using an AI design tool, then exported this
> bundle so a coding agent can implement the designs for real.
>
> The design medium is HTML/CSS/JS — these are prototypes, not production code.
> Match the visual output; don't copy the prototype's internal structure unless it
> happens to fit.
