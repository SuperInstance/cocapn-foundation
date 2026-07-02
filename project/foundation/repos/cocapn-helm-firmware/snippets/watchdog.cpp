// watchdog.cpp — the SafetySupervisor: the only component allowed to say no.
//
// Chain of authority (highest first):
//   1. physics        — outputs are normally-open; power loss = release
//   2. hardware WDT   — stalled loop resets chip; boot state = released
//   3. this supervisor— TTL, override, link loss, class gates
//   4. profile engine — executes only what the supervisor permits
//   5. the app        — merely a polite requester

#include "autopilot_hal.h"
#include <esp_task_wdt.h>

static constexpr uint32_t HW_WDT_TIMEOUT_MS   = 2000;  // stalled loop -> reset
static constexpr uint32_t OVERRIDE_LOCKOUT_MS = 10000; // human touched the helm
static constexpr uint32_t LINK_GRACE_MS       = 750;   // heartbeat silence -> release

class SafetySupervisor {
  uint32_t last_heartbeat_ms = 0;
  uint32_t action_started_ms = 0;
  uint32_t action_ttl_ms     = 0;
  uint32_t action_hard_cap_ms= 0;
  uint32_t override_until_ms = 0;
  bool     armed             = false;   // set only after profile load + link auth
  bool     c3_switch_closed  = false;   // physical enable, read every tick

 public:
  void begin() {
    esp_task_wdt_init(HW_WDT_TIMEOUT_MS / 1000, /*panic=*/true);
    esp_task_wdt_add(nullptr);
    // Boot posture: everything open. release_all() is idempotent and cheap;
    // call it even though hardware already guarantees this — belt and suspenders.
  }

  // Called every loop tick (~10 ms). The ONE place release decisions live.
  void tick(ProfileEngine& engine, uint32_t now_ms) {
    esp_task_wdt_reset();

    if (read_override_input()) {                    // human on the real controls
      override_until_ms = now_ms + OVERRIDE_LOCKOUT_MS;
      engine.release_all();
      log_event("helm.event", "override");
    }
    c3_switch_closed = read_c3_enable_input();

    const bool link_stale = (now_ms - last_heartbeat_ms) > LINK_GRACE_MS;
    const bool ttl_expired = action_ttl_ms &&
        (now_ms - action_started_ms) > action_ttl_ms;
    const bool capped = action_hard_cap_ms &&
        (now_ms - action_started_ms) > action_hard_cap_ms;

    if (link_stale || ttl_expired || capped) {
      engine.release_all();
      action_ttl_ms = 0;
      if (link_stale) log_event("helm.event", "link_release");
    }
  }

  // Gate called by ProfileEngine::execute() BEFORE any output moves.
  bool permit(const Action& a, uint32_t now_ms) {
    if (!armed)                         return deny("not_armed");
    if (now_ms < override_until_ms)     return deny("override_lockout");
    if (a.safety_class >= 3 && !c3_switch_closed)
                                        return deny("c3_switch_open");
    action_started_ms  = now_ms;
    action_ttl_ms      = a.ttl_ms ? a.ttl_ms : 500;
    action_hard_cap_ms = a.max_repeat_ms;           // 0 = single-shot only
    return true;
  }

  void on_heartbeat(uint32_t now_ms) { last_heartbeat_ms = now_ms; }

 private:
  bool deny(const char* why) { log_event("helm.event", why); return false; }
  bool read_override_input();
  bool read_c3_enable_input();
  void log_event(const char* type, const char* kind);  // -> ActiveLog via link
};
