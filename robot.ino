#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "FluxGarage_RoboEyes.h"
#include "RobotSound.h"

#define TOUCH_PIN 2
#define BUZZER_PIN 20   // <-- change to your buzzer pin

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

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  pinMode(TOUCH_PIN, INPUT);

  sound.begin();
  sound.setDefaultStyle(RobotSound::SOUND_SOFT);

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.setCuriosity(ON);
}

void loop() {
  // INPUT_PULLUP: touched/pressed is typically LOW
  bool touched = (digitalRead(TOUCH_PIN) == HIGH);

  // ---- sound events ----
  if (touched && !lastTouched) {
    // just pressed
    pressStartMs = millis();
    holdPlayed = false;
    sound.tap();
    roboEyes.setIdleMode(OFF);
    roboEyes.setPosition(DEFAULT);
  }

  if (touched && !holdPlayed && (millis() - pressStartMs >= HOLD_MS)) {
    // long hold reached (once)
    holdPlayed = true;
    sound.longHold();
    roboEyes.anim_laugh();
  }

  if (!touched && lastTouched) {
    // released
    //sound.release();
    roboEyes.setIdleMode(ON,2,2);
  }

  lastTouched = touched;

  // ---- eyes mood ----
  roboEyes.setMood(touched ? HAPPY : DEFAULT);

  roboEyes.update();
  sound.update();   // keep beeps progressing (non-blocking)
}
