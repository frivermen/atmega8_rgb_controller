uint16_t r_seed = RANDOM_SEED;

uint16_t rget() {
  r_seed = (r_seed * 2053ul) + 13849;
  return r_seed;
}

uint16_t rget(uint16_t max) {
  return ((uint32_t)max * rget()) >> 16;
}

uint16_t rget(uint16_t min, uint16_t max) {
  return (rget(max - min) + min);
}
