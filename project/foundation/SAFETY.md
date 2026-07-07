# SAFETY — voice-commanded steering on a working vessel

> **Status: this is a design brief, not a product.** Nothing described here is
> shipping hardware. Cocapn is at the stage where the safety *architecture* is
> specified in enough detail to be reviewed, argued with, and tested against on
> paper — and eventually on one boat — but no Helm unit has been installed on a
> real vessel. Phase 0 (the free timestamped-logging app) comes first; the first
> voice-steered Helm unit is Phase 1 (see `ROADMAP.md`). This document is
> written first and published loudest precisely *because* it is the thing that
> has to be airtight before any of it gets built. If the safety story isn't
> airtight, nothing else matters: nobody sane wires an internet-adjacent hobby
> board into their steering, and they're right not to.

## Start with the failure, not the fix

Picture a salmon troller working a set of gear in a choppy tide rip. The captain
has a hand on the throttle and an eye on two other boats; the crew is shouting
about a tangle. The captain says "cocapn, port ten" — nudge the autopilot ten
degrees to port so the boat shadows the trolling wire. Now imagine the voice
link simply obeyed every word it heard, the way a phone assistant might open a
calendar entry from background chatter. Here is what goes wrong, and every one
of these is a real failure mode that has happened on real boats with real
remote controls:

- **The word is wrong.** Wind, engine noise, and VHF chatter are exactly the
  conditions that break speech recognition. "Hold" becomes "helm." "Port ten"
  becomes "port twenty." A crew joke gets picked up as a command. A system that
  *guesses* in this environment is a system that eventually turns the boat the
  wrong way at the wrong moment.
- **The word was right, but the situation changed.** The captain says "port
  ten," then sees a log in the water and grabs the wheel. If the voice command
  is now *latched* — held in hardware until something explicitly cancels it — the
  boat fights the captain's hands. The human has to win an argument with
  software while also avoiding the log.
- **The brain disappears.** The phone app crashes, the battery dies, the BLE
  link drops behind a steel wheelhouse — all mid-command. If the last command
  was "hold this heading," a system that *holds state* will keep steering long
  after the thing that issued the command is gone.
- **The wrong brain is talking.** A second paired phone, a replayed radio
  packet, a malicious bystander. A system that trusts any correctly-shaped
  command steers for whoever can forge one.
- **The hardware itself faults.** A relay welds shut; a driver transistor fails
  shorted. A system whose "off" depends on software running correctly will hold
  a rudder command through exactly the crash where it most needs to stop.
- **The captain needs the boat *right now*.** Man overboard, a pot line in the
  wheel, a ferry appearing out of the fog. The one thing the captain cannot
  afford in that instant is a voice interface that has to be politely dismissed
  before the helm responds.

That list is the project's threat model, told as a scene rather than a numbered
register: software hang or crash while a command is active; link loss
mid-command; misheard command in noise; unauthorized or replayed command;
electrical fault in the Helm unit; and the captain needing immediate manual
control. Everything in the rest of this document is the answer to one specific
line on that list. The architecture is five layers because there are roughly
five distinct ways this goes wrong, and no single layer covers all of them.

## The idea that makes the rest make sense: "safe" is the resting state

Before the layers, one principle, because every layer is a variation of it:

**The Helm unit's resting state — unpowered, idle, crashed, unplugged, ignored —
is "do nothing." Any *action* is a temporary departure from that resting state,
and the action must be continuously re-authorized or it stops on its own.**

Control engineers have names for this pattern. A *deadband* is the range of
input where a system intentionally does nothing, because doing nothing is
already the safe answer. A *dead-man's switch* is the same idea over time: the
action keeps going only while a human is actively holding it open, and the
instant that pressure releases, it snaps back to safe. (This project's
`README.md` pointer calls the whole thing "dead-man logic"; the source documents
below use three more specific names — *de-energized-safe*, *momentary-only*, and
*command TTL* — one per layer, for the same underlying property. You will meet
all three where they belong.)

