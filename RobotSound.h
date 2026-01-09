#pragma once
#include <Arduino.h>

#ifdef N
  #undef N
#endif

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
  void hello()    { const Seq q = seqHello(_defaultStyle); play(q.steps, q.count); }
  void intro()    { const Seq q = seqIntro(_defaultStyle); play(q.steps, q.count); } // "M.A.R.O.M.Y"

  // ---------- API: per-call style override ----------
  void tap(Style s)      { const Seq q = seqTap(s);      play(q.steps, q.count); }
  void longHold(Style s) { const Seq q = seqLongHold(s); play(q.steps, q.count); }
  void release(Style s)  { const Seq q = seqRelease(s);  play(q.steps, q.count); }
  void hello(Style s)    { const Seq q = seqHello(s);    play(q.steps, q.count); }
  void intro(Style s)    { const Seq q = seqIntro(s);    play(q.steps, q.count); }

  // ---------- Moods / events (default style) ----------
  void happy() { const Seq q = seqHappy(_defaultStyle); play(q.steps, q.count); }
  void angry() { const Seq q = seqAngry(_defaultStyle); play(q.steps, q.count); }
  void tired() { const Seq q = seqTired(_defaultStyle); play(q.steps, q.count); }
  void cold()  { const Seq q = seqCold(_defaultStyle);  play(q.steps, q.count); }

  // ---------- Moods / events (choose style per call) ----------
  void happy(Style s) { const Seq q = seqHappy(s); play(q.steps, q.count); }
  void angry(Style s) { const Seq q = seqAngry(s); play(q.steps, q.count); }
  void tired(Style s) { const Seq q = seqTired(s); play(q.steps, q.count); }
  void cold(Style s)  { const Seq q = seqCold(s);  play(q.steps, q.count); }

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

  // ----------------- SEQUENCES: tap/hold/release -----------------
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

  // ----------------- INTRO: "M.A.R.O.M.Y" (tones only) -----------------
  // Tip: tweak notes/durations to taste.
  static inline const BeepStep SFX_INTRO[11] = {
    { 784, 120 }, { 0, 40 },   // M
    { 659, 120 }, { 0, 40 },   // A
    { 784, 120 }, { 0, 40 },   // R
    { 587, 140 }, { 0, 40 },   // O
    { 659, 120 }, { 0, 40 },   // M
    { 988, 200 }               // Y
  };

  // ----------------- HELLO: friendly motif -----------------
  static inline const BeepStep SFX_HELLO_SOFT[7] = {
    {880, 90}, {0, 60},
    {1047, 90}, {0, 60},
    {1319, 110}, {0, 70},
    {988, 140}
  };
  static inline const BeepStep SFX_HELLO_SCIFI[9] = {
    {1400, 60}, {0, 35},
    {1700, 60}, {0, 35},
    {2100, 65}, {0, 45},
    {2600, 80}, {0, 60},
    {1900, 140}
  };
  static inline const BeepStep SFX_HELLO_MINI[5] = {
    {1800, 40}, {0, 40},
    {2200, 50}, {0, 50},
    {2000, 120}
  };

  // ----------------- MOODS -----------------
  // HAPPY: upward / bright
  static inline const BeepStep SFX_HAPPY_SOFT[5] = {
    {900, 40}, {0, 25}, {1200, 45}, {0, 25}, {1500, 60}
  };
  static inline const BeepStep SFX_HAPPY_SCIFI[7] = {
    {1600, 25}, {0, 20},
    {1900, 25}, {0, 20},
    {2300, 30}, {0, 35},
    {2600, 65}
  };
  static inline const BeepStep SFX_HAPPY_MINI[3] = {
    {1800, 22}, {0, 18}, {2200, 35}
  };

  // ANGRY: harsh downward
  static inline const BeepStep SFX_ANGRY_SOFT[5] = {
    {600, 55}, {0, 25}, {520, 55}, {0, 25}, {440, 90}
  };
  static inline const BeepStep SFX_ANGRY_SCIFI[7] = {
    {900, 35}, {0, 20},
    {850, 35}, {0, 20},
    {780, 45}, {0, 20},
    {650, 110}
  };
  static inline const BeepStep SFX_ANGRY_MINI[3] = {
    {900, 35}, {0, 18}, {700, 70}
  };

  // TIRED: slow, low, spaced
  static inline const BeepStep SFX_TIRED_SOFT[5] = {
    {500, 70}, {0, 120}, {420, 70}, {0, 140}, {360, 120}
  };
  static inline const BeepStep SFX_TIRED_SCIFI[7] = {
    {700, 60}, {0, 120},
    {640, 60}, {0, 140},
    {580, 60}, {0, 160},
    {520, 140}
  };
  static inline const BeepStep SFX_TIRED_MINI[3] = {
    {700, 55}, {0, 140}, {520, 110}
  };

  // COLD: “shiver” ticks then a low note
  static inline const BeepStep SFX_COLD_SOFT[7] = {
    {1200, 25}, {0, 45},
    {1200, 25}, {0, 60},
    {950, 45},  {0, 70},
    {700, 120}
  };
  static inline const BeepStep SFX_COLD_SCIFI[9] = {
    {2000, 18}, {0, 35},
    {2000, 18}, {0, 45},
    {1700, 22}, {0, 55},
    {1400, 28}, {0, 60},
    {900, 120}
  };
  static inline const BeepStep SFX_COLD_MINI[5] = {
    {1800, 18}, {0, 40}, {1800, 18}, {0, 55}, {900, 90}
  };

  // ----------------- LENGTH HELPER -----------------
  template <size_t kLen>
  static constexpr uint8_t LEN(const BeepStep (&)[kLen]) { return (uint8_t)kLen; }

  // ----------------- PICKERS -----------------
  Seq seqTap(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_TAP_SOFT,  LEN(SFX_TAP_SOFT) };
      case SOUND_MINIMAL: return { SFX_TAP_MINI,  LEN(SFX_TAP_MINI) };
      case SOUND_SCIFI:
      default:            return { SFX_TAP_SCIFI, LEN(SFX_TAP_SCIFI) };
    }
  }

  Seq seqLongHold(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_HOLD_SOFT,  LEN(SFX_HOLD_SOFT) };
      case SOUND_MINIMAL: return { SFX_HOLD_MINI,  LEN(SFX_HOLD_MINI) };
      case SOUND_SCIFI:
      default:            return { SFX_HOLD_SCIFI, LEN(SFX_HOLD_SCIFI) };
    }
  }

  Seq seqRelease(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_REL_SOFT,  LEN(SFX_REL_SOFT) };
      case SOUND_MINIMAL: return { SFX_REL_MINI,  LEN(SFX_REL_MINI) };
      case SOUND_SCIFI:
      default:            return { SFX_REL_SCIFI, LEN(SFX_REL_SCIFI) };
    }
  }

  Seq seqHello(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_HELLO_SOFT,  LEN(SFX_HELLO_SOFT) };
      case SOUND_MINIMAL: return { SFX_HELLO_MINI,  LEN(SFX_HELLO_MINI) };
      case SOUND_SCIFI:
      default:            return { SFX_HELLO_SCIFI, LEN(SFX_HELLO_SCIFI) };
    }
  }

  Seq seqIntro(Style s) const {
    (void)s; // same intro for all styles (easy to branch later)
    return { SFX_INTRO, LEN(SFX_INTRO) };
  }

  Seq seqHappy(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_HAPPY_SOFT,  LEN(SFX_HAPPY_SOFT) };
      case SOUND_MINIMAL: return { SFX_HAPPY_MINI,  LEN(SFX_HAPPY_MINI) };
      case SOUND_SCIFI:
      default:            return { SFX_HAPPY_SCIFI, LEN(SFX_HAPPY_SCIFI) };
    }
  }

  Seq seqAngry(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_ANGRY_SOFT,  LEN(SFX_ANGRY_SOFT) };
      case SOUND_MINIMAL: return { SFX_ANGRY_MINI,  LEN(SFX_ANGRY_MINI) };
      case SOUND_SCIFI:
      default:            return { SFX_ANGRY_SCIFI, LEN(SFX_ANGRY_SCIFI) };
    }
  }

  Seq seqTired(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_TIRED_SOFT,  LEN(SFX_TIRED_SOFT) };
      case SOUND_MINIMAL: return { SFX_TIRED_MINI,  LEN(SFX_TIRED_MINI) };
      case SOUND_SCIFI:
      default:            return { SFX_TIRED_SCIFI, LEN(SFX_TIRED_SCIFI) };
    }
  }

  Seq seqCold(Style s) const {
    switch (s) {
      case SOUND_SOFT:    return { SFX_COLD_SOFT,  LEN(SFX_COLD_SOFT) };
      case SOUND_MINIMAL: return { SFX_COLD_MINI,  LEN(SFX_COLD_MINI) };
      case SOUND_SCIFI:
      default:            return { SFX_COLD_SCIFI, LEN(SFX_COLD_SCIFI) };
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
