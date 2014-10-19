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

  http://ledlogic.blogspot.com/2014/10/arduino-8x8-shields.html
 ****************************************************/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(0));

  matrix.begin(0x70);  // pass in the address
}

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
;

void eyesDraw(const uint8_t *bitmap) {
  matrix.clear();
  matrix.drawBitmap(0, 0, bitmap, 8, 8, LED_ON);
  matrix.writeDisplay();
  eyesWait();
}

void eyesWait() {
  delay(random(250, 750));
}

void eyesUp() {
  eyesDraw(eye_off);
  eyesDraw(eye_up_left);
  eyesDraw(eye_up_center);
  eyesDraw(eye_up_right);
  eyesDraw(eye_off);
}

void eyesMiddle() {
  eyesDraw(eye_off);
  eyesDraw(eye_left);
  eyesDraw(eye_center);
  eyesDraw(eye_right);
  eyesDraw(eye_center);
  eyesDraw(eye_left);
  eyesDraw(eye_off);
}

void eyesDown() {
  eyesDraw(eye_off);
  eyesDraw(eye_down_left);
  eyesDraw(eye_down_center);
  eyesDraw(eye_down_right);
  eyesDraw(eye_off);
}

void eyesClock() {
  eyesDraw(eye_off);
  eyesDraw(eye_up_center);
  eyesDraw(eye_up_right);
  eyesDraw(eye_right);
  eyesDraw(eye_down_right);
  eyesDraw(eye_down_center);
  eyesDraw(eye_down_left);
  eyesDraw(eye_left);
  eyesDraw(eye_up_left);
  eyesDraw(eye_up_center);
  eyesDraw(eye_off);
}


void loop() {
  int choice = random(4);
  switch (choice) {
    case 0:
      eyesUp();
      break;
    case 1:
      eyesMiddle();
      break;
    case 2:
      eyesDown();
      break;
    case 3:
      eyesClock();
      break;
    default:
      ;
  }
}