A *lockout*, which comes up in Layer 2, is different and worth distinguishing
now: a lockout is a deliberate, time-boxed refusal to accept a certain class of
command *after* a triggering event — not "do nothing because nothing is asked,"
but "refuse even a correct command, for this many seconds, because something
just happened." The trigger and duration in this design are stated exactly
where the mechanism lives.

Lock those three terms in — deadband, dead-man's switch, lockout — and the five
layers stop being a checklist and become a single idea, enforced five times in
five independent ways.

## The five-layer defense

### 1 · Electrical: parallel, momentary, de-energized-safe

The Helm unit is a small ESP32 board that wires into the autopilot's **remote
port** — the same jack where a manufacturer's jog lever or handheld remote would
plug. It does not sit between the captain and the steering; it sits *next to*
the existing controls.

- **Wired in parallel, never in series.** "In parallel" means the Helm unit's
  outputs connect across the same two points as the existing helm controls, like
  a second light switch added beside the first. Either one can actuate the
  circuit; the Helm unit being dead or unplugged does not break the captain's
  own controls. (Wired "in series" — spliced *into* the captain's control line —
  would mean our failure breaks their steering. That is the one topology this
  design forbids.)
- **Normally-open outputs.** Every actuation output is a *momentary contact
  closure* (a switch held closed only while actively driven) through a
  **normally-open relay or optocoupler** (a relay/optocoupler being an
  electrically-isolated switch; "normally-open" means its resting state is
  *broken*, no connection, unless something drives it closed). Unpowered,
  crashed, unplugged, or browned-out, the autopilot's remote port simply sees
  "nobody is pressing the remote." There is no hardware state in which our
  failure holds a rudder command. This is what answers the "relay welds shut /
  driver fails shorted" line on the failure list: the default electrical
  posture is *open*.
- **Nothing is ever latched in hardware.** When the captain says "hold this
  heading," there is no physical switch that flips and stays flipped. "Hold" is
  implemented as *repeated short pulses*, issued over and over by live software.
  The moment the software stops issuing them — crash, link loss, belay — the
  behavior stops, because nothing hardware-side is remembering it. This is the
  electrical layer's version of the dead-man idea.

### 2 · Firmware: hardware watchdog + command TTL

If Layer 1 makes "safe" the resting state of the *wires*, Layer 2 makes "safe"
the resting state of the *chip*.

- **Every command carries a TTL (time-to-live), default 500 ms.** TTL is
  borrowed from networking, where it means "drop this packet after this many
  hops." Here it means: the firmware will hold an actuation output closed for
  only 500 ms unless a fresh, authenticated **heartbeat** message extends it.
  (A heartbeat is a tiny "still here, still authorized" message the app sends
  repeatedly while a command is active.) No heartbeat in time = the output
  releases. This is the firmware layer's dead-man switch: the action keeps going
  only while the app is actively saying "still going," tick by tick.
- **A hardware watchdog resets the chip if the main loop stalls.** The ESP32's
  *hardware watchdog timer* (WDT) is a separate counter on the silicon: the main
  program must periodically "kick" (reset) it, and if it doesn't — because it
  hung, deadlocked, or got stuck in a bad loop — the watchdog hard-resets the
  chip after **2000 ms**. Outputs are *open* during boot by design (pull-down
  resistors hold every driver off until the firmware explicitly brings it up),
  so a software freeze self-clears in two seconds and lands in the safe posture.
- **The override input: human always outranks radio.** A physical wire (the
  *override detect* input, connected to the helm's own controls where the
  installed profile allows it) is read on every loop tick. The instant the human
  touches the real controls, the firmware drops every output immediately and
  enters a **10-second lockout**. This is the direct answer to "the captain sees
  a log and grabs the wheel": grabbing the wheel should never have to negotiate
  with a voice command.

