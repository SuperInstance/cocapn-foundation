// voice_state_machine.ts — the mode + segmentation core, portable pseudocode.
// The hard-won decisions are in the comments.

type Mode = "IDLE" | "COMMAND_LISTEN" | "COMMAND_CONFIRM" | "DICTATION";

interface Clock { wallMs(): number; monoMs(): number; }   // captured at AUDIO time
interface GpsFix { lat: number; lon: number; sog?: number; cog?: number; }

interface Segment {
  ts: number; mono: number;          // stamped when the FIRST audio frame arrived
  text: string; durMs: number; conf: number;
  fix?: GpsFix;
}

const PAUSE_CLOSES_SEGMENT_MS = 2500;   // "extended pause" — captain-tunable
const CONFIRM_WINDOW_MS = 6000;         // C2 confirm must arrive within this
const CONFIDENCE_FLOOR = 0.85;          // below -> ask, never guess

class VoicePipeline {
  mode: Mode = "IDLE";
  private segStart?: { ts: number; mono: number };   // set on first voiced frame
  private pending?: { action: string; expiresMono: number }; // awaiting C2 confirm

  // ---- wake word fires (on-device engine, always running) ----
  onWake() {
    // Wake NEVER interrupts an open dictation segment mid-sentence;
    // it closes the segment first so the log stays clean, then switches.
    if (this.mode === "DICTATION") this.closeOpenSegment();
    this.mode = "COMMAND_LISTEN";
    this.startCommandStt();       // platform STT, short utterance, then auto-stop
  }

  // ---- command path (closed grammar; profile-supplied) ----
  onCommandSttResult(text: string, conf: number, clock: Clock) {
    const match = this.grammar.match(text);   // fuzzy over profile phrases only
    if (!match || conf < CONFIDENCE_FLOOR) return this.askAgain(text);

    if (this.grammar.isCancel(text)) return this.belay();

    if (match.class === "C2") {
      this.speak(match.confirmPrompt);        // "engage auto?"
      this.pending = { action: match.action, expiresMono: clock.monoMs() + CONFIRM_WINDOW_MS };
      this.mode = "COMMAND_CONFIRM";
      return;
    }
    this.speak(match.echo);                   // "port ten" — echo BEFORE acting:
    this.execute(match.action);               // the echo IS the cancel window
    this.logHelmCommand(match, "executed");
    this.mode = this.resumeMode();
  }

  onConfirmResult(text: string, clock: Clock) {
    const yes = /\b(confirm|yes|do it)\b/i.test(text);
    const expired = this.pending && clock.monoMs() > this.pending.expiresMono;
    if (this.pending && yes && !expired) this.execute(this.pending.action);
    else this.speak("belayed");
    this.pending = undefined;
    this.mode = this.resumeMode();
  }

  // ---- dictation path (open vocabulary; NO route to execute()) ----
  onVoicedFrame(clock: Clock) {
    // Timestamp discipline: the segment's ts/mono are the FIRST audio frame,
    // not when STT returns. Labels anchor to when words were SPOKEN.
    if (this.mode !== "DICTATION") return;
    if (!this.segStart) this.segStart = { ts: clock.wallMs(), mono: clock.monoMs() };
    this.pauseTimer.reset(PAUSE_CLOSES_SEGMENT_MS, () => this.closeOpenSegment());
  }

  onDictationText(partial: string) { this.buffer = partial; /* streaming display */ }

  private closeOpenSegment() {
    if (!this.segStart || !this.buffer.trim()) { this.segStart = undefined; return; }
    const seg: Segment = {
      ts: this.segStart.ts, mono: this.segStart.mono,
      text: this.buffer.trim(),
      durMs: this.clock.monoMs() - this.segStart.mono,
      conf: this.lastConf, fix: this.gps.best(),
    };
    this.log.append("speech.segment", seg);       // append-only, local first
    for (const a of parseAssertions(seg))          // deterministic, conservative
      this.log.append("catch.assertion", a);       // ambiguous -> emit nothing
    this.segStart = undefined; this.buffer = "";
  }

  // Dictation on/off is itself voice-driven: "cocapn, start dictation" /
  // "cocapn, stop dictation" are C0 grammar entries — no touching required.
  private resumeMode(): Mode { return this.dictationWasOn ? "DICTATION" : "IDLE"; }
}
