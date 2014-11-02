/***************************************************
  This is a library for our I2C LED Backpacks

  Designed specifically to work with the Adafruit LED Matrix backpacks
  ----> http://www.adafruit.com/products/872
  ----> http://www.adafruit.com/products/871
  ----> http://www.adafruit.com/products/870

  These displays use I2C to communicate, 2 pins are required to
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution

  Modified 2014 by Jeff D. Conrad as skulleyes project.
  * Controls two 8x8 eyes.
  * Plays sounds (commented out).
  * Might need to remove functionality from sound library to work - mp3 etc.

  http://ledlogic.blogspot.com/2014/10/arduino-8x8-shields.html
 ****************************************************/

#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <SkulleyesMusicPlayer.h>
#include <Skulleyes_LEDBackpack.h>
#include <Skulleyes_GFX.h>

Skulleyes_8x8matrix matrix = Skulleyes_8x8matrix();

const boolean eyeDbg = false;
boolean talkMode = false;
boolean talkPlayed = false;
int talkModeChecked = 0;

int lastTalkChoice = -1;
int lastTalk = -1;

const int wait_eyes = 128;
const int wait_events = 1024;
int wait_events_multiplier = 5;
int wait_talk = -1;

static const uint8_t PROGMEM
eye_off[] =
{ B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
},
eye_up_left[] =
{ B00111100,
  B01111110,
  B10011111,
  B10011111,
  B11111111,
  B11111111,
  B01111110,
  B00111100
},
eye_up_center[] =
{ B00111100,
  B01100110,
  B11100111,
  B11111111,
  B11111111,
  B11111111,
  B01111110,
  B00111100
},
eye_up_right[] =
{ B00111100,
  B01111110,
  B11111001,
  B11111001,
  B11111111,
  B11111111,
  B01111110,
  B00111100
},
eye_left[] =
{ B00111100,
  B01111110,
  B11111111,
  B10011111,
  B10011111,
  B11111111,
  B01111110,
  B00111100
},
eye_center[] =
{ B00111100,
  B01111110,
  B11111111,
  B11100111,
  B11100111,
  B11111111,
  B01111110,
  B00111100
},
eye_right[] =
{ B00111100,
  B01111110,
  B11111111,
  B11111001,
  B11111001,
  B11111111,
  B01111110,
  B00111100
},
eye_down_left[] =
{ B00111100,
  B01111110,
  B11111111,
  B11111111,
  B10011111,
  B10011111,
  B01111110,
  B00111100
},
eye_down_center[] =
{ B00111100,
  B01111110,
  B11111111,
  B11111111,
  B11111111,
  B11100111,
  B01100110,
  B00111100
},
eye_down_right[] =
{ B00111100,
  B01111110,
  B11111111,
  B11111111,
  B11111001,
  B11111001,
  B01111110,
  B00111100
};

static char* soundFiles[] = {
  "BOOLEAN.mp3",
  "DYLNSCRY.mp3",
  "GHOSTMCH.mp3",
  "HASKULL.mp3",
  "ICDEADCD.mp3",

  "MNSTERHM.mp3",
  "NOBODY.mp3",
  "PUMPKNPI.mp3",
  "SPIRITS.mp3",
  "TOBNOTB.mp3",

  "TRVRSCRY.mp3",
  "UNLBYTES.mp3",
  "WWW.mp3",
  "TRVRSCRY.mp3"
};

void setup() {
  Serial.begin(9600);
  player.begin();
  player.setVolume(80);
  randomSeed(analogRead(0));
  matrix.begin(0x70);
}

void eyesOff(void) {
  eyesDraw(eye_off, 0, 0);
}

void eyesDrawDelayOff(void) {
  eyesDrawDelay(eye_off);
}

void eyesWait(int minWait, int maxWait) {
  delay(random(minWait, maxWait));
}

void eyesDraw(const uint8_t *bitmap, int minWait, int maxWait) {
  matrix.clear();
  matrix.drawBitmap(0, 0, bitmap, 8, 8, LED_ON);
  matrix.writeDisplay();
  
  if (maxWait > 0) {
    eyesWait(minWait, maxWait);
  }
}