#### Lockout, defined precisely

In this design the lockout's trigger is **the human touching the real helm
controls** (read every loop tick via the override-detect input), and its
duration is **10 seconds** (`OVERRIDE_LOCKOUT_MS = 10000` in the firmware).
During those 10 seconds, voice steering commands are refused outright — even
correct, authenticated, freshly spoken ones. Two reasons, not one. The obvious
one: the human just took over and the system should get out of the way. The
subtler one: a voice link that was *flapping* — dropping and reconnecting, or
echoing a stale "port ten" out of a buffer — should not be able to resume
steering the instant the human lets go. The 10 seconds is a cool-down that
forces the voice side to re-establish *intent*, not merely re-establish
connection.

### 3 · Protocol: authenticated, replay-proof, single-master

The first two layers assume the command reaching the Helm unit is the *right*
command. Layer 3 guarantees that.

- **Every command travels inside an encrypted session.** Pairing is done by
  scanning a QR code printed on the unit, which carries the unit's public key;
  the phone and unit then run an authenticated key exchange. All traffic — BLE
  and WiFi alike — is inside that encrypted session. (WiFi security never
  *depends* on WPA2/WPA3 alone; the session encryption is independent of
  whatever the boat's WiFi happens to be configured as.)
- **Per-message monotonic counter — replays are dropped.** A *monotonic counter*
  is a number that only ever goes up. Every message carries the next number in
  its sequence; the unit rejects any message whose counter isn't ahead of the
  last one it accepted. So an attacker who records a perfectly valid "port ten"
  packet and replays it later gets nowhere — the counter is stale. This is what
  answers "the wrong brain is talking, using a captured packet."
- **Exactly one brain holds the helm token.** The *helm token* is the
  right-to-steer; exactly one paired device may hold it at a time. A second
  phone can *listen* (telemetry, dictation) but cannot steer. Transferring the
  token to a new device is an explicit, logged action — not something that
  happens automatically or silently.
- **Link loss = immediate release to standby.** If the heartbeat goes silent
  for more than **750 ms** (`LINK_GRACE_MS`), the firmware releases all outputs.
  Note what "release to standby" means: the autopilot itself keeps doing
  whatever *it* was doing — Cocapn is a *remote control*, not a replacement for
  the pilot. We stop pressing the remote buttons; the autopilot's own
  hold-heading or track mode is unaffected. This answers "the brain disappears
  mid-command."

### 4 · Voice: command classes and escalating confirmation

This is the layer most readers came here for: what the captain is actually
allowed to say, and what the system does about it. The core idea is that *not
all voice commands carry the same risk*, so they should not all carry the same
burden of confirmation.

A **command class** is a tier that groups commands by how much damage a wrong
one does, so the confirmation burden scales with the stakes. There are four:

| Class | What it is | Examples | What the system does before acting |
|---|---|---|---|
| **C0 · INFO** | Read-only questions. The system tells you something; it moves nothing. | "What's our heading?" | Nothing — no confirmation, because there is nothing to confirm. |
| **C1 · TRIM** | Small, instantly-reversible nudges to an already-safe autopilot. | "Port ten," "starboard five," a quick dodge. | No confirmation required, **but** the system speaks the command back ("port ten") *before* acting — that echo *is* your cancel window — and a "belay that" at any point kills it instantly. The risk is low because the boat was already steering a sane course and a ten-degree nudge is easy to undo. |
| **C2 · MODE** | Changes *what the autopilot is doing*, or large course changes. | "Auto," "standby," "tack," any course change over 30°. | **Spoken confirmation required.** The system asks "engage auto?"; the captain must say "confirm" within **6 seconds** (`CONFIRM_WINDOW_MS`), or the command is belayed. The risk is higher because mode changes and big turns are not self-correcting the way a trim nudge is. |
| **C3 · PROPULSION / anything irreversible** | Throttle, gear, winches — anything that moves mass you cannot immediately call back. | Throttle, anchor winch, hydraulics. | **Disabled by default.** Enabling C3 at all requires dockside setup, plus per-command confirmation, plus a *physical enable switch* on the unit itself (`c3_switch_closed` in the firmware, read every tick). The risk is highest because these commands do things you cannot undo by shouting "belay." |

Three load-bearing details sit around that table:

- **The grammar is closed.** "Closed" means the system only ever tries to match
  what you say against a fixed list of a few dozen phrases defined by the
  installed profile — it is not transcribing open speech and then guessing what
  you meant. This is what makes recognition reliable in wind and engine noise:
  the question is not "what did the captain probably say?" but "which of these
  thirty phrases is closest?"
- **Below the confidence floor, the system asks, never guesses.** Speech
  recognition returns a confidence score; if it is under **0.85**
  (`CONFIDENCE_FLOOR`), the system does *not* pick the closest match and run it.
  It asks you to repeat. A wrong guess on "port" versus "starboard" is not worth
  the 0.15 of confidence.
- **"Belay" / "cancel" / touching anything physical kills the active command.**
  And — critically — **dictation mode cannot actuate, ever.** Dictation (the
  open-vocabulary "remember this for the log" mode) and command (the "steer the
  boat" mode) are *separate code paths by construction*, not different settings
  on one pipeline. You cannot talk the boat into a turn by dictating a sentence
  that happens to contain the word "port." A wake word ("cocapn") is required
  before every command; there is no open-mic actuation.

### 5 · Human: sea-trial checklist and honest documentation

The first four layers are things the *system* does. The fifth is something the
*project* does, and it is the one that ultimately decides whether any of the
above is true on a given boat.

- **No install is marked complete until a guided sea trial passes — with the
  installer's hands on the wheel.** The provisioner (the dockside installer
  tool) walks a four-test checklist on the water: the **override test** (touch
  the real controls → outputs drop, lockout begins), the **link-kill test**
  (kill the phone's radio → outputs release within the grace window), the
  **TTL test** (let a command run without re-heartbeating → it releases at
  500 ms), and the **wrong-word test** (say something off-grammar → the system
  asks, it does not guess). These tests exist because they re-enact,
  deliberately and in calm water, each line on the failure list at the top of
  this document.
- **Every wiring card carries the same sentence:** *"This device is a remote
  control accessory. The vessel's master remains responsible for navigation and
  watchkeeping at all times."* The design is explicit about what the system is
  and is not. It is a remote — a polite, logged, fail-safe remote — not an
  autopilot, and not a substitute for the watchkeeper.

## What voice never does (v1 policy)

Even with all five layers above, the v1 design refuses entire categories on
purpose:

- **No waypoint or route following** beyond what the autopilot itself already
  does. Cocapn presses the remote's buttons; it does not become a navigator.
- **No propulsion, no anchor winch, no hydraulics.** C3 is disabled by default
  (see the table above) and is expected to stay disabled for v1.
- **No actuation from dictation mode, chat mode, or any cloud path.** Every
  actuation originates only from the *local* command pipeline — the one running
  on the device holding the helm token. The cloud can listen; the cloud cannot
  steer.

These limits are policy, not missing features. They exist to make the v1 safety
claim small enough to actually verify.

## Logged accountability

Every command, confirmation, override, link event, and watchdog trip is an
**ActiveLog** event — a timestamped, append-only record (see `ARCHITECTURE.md`,
Layer 3). After any incident, the exact sequence of what the system heard, what
it did, and when it released is replayable to the millisecond. This is not
incidental: it is what makes the whole architecture *trustworthy* rather than
merely *safe on paper*. It protects the captain (the log shows they belayed in
time), the installer (the log shows the sea trial passed), and the project (the
log shows the system did what it was designed to do). A design that cannot be
audited after the fact cannot be trusted before the fact; this layer exists so
that it can.
