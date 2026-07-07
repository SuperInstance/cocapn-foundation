# EDUCATIONAL_NOTES — choices made in this pass

Written to satisfy the `BRIEF.md` requirement to document the specific editorial
decisions behind the rewrite. Read alongside `STYLE_BRIEF.md`, which is the
standard this pass was measured against.

## Which files I touched, and why

The BRIEF said "rewrite `README.md` (and `SAFETY.md` if it's the one carrying
the technical weight — use your judgment)." The real file structure turned out
to decide this:

- The **top-level `README.md`** is *not* a Cocapn project README — it is Claude
  Design's handoff-wrapper boilerplate ("CODING AGENTS: READ THIS FIRST"),
  instructing a future coding agent how to implement the HTML prototypes in
  `project/Cocapn Foundation.dc.html`. Turning it into a safety-architecture
  explainer would (a) destroy its actual function in that handoff and (b) put
  the project's real entry point in the wrong place. I left it untouched.
- **`project/foundation/SAFETY.md`** is where the technical weight actually
  lives — command classes, the TTL / watchdog / override mechanism, the
  five-layer defense. This got the deepest pass.
- **`project/foundation/README.md`** is the real project entry point. It got a
  lighter pass: a motivational opening (the BRIEF's "felt scenario before
  mechanism" requirement applies here too), inline jargon definitions for
  `profile` and `append-only log`, and a corrected `SAFETY.md` pointer.

If the underlying intent was actually "fix the repo *landing page*," that is a
separate task from "teach the safety architecture," and it would mean replacing
the handoff wrapper with a proper top-level README — a scope change, not a
clarity pass.

## Specific editorial choices

1. **Failure scenario before any mechanism.** The original `SAFETY.md` opened
   with a dry threat-model list and went straight into "the five-layer defense,"
   so a reader with no background hit `normally-open relays`, `TTL`, `watchdog`,
   `monotonic counter` in the first screen with no motivation. The rewrite
   opens with a single troll-boat scene that walks through six concrete ways
   blindly obeying voice goes wrong; each later layer is then framed as the
   answer to a specific line on that list. All six original threat-model
   entries are preserved inside the narrative and named once explicitly so the
   formal artifact is not silently dropped.

2. **The "deadband" vocabulary problem, handled honestly.** The BRIEF uses
   "deadband/lockout" to describe the mechanism; the foundation `README.md`'s
   own pointer calls it "dead-man logic"; the actual `SAFETY.md` source uses
   *de-energized-safe*, *momentary-only*, and *command TTL*. I did not pretend
   the source uses a word it does not. The rewrite introduces the general
   control term (*deadband*, *dead-man's switch*) in a short principle section,
   names the three source terms as "three names for the same property, each
   from a different layer," and then teaches each layer's version of it. The
   reader learns both the general concept and the project's actual vocabulary.

3. **Command classes taught by risk, not label.** The original C0–C3 table is
   accurate but terse; a newcomer reads "C1 TRIM ... none" and does not know
   why trim gets less confirmation than mode. The rewrite keeps the table
   (every row, every original example) and adds a "what the system does before
   acting" column plus a lead paragraph stating the grading principle:
   confirmation burden scales with stakes. C3's three-gate default-disabled
   policy (dockside setup + per-command confirm + physical enable switch) is
   preserved exactly.

4. **Lockout gets its own short subsection** with the real trigger (human
   touches real helm controls) and duration (10 s, `OVERRIDE_LOCKOUT_MS =
   10000`), plus the less-obvious second reason for the cool-down (a flapping
   voice link should not resume steering the instant the human lets go).
   Numbers verified against `watchdog.cpp`.

5. **Every number preserved and sourced.** 500 ms TTL, 2000 ms watchdog,
   10 000 ms override lockout, 750 ms link grace, 6 s C2 confirm window,
   0.85 confidence floor, >30° course-change threshold — all checked against
   `SAFETY.md`, `watchdog.cpp`, and `voice_state_machine.ts` before writing.
   No number was rounded, dropped, or invented.

6. **"Not yet built" framing strengthened.** The original `SAFETY.md` had a
   strong line ("nobody sane wires an internet-adjacent hobby board into their
   steering, and they're right not to") but was otherwise written in the
   present tense ("the Helm unit wires...", "actuation outputs are..."), which
   a casual reader could mistake for a description of shipping hardware. The
   rewrite leads both foundation documents with an explicit status block:
   design brief, not product; Phase 0 is the logging app; the first Helm unit
   is Phase 1; the safety spec is detailed *because it has to be*, not because
   it has been validated in the field.

7. **No two-tier split.** Per `STYLE_BRIEF.md`'s explicit instruction, there is
   one `SAFETY.md`, not a "simple" and a "real" version. Every term is defined
   inline at first use; nothing is relegated to a glossary or a TL;DR. The
   target is one document a newcomer and a control-systems engineer both read
   end to end.

## What I deliberately did not do

- Did not touch any source code (`watchdog.cpp`, `voice_state_machine.ts`,
  `autopilot_hal.h`, profile JSON). The BRIEF said don't.
- Did not touch `ARCHITECTURE.md`, `VISION.md`, `ROADMAP.md`, or any per-repo
  README. They were read for grounding (especially the "not yet built" status
  and the verified numbers) but are out of scope for this clarity pass.
- Did not invent any capability, command, threshold, or scenario that is not
  traceable to a source document. The troll-boat opener is illustrative, but
  every mechanism it invokes (misheard word, latched command, link loss,
  unauthorized packet, welded relay, man-overboard manual takeover) maps to a
  documented threat-model entry.