void eyesDrawDelay(const uint8_t *bitmap) {
  eyesDraw(bitmap, wait_eyes, wait_eyes);
}

void eyesChoiceUp() {
  eyesDrawDelayOff();
  eyesDrawDelay(eye_up_left);
  eyesDrawDelay(eye_up_center);
  eyesDrawDelay(eye_up_right);
  eyesDrawDelay(eye_up_center);
  eyesDrawDelay(eye_up_left);
  eyesDrawDelayOff();
}

void eyesChoiceMiddle() {
  eyesDrawDelayOff();
  eyesDrawDelay(eye_left);
  eyesDrawDelay(eye_center);
  eyesDrawDelay(eye_right);
  eyesDrawDelay(eye_center);
  eyesDrawDelay(eye_left);
  eyesDrawDelayOff();
}

void eyesChoiceDown() {
  eyesDrawDelayOff();
  eyesDrawDelay(eye_down_left);
  eyesDrawDelay(eye_down_center);
  eyesDrawDelay(eye_down_right);
  eyesDrawDelay(eye_down_center);
  eyesDrawDelay(eye_down_left);
  eyesDrawDelayOff();
}

void eyesChoiceClock() {
  eyesDrawDelayOff();
  eyesDrawDelay(eye_up_center);
  eyesDrawDelay(eye_up_right);
  eyesDrawDelay(eye_right);
  eyesDrawDelay(eye_down_right);
  eyesDrawDelay(eye_down_center);
  eyesDrawDelay(eye_down_left);
  eyesDrawDelay(eye_left);
  eyesDrawDelay(eye_up_left);
  eyesDrawDelay(eye_up_center);
  eyesDrawDelayOff();
}

void eyesChoiceTalk() {
  int talkChoice = -1;
  while (talkChoice == -1 || talkChoice == lastTalkChoice) {
    talkChoice = random(13);
  }
  lastTalkChoice = talkChoice;
  char* fn = soundFiles[talkChoice];
  if (eyeDbg) Serial.println("Playing talkChoice[" + String(talkChoice) + "], fn[" + String(fn) + "]");
  talkMode = true;
  talkPlayed = false;
  talkModeChecked = 0;
  wait_talk = 0;
  player.playOne(fn);
}

void eyesTalkStep() {
  if (eyeDbg) Serial.println("playing");
  wait_talk--;
  if (wait_talk < 1) {
    int eyeSize = random(1000);
    if (eyeSize > 0) {
      eyeSize = 1;
    }
    if (lastTalk != eyeSize) {
      if (eyeSize == 0) {
        eyesOff();
        wait_talk = random(2 * wait_eyes, 6 * wait_eyes);
      } else {
        eyesDraw(eye_center, 0, 0);
        wait_talk = random(4 * wait_eyes, 10 * wait_eyes);
      }
      lastTalk = eyeSize;
    }
  }

  talkPlayed = true;
  player.play();
}

void eyesTalkCheck() {
  String state = player.getPlayingState();
    if (eyeDbg) Serial.println("state[" + state + "], talkPlayed[" + talkPlayed + "]");

    if (talkPlayed && state != "1" && state != "3") {
      talkModeChecked++;

      eyesOff();

      if (talkModeChecked > 20) {
        if (eyeDbg) Serial.println("was playing but do not think so now, state[" + state + "]");
        talkMode = false;
        eyesWait(wait_events, wait_events);
      }

    } else {
      eyesTalkStep();
    }
}

void eyesRandom() {
  int eyeChoice = random(5);
  switch (eyeChoice) {
    case 0:
      eyesChoiceUp();
      break;
    case 1:
      eyesChoiceMiddle();
      break;
    case 2:
      eyesChoiceDown();
      break;
    case 3:
      eyesChoiceClock();
      break;
    case 4:
      eyesChoiceTalk();
      break;
  }
}

void loop() {
  // if talking, keep talking
  if (talkMode) {
    eyesTalkCheck();
  } else {
    eyesWait(wait_events, wait_events_multiplier * wait_events);
    eyesRandom();
  }
}
