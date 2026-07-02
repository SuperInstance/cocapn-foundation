// autopilot_hal.h — the profile interpreter's core interfaces.
// There is no per-brand code. A Profile (JSON) fully describes a device's
// remote interface; this HAL executes it under safety supervision.
//
// Design rules embodied here:
//   * Outputs are momentary. There is no API to latch anything.
//   * Every actuation carries a TTL. Expiry releases, unconditionally.
//   * The safety supervisor (watchdog.cpp) can slam everything open at any time
//     and outranks every caller.

#pragma once
#include <stdint.h>

// ---- Electrical primitives a profile may use --------------------------------
// A profile compiles to a sequence of these. Adding a new autopilot never adds
// a new primitive unless the *electrical physics* is genuinely new.

enum class OutKind : uint8_t {
  CONTACT,     // optocoupled dry-contact closure on channel N
  RESISTOR,    // present resistance R between ladder terminals (analog remotes)
  NMEA0183,    // transmit sentence on serial port
  NMEA2000,    // transmit PGN on CAN
  PWM,         // duty cycle on channel N (never for steering in v1)
};

struct OutStep {
  OutKind kind;
  uint8_t channel;      // CONTACT/PWM channel or serial/CAN port index
  uint32_t value;       // ohms (RESISTOR), duty*1000 (PWM), else 0
  const char* payload;  // NMEA sentence/PGN template, else nullptr
  uint16_t hold_ms;     // how long this step is held
  uint16_t gap_ms;      // pause before next step
};

// An Action = named recipe of steps + its safety class + repeat semantics.
struct Action {
  const char* name;       // "port_10", "mode_auto", "dodge_port"
  uint8_t safety_class;   // 0..3 (C0..C3) — enforced HERE, not only in the app
  const OutStep* steps;
  uint8_t n_steps;
  // For "held" behaviors (dodge): the brain must re-send before ttl_ms elapses.
  // Holding is re-pulsing under live software control — never a latched output.
  uint16_t ttl_ms;        // default 500
  uint16_t max_repeat_ms; // hard cap regardless of heartbeats (e.g. dodge 10 s)
};

// ---- Execution under supervision -------------------------------------------

class SafetySupervisor;  // watchdog.cpp: TTL bookkeeping, override, kill

class ProfileEngine {
 public:
  // Parse + validate profile JSON at boot. Rejects: unknown primitives,
  // steps exceeding class limits, C3 actions when enable switch is open,
  // any action with no ttl. Invalid profile -> unit refuses to arm.
  bool load(const char* profile_json);

  // Called by the link layer for an authenticated, replay-checked command
  // from the current helm-token holder. Returns false (and logs) if the
  // supervisor vetoes: override lockout, TTL exhausted, class gate, not armed.
  bool execute(const char* action_name, SafetySupervisor& sup);

  // Heartbeat from the brain extends the active action's TTL (up to max_repeat_ms).
  void heartbeat(uint32_t session_counter);

  // Unconditional release of all outputs. Called by supervisor on TTL expiry,
  // override, link loss, watchdog pre-reset hook, and at every boot.
  void release_all();
};

// ---- What is deliberately absent --------------------------------------------
// * No set_output(pin, HIGH). Raw pin control is not an API.
// * No persistent output state. Reboot = everything open, always.
// * No route/waypoint logic. We emulate a human hand on a remote, nothing more.
