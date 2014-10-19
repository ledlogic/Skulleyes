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
 ****************************************************/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

void setup() {
  Serial.begin(9600);
  Serial.println("8x8 LED Matrix Test");
  
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
    B00000000 },
  eye_up_left[] =
  { B00111100,
    B01111110,
    B10011111,
    B10011111,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
  eye_up_center[] =
  { B00111100,
    B01100110,
    B11100111,
    B11111111,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
  eye_up_right[] =
  { B00111100,
    B01111110,
    B11111001,
    B11111001,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
   eye_left[] =
  { B00111100,
    B01111110,
    B11111111,
    B10011111,
    B10011111,
    B11111111,
    B01111110,
    B00111100 },
  eye_center[] =
  { B00111100,
    B01111110,
    B11111111,
    B11100111,
    B11100111,
    B11111111,
    B01111110,
    B00111100 },
  eye_right[] =
  { B00111100,
    B01111110,
    B11111111,
    B11111001,
    B11111001,
    B11111111,
    B01111110,
    B00111100 },
  eye_down_left[] =
  { B00111100,
    B01111110,
    B11111111,
    B11111111,
    B10011111,
    B10011111,
    B01111110,
    B00111100 },
  eye_down_center[] =
  { B00111100,
    B01111110,
    B11111111,
    B11111111,
    B11111111,
    B11100111,
    B01100110,
    B00111100 },
  eye_down_right[] =
  { B00111100,
    B01111110,
    B11111111,
    B11111111,
    B11111001,
    B11111001,
    B01111110,
    B00111100 };
;

void draw(const uint8_t *bitmap) {
  matrix.clear();
  matrix.drawBitmap(0, 0, bitmap, 8, 8, LED_ON);
  matrix.writeDisplay();
  delay(1000);
}

void loop() {
  draw(eye_off);
  draw(eye_up_left);
  draw(eye_up_center);
  draw(eye_up_right);
  draw(eye_off);
  draw(eye_left);
  draw(eye_center);
  draw(eye_right);
  draw(eye_off);
  draw(eye_down_left);
  draw(eye_down_center);
  draw(eye_down_right);
}
