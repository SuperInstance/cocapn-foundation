# ROADMAP

Phases, not dates. Each phase ends with something a real captain uses daily.

## Phase 0 — The spec and the free app (the hook)
- `activelog-spec` v1 frozen: event envelope, core event types, merge rule, media
  anchoring. **This is the one thing we don't get to churn later.**
- `cocapn-app` MVP: wake word, dictation mode with pause segmentation + GPS
  stamping, day-file review, export (JSONL + plain text). Ships as the free
  fishinglog.ai experience. Helm tab present but empty ("connect a Helm unit").
- Desktop app skeleton sharing core logic.
- **Exit test:** a troll captain uses dictation for a full day, and reviewing
  the day that evening is genuinely useful.

## Phase 1 — The Helm unit on my boat (ComNav P-series)
- `cocapn-helm-firmware`: profile interpreter, BLE+WiFi secure link, TTL +
  watchdog + override, momentary relay board.
- `autopilot-profiles`: `comnav-p-series.json` verified on the real SPU
  (pinout confirmed with a meter at the dock — the profile ships with the
  discovery notes that found it).
- App Helm tab: profile-generated GUI matching the physical autopilot; C0/C1/C2
  voice commands; echo-back; belay.
- Full SAFETY.md sea-trial passes on my boat.
- **Exit test:** hands-free trolling through a full set of gear, all season tacks
  by voice, zero unsafe events in the log.

## Phase 2 — Anyone's boat, anyone's autopilot
- Profiles: ComNav 2001, 1001; Simrad (NFU/NMEA); Furuno; Wood Freeman.
- `cocapn-provisioner` v1: pick profile → flash over USB → guided meter checks →
  guided sea trial → install report. Medium-skilled-technician grade.
- BYOK chat polished (interface designed in Phase 0; events already in the spec):
  "cocapn, ask…", chat can query the day's log.
- Maker/vendor pilot: 10 preloaded units through one vendor; wiring cards;
  makerlog.ai affiliate skeleton.
- **Exit test:** an installer we've never met completes a Simrad install from the
  provisioner alone.

## Phase 3 — Perception and fleet
- Deck camera ingestion: `media.frame` events time-anchored to narration; first
  supervised dataset exports (species/tote assertions vs footage).
- Sounder + chartplotter integration: catches and speech events as chart overlays;
  season replay. Sonarvision ideas return here.
- `activelog-sync` v1 on BYO Cloudflare; first two-boat fleet.
- **Exit test:** "where did we catch lingcod in May?" answered from the log, on a
  chart, offline.

## Phase 4 — Vessel-as-a-Robot, and beyond fishing
- Helm units in non-autopilot roles via conversational provisioning: lights,
  pumps, gauges, throttle (C3 discipline).
- Thermal engine-room and underwater cameras; sounder calibration from confirmed
  catches.
- Domain expansion begins only now: studylog / reallog / playerlog as grammars
  over the same substrate; hosted tier with banner-removal fee.

## Standing rules
- Nothing ships without its two-mode setup story (dockside / sea).
- Nothing enters the spec without a real consumer *and* a real producer.
- Safety regressions block release, full stop.
