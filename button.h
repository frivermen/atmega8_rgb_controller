_Bool button_state = 0;
_Bool button_flag = 0;
uint32_t button_timer = 0;


void button() {
  button_state = !digitalRead(BUT0);

  if (button_state && !button_flag && millis() - button_timer > 50) { // short press
    button_flag = 1;
    
    if (enabled) {
      system_off();
      on_off_speed = 255;
    }

    else {
      system_on();
    }

    button_timer = millis();
  }

  if (button_state && button_flag && millis() - button_timer > 500) { // long press
    on_off_speed = 0;
    enabled = 0;

    button_timer = millis();
  }

  if (!button_state && button_flag && millis() - button_timer > 50) { // release
    button_flag = 0;

    button_timer = millis();
  }
}
