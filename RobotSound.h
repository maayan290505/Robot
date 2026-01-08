#pragma once
#include <Arduino.h>

struct BeepStep {
  uint16_t freq; // Hz, 0 = silence
  uint16_t ms;   // duration
};

class RobotSound {
public:
  enum Style : uint8_t {
    SOUND_SOFT    = 1,
    SOUND_SCIFI   = 2,
    SOUND_MINIMAL = 3,
  };

  explicit RobotSound(uint8_t pin, Style defaultStyle = SOUND_SCIFI)
  : _pin(pin), _defaultStyle(defaultStyle),
    _steps(nullptr), _count(0), _idx(0), _active(false), _nextMs(0) {}

  void begin() {
    pinMode(_pin, OUTPUT);
    noTone(_pin);
    _active = false;
  }

  // Call every loop (non-blocking)
  void update() {
    if (!_active) return;

    const unsigned long now = millis();
    if (now < _nextMs) return;

    _idx++;
    if (_idx >= _count) { stop(); return; }
    startStep(_steps[_idx]);
  }

  void stop() {
    noTone(_pin);
    _active = false;
    _steps = nullptr;
    _count = 0;
    _idx = 0;
    _nextMs = 0;
  }

  bool isPlaying() const { return _active; }

  // Default style (used by tap()/longHold()/release() with no args)
  void setDefaultStyle(Style s) { _defaultStyle = s; }
  Style getDefaultStyle() const { return _defaultStyle; }

  // ---------- API: default style ----------
  void tap()      { tap(_defaultStyle); }
  void longHold() { longHold(_defaultStyle); }
  void release()  { release(_defaultStyle); }

  // ---------- API: per-call style override ----------
  void tap(Style s)      { const Seq q = seqTap(s);      play(q.steps, q.count); }
  void longHold(Style s) { const Seq q = seqLongHold(s); play(q.steps, q.count); }
  void release(Style s)  { const Seq q = seqRelease(s);  play(q.steps, q.count); }

  // Optional: play your own sequences
  void playCustom(const BeepStep* seq, uint8_t count) { play(seq, count); }

private:
  struct Seq { const BeepStep* steps; uint8_t count; };

  void play(const BeepStep* seq, uint8_t count) {
    if (!seq || count == 0) return;
    _steps = seq;
    _count = count;
    _idx = 0;
    _active = true;
    startStep(_steps[0]);
  }

  void startStep(const BeepStep& s) {
    if (s.freq == 0) noTone(_pin);
    else tone(_pin, s.freq);
    _nextMs = millis() + s.ms;
  }

  // ----------------- SEQUENCES -----------------
  static inline const BeepStep SFX_TAP_SOFT[3] = {
    {880, 35}, {0, 30}, {1175, 55}
  };
  static inline const BeepStep SFX_HOLD_SOFT[5] = {
    {660, 70}, {0, 90}, {880, 70}, {0, 90}, {1320, 110}
  };
  static inline const BeepStep SFX_REL_SOFT[3] = {
    {1175, 45}, {0, 40}, {880, 85}
  };

  static inline const BeepStep SFX_TAP_SCIFI[5] = {
    {1800, 25}, {0, 25}, {1400, 45}, {0, 20}, {2000, 25}
  };
  // (Pauses made longer so it doesn't feel "too close")
  static inline const BeepStep SFX_HOLD_SCIFI[7] = {
    {1200, 60}, {0, 90},
    {1500, 60}, {0, 90},
    {1800, 70}, {0, 110},
    {2200, 120}
  };
  static inline const BeepStep SFX_REL_SCIFI[5] = {
    {2000, 22}, {0, 30}, {1500, 45}, {0, 20}, {1100, 70}
  };

  static inline const BeepStep SFX_TAP_MINI[2] = {
    {1500, 22}, {0, 18}
  };
  static inline const BeepStep SFX_HOLD_MINI[5] = {
    {1400, 60}, {0, 80}, {1700, 60}, {0, 90}, {2000, 90}
  };
  static inline const BeepStep SFX_REL_MINI[3] = {
    {1700, 22}, {0, 25}, {1400, 50}
  };

  static constexpr uint8_t lenTapSoft()  { return (uint8_t)(sizeof(SFX_TAP_SOFT)  / sizeof(SFX_TAP_SOFT[0])); }
  static constexpr uint8_t lenHoldSoft() { return (uint8_t)(sizeof(SFX_HOLD_SOFT) / sizeof(SFX_HOLD_SOFT[0])); }
  static constexpr uint8_t lenRelSoft()  { return (uint8_t)(sizeof(SFX_REL_SOFT)  / sizeof(SFX_REL_SOFT[0])); }

  static constexpr uint8_t lenTapSci()   { return (uint8_t)(sizeof(SFX_TAP_SCIFI) / sizeof(SFX_TAP_SCIFI[0])); }
  static constexpr uint8_t lenHoldSci()  { return (uint8_t)(sizeof(SFX_HOLD_SCIFI)/ sizeof(SFX_HOLD_SCIFI[0])); }
  static constexpr uint8_t lenRelSci()   { return (uint8_t)(sizeof(SFX_REL_SCIFI) / sizeof(SFX_REL_SCIFI[0])); }

  static constexpr uint8_t lenTapMini()  { return (uint8_t)(sizeof(SFX_TAP_MINI)  / sizeof(SFX_TAP_MINI[0])); }
  static constexpr uint8_t lenHoldMini() { return (uint8_t)(sizeof(SFX_HOLD_MINI) / sizeof(SFX_HOLD_MINI[0])); }
  static constexpr uint8_t lenRelMini()  { return (uint8_t)(sizeof(SFX_REL_MINI)  / sizeof(SFX_REL_MINI[0])); }

  Seq seqTap(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_TAP_SOFT,  lenTapSoft() };
      case SOUND_MINIMAL: return { SFX_TAP_MINI,  lenTapMini() };
      case SOUND_SCIFI:
      default:            return { SFX_TAP_SCIFI, lenTapSci() };
    }
  }

  Seq seqLongHold(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_HOLD_SOFT,  lenHoldSoft() };
      case SOUND_MINIMAL: return { SFX_HOLD_MINI,  lenHoldMini() };
      case SOUND_SCIFI:
      default:            return { SFX_HOLD_SCIFI, lenHoldSci() };
    }
  }

  Seq seqRelease(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_REL_SOFT,  lenRelSoft() };
      case SOUND_MINIMAL: return { SFX_REL_MINI,  lenRelMini() };
      case SOUND_SCIFI:
      default:            return { SFX_REL_SCIFI, lenRelSci() };
    }
  }

  // ----------------- STATE -----------------
  uint8_t _pin;
  Style _defaultStyle;

  const BeepStep* _steps;
  uint8_t _count;
  uint8_t _idx;
  bool _active;
  unsigned long _nextMs;
};
