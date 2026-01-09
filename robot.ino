#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <math.h>

#include "FluxGarage_RoboEyes.h"
#undef N
#include "RobotSound.h"

#define TOUCH_PIN 2
#define BUZZER_PIN 20  
#define THERM_PIN 1          

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RoboEyes<Adafruit_SSD1306> roboEyes(display);

RobotSound sound(BUZZER_PIN, RobotSound::SOUND_SCIFI);

// press/hold detection
bool lastTouched = false;
bool holdPlayed = false;
unsigned long pressStartMs = 0;
const unsigned long HOLD_MS = 800;

// ---- Cold behavior ----
const float COLD_ON_C  = 18.0f; 
const float COLD_OFF_C = 18.5f; 
const unsigned long COLD_CONFIRM_MS = 2000;
bool coldActive = false;
unsigned long coldBelowSince = 0;
unsigned long lastColdBeepMs = 0;
static const float R_FIXED = 10000.0f;
static const float BETA    = 2310.0f; 
static const float R0      = 13400.0f;
static const float T0K     = 25.0f + 273.15f;
static const float ADC_MAX = 4095.0f;
unsigned long nextColdSoundMs = 0;

// ---- hello scheduling ----
const unsigned long HELLO_PERIOD_MS = 15UL * 60UL * 1000UL; // 15 minutes
unsigned long nextHelloMs = 0;


// ---- thermistor read ----
float readRntc(int adc) {
  if (adc < 1) adc = 1;
  if (adc > (int)ADC_MAX - 1) adc = (int)ADC_MAX - 1;

  float x = (float)adc / ADC_MAX;            
  return R_FIXED * (1.0f / x - 1.0f);
}

float rToTempC(float r) {
  float invT = (1.0f / T0K) + (1.0f / BETA) * logf(r / R0);
  float tK = 1.0f / invT;
  return tK - 273.15f;
}

void enterColdMode() {
  coldActive = true;
  roboEyes.setIdleMode(OFF);
  roboEyes.setMood(TIRED);
  roboEyes.setPosition(DEFAULT);

  roboEyes.setHFlicker(ON, 2); 
  roboEyes.setVFlicker(OFF);

  // optional: one-time cold sound
  sound.cold(RobotSound::SOUND_SOFT);

  Serial.println("[COLD] entered");
}

void exitColdMode() {
  coldActive = false;
  coldBelowSince = 0;

  roboEyes.setHFlicker(OFF);
  roboEyes.setVFlicker(OFF);

  roboEyes.setMood(DEFAULT);
  roboEyes.setIdleMode(ON, 2, 2);

  Serial.println("[COLD] exited");
}


void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  randomSeed(micros());
  nextHelloMs = millis() + HELLO_PERIOD_MS;


  pinMode(TOUCH_PIN, INPUT);

  analogReadResolution(12);

  sound.begin();
  sound.setDefaultStyle(RobotSound::SOUND_SOFT);

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.setCuriosity(ON);

    
}

void loop() {
  unsigned long now = millis();

  float tC = rToTempC(readRntc(analogRead(THERM_PIN)));
  Serial.print("TempC: "); Serial.println(tC);

  // update coldActive (with confirm + hysteresis)
  if (!coldActive) {
    if (tC < COLD_ON_C) {
      if (coldBelowSince == 0) coldBelowSince = now;
      if (now - coldBelowSince >= COLD_CONFIRM_MS) {
        enterColdMode();
      }
    } else {
      coldBelowSince = 0;
    }
  } else {
    if (tC >= COLD_OFF_C) {
      exitColdMode();
    }
  }

  // ---- touch logic (ONLY when not cold) ----
  bool touched = (digitalRead(TOUCH_PIN) == HIGH);

  if (!coldActive) {
    //Just touched
    if (touched && !lastTouched) {
      pressStartMs = now;
      holdPlayed = false;

      sound.tap();
      roboEyes.setMood(HAPPY);
      roboEyes.setIdleMode(OFF);
      roboEyes.setPosition(DEFAULT);
    }
    //Holding touch
    if (touched && !holdPlayed && (now - pressStartMs >= HOLD_MS)) {
      holdPlayed = true;
      sound.longHold();
      roboEyes.anim_laugh();
    }
    //Release touch
    if (!touched && lastTouched) {
      roboEyes.setIdleMode(ON, 2, 2);
      roboEyes.setMood(DEFAULT);
    }

    // keep mood synced
    roboEyes.setMood(touched ? HAPPY : DEFAULT);
  } else {
    // cold overrides everything
    roboEyes.setMood(TIRED);
  }

  lastTouched = touched;

  // ----- HELLO every 15 minutes -----
  if ((long)(now - nextHelloMs) >= 0) {
    // optional rule: only say hello if NOT being touched and not cold
    if (!touched && !coldActive){
      if (!sound.isPlaying()) {
        sound.hello(RobotSound::SOUND_SOFT);   // choose the vibe you want
      }
      nextHelloMs = now + HELLO_PERIOD_MS;
    }
  }

  // ----- COLD sound every 5-10 seconds while coldActive -----
  if (coldActive) {
    if (nextColdSoundMs == 0) {
      nextColdSoundMs = now + (unsigned long)random(5000, 10001); // first schedule
    }

    if ((long)(now - nextColdSoundMs) >= 0) {
      if (!sound.isPlaying()) {
        sound.cold(RobotSound::SOUND_SCIFI); // or SOUND_SOFT if you prefer
      }
      nextColdSoundMs = now + (unsigned long)random(5000, 10001);
    }
  } else {
    nextColdSoundMs = 0; // reset schedule when not cold
  }

  roboEyes.update();
  sound.update();
}
