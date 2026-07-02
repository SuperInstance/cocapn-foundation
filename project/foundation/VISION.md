# VISION — Cocapn, ActiveLog, and the Vessel-as-a-Robot

*A thinking document. Audience: us.*

---

## 1. The wedge

The killer MVP is deliberately small:

1. **Free timestamped dictation, tuned for fishing work.** A captain talks all day;
   the phone remembers *when* everything happened, segments on extended pauses,
   stamps GPS. No hardware, no account, no cloud. This is the download hook.
2. **Voice autopilot via a $40 box.** The unused "Helm" tab in the app is the
   upsell that sells itself: once a captain narrates his day hands-free, steering
   hands-free is obvious.

Everything else — cameras, sounders, fleet sync, RL — rides in on data these two
features already produce. **The captain never does data-entry. Labeled data is a
byproduct of talking while working.** That sentence is the whole company.

## 2. Why the log is the moat

The code is open source; anyone can fork it. What can't be forked:

- **The spec + network effect.** If ActiveLog is the format that chartplotter
  overlays, camera trainers, and fleet dashboards consume, every new consumer makes
  every producer more valuable. Formats win by being boring, open, and everywhere.
- **The profile library.** Hundreds of community-verified device profiles (this
  autopilot, that throttle, this pump) is a Wikipedia-style asset. Forks start at
  zero profiles.
- **The domain grammar.** Years of accumulated fishing vocabulary, command sets,
  and confirmation phrasing tuned on real decks in real wind. Small data, huge UX gap.
- **The maker channel.** Vendors and makers selling preloaded, pre-wired Helm units
  under Makerlog affiliation. Distribution is a moat software can't copy.
- **Trust.** A published safety architecture, sea-trial checklists, and a track
  record. Nobody wires a no-name fork into their steering.

## 3. Three years out

- Cocapn is the default "talk to your boat" app in a few fisheries (troll, longline,
  gillnet on the Pacific coast first — word of mouth travels dock to dock).
- The Helm unit is a product line: autopilot remote, throttle, lighting, pumps,
  engine gauges — same board, different profile and wiring card.
- The **conversational provisioner** is real: a captain tells Cocapn "I want the
  deck lights on a voice command," the system generates the profile + firmware
  config, and prints a wiring card for the installer. The human is the wiring
  assistant; the system is the engineer.
- ActiveLog events overlay on navigation software: where fish bit, where gear went
  down, what the captain said there. Season-over-season replay.
- First fleets share a BYO Cloudflare instance; a skipper asks "where did the fleet
  find coho this week?" and gets a chart layer.
- Camera work matures: deck cameras cross-checked against the captain's narration
  ("lingcod in the port tote") deliver species/size/count logging that regulators and
  buyers start to trust. Sonarvision's ideas live here.

## 4. Five years out

- **Vessel-as-a-Robot (VaaR)** is a recognized category and we named it. A boat is
  a fleet of cheap actuators/sensors (Helm units) + one brain (the captain's phone)
  + one memory (ActiveLog). Autonomy arrives incrementally and per-captain: the
  system first *watches* (log), then *suggests* (assist), then *acts under
  supervision* (execute with confirmation) — each captain moves each function up
  that ladder at their own pace, and the ladder is auditable in the log.
- The substrate generalizes: studylog.ai, reallog.ai, playerlog.ai, businesslog.ai
  are thin domain grammars over the same spec + apps. "Timestamped narration +
  cheap actuators + BYOK intelligence" turns out to be a general shape.
- Makers are an economy: makerlog.ai lists certified builders; a profile author in
  Norway sells preloaded units for a Simrad installation she's never seen, because
  the profile + provisioner carry the knowledge.
- Research groups train on (opt-in, captain-owned) ActiveLog corpora — the richest
  labeled dataset of small-vessel fishing behavior ever collected, owned by the
  people who made it.

## 5. The ecosystem of people

| Actor | What they get | What they give |
|---|---|---|
| **Captain** | Hands-free helm, a perfect memory of the season, answers about his own fishing | Narration (= labels), word of mouth |
| **Crew** | Hands-free logging while working gear | Narration, corrections |
| **Installer/technician** | A guided, meter-in-hand install that makes them the local expert | Verified profiles, install reports |
| **Maker/vendor** | A product line with software they don't write | Hardware sales channel, support |
| **Developer** | A clean spec, real data, honest hardware problems | Profiles, consumers, apps |
| **Fleet manager** | Cross-vessel knowledge on a chart | Hosting (BYO cloud) |
| **Researcher** | Consented labeled data | Models back to the commons |

## 6. What we must get right *today* for the 5-year picture to be reachable

1. **Spec before features.** ActiveLog's schema, ID scheme, and merge rules are the
   one thing that must not churn. Version it from day one (`alv: 1`).
2. **Profiles as data.** If autopilot support is C++ per-device, an AI can't safely
   author new device support and the maker economy never happens. The firmware must
   be a *profile interpreter*.
3. **Safety architecture published first.** It is the marketing. It is also the
   liability shield and the reason a technician will touch the steering.
4. **Two-mode setup discipline.** Every feature must answer: what does dockside
   setup look like, and what does sea setup look like? If sea setup isn't
   self-explanatory, the feature isn't done.
5. **Own the timestamps.** Monotonic clock discipline, GPS time reconciliation,
   per-device sequence numbers. Every future promise (media anchoring, fleet merge,
   RL) rests on trustworthy time.
6. **BYOK from the start, hosted later.** Captains bring a DeepSeek/OpenAI/Anthropic
   key; we never sit between a captain and his data or his model. Hosting is a
   convenience tier, not a chokepoint. (Decision: BYOK chat ships as a designed
   hook — interface + event types now, polished UX in Phase 2.)
7. **Free tier stays genuinely great.** fishinglog.ai (no hardware) must be a tool
   a captain would pay for, given away. That's the funnel.

## 7. Business, lightly

Donations + hardware. Helm units sold preloaded through vendors/makers (affiliate,
makerlog.ai). Later: hosted sync with a banner, nominal fee to remove. Nothing in
the architecture may assume revenue-side lock-in — the moat is the ecosystem, not
a paywall.
