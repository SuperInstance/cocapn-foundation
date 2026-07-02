# cocapn-provisioner

The "post-coding-era" installer. The system is the engineer; the human is the
wiring assistant. This repo is what makes the whole ecosystem installable by a
medium-skilled technician and expandable by conversation.

## Two setup modes (project-wide discipline)

### Dockside setup — technician + multimeter + this tool
1. **Identify.** Pick the device (or photograph the unit + remote port; the
   conversational flow drafts/selects a profile).
2. **Flash.** USB-C to the Helm unit; provisioner flashes firmware + writes the
   profile + generates the unit's keypair; prints/screens the QR pairing sticker.
3. **Verify wiring, guided, one fact at a time.**
   > "Set your meter to continuity. Probe pins 3 and 5 on the remote plug.
   > Now press PORT DODGE on the helm. Beep? → channel 0 confirmed."
   Each confirmed fact is written into the profile (`verified_by`, date, method).
   The unit **will not arm** with unverified channels.
4. **Bench test.** Unit pulses each channel; technician confirms the autopilot
   responds as expected with the boat safely tied up.
5. **Sea trial.** Guided SAFETY.md checklist: override test, link-kill test, TTL
   test, wrong-word test. Pass = install report + arm.

### Sea setup — captain, no skills assumed
1. Open app → point camera at QR on the Helm unit.
2. Say "cocapn." It answers.
3. There is no step 3. Everything needing knowledge happened dockside.

## The conversational provisioner (Phase 2+, the endgame)

Captain: *"I want the deck lights on voice."*
System: drafts `deck-lights.json` (relay channel, "lights on"/"lights off"
grammar, C1 class, GUI card), generates the wiring card (supply, fusing, relay
rating), and queues the dockside verification flow. Because device support is
**data**, the AI's output is reviewable line-by-line and gated by the same
verification discipline as a human-authored profile. No generated C++ ever
ships to a unit.

## Form

Desktop app (shares cocapn-desktop shell) + CLI. Talks to units over USB for
flashing, BLE/WiFi for tests. Works fully offline from a bundled profile library;
the conversational drafting uses BYOK when available.
