#include <avr/eeprom.h>

// pins
#define IR      2
#define RED     9
#define GREEN   10
#define BLUE    11
#define BUT0    0
#define BUT1    1

// blink time
#define OVERFLOW_DELAY 150

// fire effect
#define FIRE_MAX 254
#define FIRE_MIN 100
#define FIRE_GAP 50

// rgb settings
#define BRIGHNESS 255 // from 0 to 255
#define SPEED 255 // from 0 to 255

// random settings
#define RANDOM_SEED 9 // uint16_t

// on/off speed settings
#define ON_SPEED 4 // 0..255
#define OFF_SPEED 0 // 0..255



void command_parser();
void system_toggle();
void system_on();
void system_off();

_Bool enabled = 1;
_Bool fade_mode = 0; // 1 - fade, 0 - static color
_Bool fire_mode = 1; // 1 - fire, 0 - static color
uint32_t last_brightness_timer = 0;
uint32_t smooth_brightness_timer = 0;
uint8_t save_brightness = 0;
uint8_t on_off_speed = 0;
uint32_t on_off_timer = 0;
_Bool save_brightness_flag = 1;

#include "ir.h"
#include "random.h"
#include "rgb.h"
#include "button.h"

void setup() {
  fade_mode = eeprom_read_byte((uint8_t*)0);
  fire_mode = eeprom_read_byte((uint8_t*)1);
  rgb[0] =    eeprom_read_byte((uint8_t*)2);
  rgb[1] =    eeprom_read_byte((uint8_t*)3);
  rgb[2] =    eeprom_read_byte((uint8_t*)4);
  brightness = eeprom_read_byte((uint8_t*)5);
  speed =     eeprom_read_byte((uint8_t*)6);

  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(GREEN, OUTPUT);

  pinMode(IR, INPUT_PULLUP);

  //#define DEBUG
#ifdef DEBUG
  Serial.begin(115200);
#else
  pinMode(BUT0, INPUT_PULLUP);
  pinMode(BUT1, OUTPUT);
  bitClear(PORTD, BUT1); // set LOW
#endif

  attachInterrupt(0, ir, FALLING);
}

void loop() {
  button();

  if (ir_available) {
    command_parser();
    ir_command = 0;
    ir_available = 0;
  }



  if (fade_mode) {
    rgb_fade();
  }
  else if (fire_mode) {
    rgb_fire();
  }



  if (enabled) {
    if (brightness < save_brightness) {
      if (millis() - smooth_brightness_timer > on_off_speed) {
        smooth_brightness_timer = millis();
        brightness++;
      }
    }
    else {
      save_brightness_flag = 1;
      save_brightness = 0;
    }
    rgb_write();
  }



  if (!enabled) {
    if (save_brightness_flag) {
      save_brightness = brightness;
      save_brightness_flag = 0;
    }
    if (brightness > 0) {
      if (millis() - smooth_brightness_timer > on_off_speed) {
        smooth_brightness_timer = millis();
        brightness--;
      }
      rgb_write();
    }
    else {
      rgb_off();
    }
  }
}



void system_toggle() {
  if (fade_mode) {
    rgb[0] = buf_color[0];
    rgb[1] = buf_color[1];
    rgb[2] = buf_color[2];
  }
  else {
    buf_color[0] = rgb[0];
    buf_color[1] = rgb[1];
    buf_color[2] = rgb[2];
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
  }
  fade_mode = !fade_mode;
}

void speed_up() {
  if (speed == 0) speed = 1;
  else {
    if (speed < 129) {
      speed <<= 1;
      speed |= 1;
    }
    else {
      rgb_off();
      delay(OVERFLOW_DELAY);
    }
  }
}

void speed_down() {
  if (speed > 0) {
    speed >>= 1;
  }
  else {
    rgb_off();
    delay(OVERFLOW_DELAY);
  }
}

void brightness_up() {
  if (millis() - last_brightness_timer > 500) {
    if (brightness < 255 - 10) {
      brightness += 10;
    }
    else {
      brightness = 255;
      rgb_off();
      delay(OVERFLOW_DELAY);
    }
  }
  else {
    if (brightness < 255 - 50) {
      brightness += 50;
    }
    else {
      brightness = 255;
      rgb_off();
      delay(OVERFLOW_DELAY);
    }
  }
  last_brightness_timer = millis();
}

void brightness_down() {
  if (millis() - last_brightness_timer > 500) {
    if (brightness > 0 + 10) {
      brightness -= 10;
    }
    else {
      brightness = 5;
      rgb_off();
      delay(OVERFLOW_DELAY);
    }
  }
  else {
    if (brightness > 0 + 50) {
      brightness -= 50;
    }
    else {
      brightness = 5;
      rgb_off();
      delay(OVERFLOW_DELAY);
    }
  }
  last_brightness_timer = millis();
}

void system_on() {
  enabled = 1;
  on_off_speed = ON_SPEED;
}

void system_off() {
  if (enabled) {
    enabled = 0;
    eeprom_update_byte((uint8_t*)0, fade_mode);
    eeprom_update_byte((uint8_t*)1, fire_mode);
    eeprom_update_byte((uint8_t*)2, rgb[0]);
    eeprom_update_byte((uint8_t*)3, rgb[1]);
    eeprom_update_byte((uint8_t*)4, rgb[2]);
    eeprom_update_byte((uint8_t*)5, brightness);
    eeprom_update_byte((uint8_t*)6, speed);
  }
  if (millis() - on_off_timer > 500) {
    on_off_speed = 255;
  }
  else {
    on_off_speed = OFF_SPEED;
  }
  on_off_timer = millis();
}

void command_parser() {
  switch (ir_command) {
    case 0xFD:                                  // ok
      system_toggle();
      break;

    case 0x97:                                  // 1
      set_color(255, 0, 0);
      break;

    case 0x67:                                  // 2
      set_color(0, 255, 0);
      break;

    case 0x4F:                                  // 3
      set_color(0, 0, 255);
      break;

    case 0xCF:                                  // 4
      set_color(255, 255, 0);
      break;

    case 0xE7:                                  // 5
      set_color(0, 255, 255);
      break;

    case 0x85:                                  // 6
      set_color(255, 0, 255);
      break;

    case 0xEF:                                  // 7
      set_color(100, 0, 255);
      break;

    case 0xC7:                                  // 8
      set_color(255, 0, 100);
      break;

    case 0xA5:                                  // 9
      set_color(0, 255, 100);
      break;

    case 0xB5:                                  // 0
      fade_mode = 0;
      fire_mode = 1;
      break;

    case 0x3D:                                  // right
      speed_up();
      break;

    case 0xDD:                                   // left
      speed_down();
      break;

    case 0x9D:                                   // up
      brightness_up();
      break;

    case 0x57:                                   // down
      brightness_down();
      break;

    case 0xAD:                                  // sharh
      system_on();
      break;

    case 0xBD:                                  // star
      system_off();
      break;
  }
}
