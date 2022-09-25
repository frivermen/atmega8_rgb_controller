// ir nec
volatile uint32_t ir_buffer = 0;
volatile uint32_t ir_timer = 0;
volatile uint8_t ir_counter = 0;
volatile uint8_t ir_command = 0;
volatile _Bool ir_start = 0;
volatile _Bool ir_available = 0;

void ir() {
  uint32_t time = micros() - ir_timer;
  ir_timer += time;

  if (ir_start && time < 2400) {

    if (time > 1000 && time < 1300) { // low bit
      ir_buffer <<= 1;
      ir_counter++;
    }

    if (time > 2100 && time < 2400) { // high bit
      ir_buffer <<= 1;
      ir_buffer |= 1;
      ir_counter++;
    }

    if (ir_counter == 32) {
      if ((~ir_buffer & 0xFF) == ((ir_buffer & 0xFF00) >> 8)) {
        ir_command = ir_buffer & 0xFF;
        ir_available = 1;
      }
      ir_start = 0;
    }
  }

  if (time > 12900 && time < 15900) {
    ir_counter = 0;
    ir_buffer = 0;
    ir_start = 1;
  }
}
