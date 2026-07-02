# cocapn-desktop

The laptop at the helm. Same core logic as the phone app (shared spec, shared
grammar/profile handling), different strengths:

- **Local STT without platform dependence:** whisper.cpp (tiny/base) or Vosk
  small model. The command set is closed grammar, so "nothing fancy" is truly
  enough — a small model matching against a few dozen phrases beats a big model
  doing open transcription.
- **Review and annotation surface:** the day timeline, transcript search,
  correction events (`correction.amend`), label-export runs for training data.
- **Bridge duties:** can hold the WiFi link to Helm units and cameras while the
  phone stays in the captain's pocket on BLE; navigation-software screenshots and
  chart overlay experiments live here first.

## Stack

Tauri (Rust core + web UI) or Electron; whisper.cpp bound natively. The UI reuses
the app's profile-driven Helm rendering (same JSON → same control head).

## Rule

Desktop is a *peer brain*, not a server. It writes the same ActiveLog streams
with its own `dev` id; merge-by-union means phone and laptop never conflict.
Helm token still applies: only one brain steers at a time.
