// rgb
uint8_t rgb[3] = {0, 0, 0};
uint8_t buf_color[3] = {0, 0, 0};
uint8_t i = 0;
uint8_t mask = 0b100;
uint8_t brightness = BRIGHNESS;
uint8_t speed = SPEED;
uint32_t speed_counter = 0;

// fire
_Bool fire_direction = 1;
uint8_t fire_brightness = FIRE_MIN;
uint8_t new_max = FIRE_MAX;
uint8_t new_min = FIRE_MIN;



// crt correction
uint8_t crt(uint8_t x) {
  return (x * x + 0xFF) >> 8;
}

void rgb_write() {
  analogWrite(RED, crt((rgb[0] * brightness) >> 8));
  analogWrite(BLUE, crt((rgb[1] * brightness) >> 8));
  analogWrite(GREEN, crt((rgb[2] * brightness) >> 8));
}

void rgb_off() {
  analogWrite(RED, 0);
  analogWrite(BLUE, 0);
  analogWrite(GREEN, 0);
}

void rgb_fade() {
  if (millis() - speed_counter > speed) {
    speed_counter = millis();
    rgb[(mask >> 1) & 0b11] = i;
    rgb[(mask >> 2) & 0b11] = 255 - i;

    if (++i == 255) {
      i = 0;
      mask |= mask << 3;
      mask = mask >> 2;
    }
  }
}

void rgb_fire() {
  if (millis() - speed_counter > speed) {
    speed_counter = millis();

    rgb[0] = (255 * fire_brightness) >> 8;
    rgb[1] = (255 * fire_brightness) >> 8;
    rgb[2] = (0 * fire_brightness) >> 8;

    if (fire_direction) {
      if (++fire_brightness > new_max) {
        fire_direction = 0;
        new_max = rget(new_min, FIRE_MAX);
      }
    }
    else {
      if (--fire_brightness < new_min) {
        fire_direction = 1;
        new_min = rget(FIRE_MIN, new_max);
      }
    }
  }
}

void set_color(uint8_t red, uint8_t green, uint8_t blue) {
  rgb[0] = red;
  rgb[1] = green;
  rgb[2] = blue;
  fade_mode = 0;
  fire_mode = 0;
}
