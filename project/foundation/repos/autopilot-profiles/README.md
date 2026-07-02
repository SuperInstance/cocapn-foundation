# autopilot-profiles

Device support as **data, not code**. Each profile is one JSON file that defines a
device's electrical interface, named actions, spoken command grammar, and the GUI
the app renders. The firmware interprets it; the app renders it; the provisioner
verifies it.

## Why this matters

- **New device = new file.** No firmware fork, no app release.
- **Auditable.** A profile can be reviewed line-by-line by a human before it ever
  touches steering — unlike generated code.
- **AI-authorable.** The conversational provisioner drafts profiles from a
  captain's description + photos of the port; a human with a meter verifies.
- **Community asset.** Verified profiles accumulate like Wikipedia articles. The
  `verified_by` chain records who confirmed what, on which unit, with what method.

## Verification states

| state | meaning | may arm? |
|---|---|---|
| `verified: false` | hypothesis — drafted from manuals/photos/AI | **no** |
| channel-level `verified_by` set | each electrical fact meter-confirmed at dockside | after sea trial |
| sea-trial record attached | SAFETY.md checklist passed on a real installation | yes |

## Planned profiles

- `comnav-p-series.json` — here, hypothesis state (Phase 1 target: my boat)
- `comnav-2001.json`, `comnav-1001.json`
- `simrad-ap.json` — NFU contact closures and/or NMEA path
- `furuno-navpilot.json`
- `wood-freeman.json` — classic troller pilot; likely the simplest electrically
- Non-autopilot roles later: `deck-lights.json`, `throttle.json` (C3 discipline),
  `engine-gauges.json` (input-only)

## Grammar notes

Command grammars stay closed and tiny — a few dozen phrases. That closedness is
the reliability strategy in wind and engine noise. Domain vocabulary for
*dictation* parsing (species names, gear terms) is a separate package
(`vocab-fishing`, Phase 3), not part of device profiles.
