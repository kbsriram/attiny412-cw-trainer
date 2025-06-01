#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "key.h"
#include "state.h"
#include "ticks.h"
#include "tone.h"

// (1) Vdd
// (2) PA6 - KEY
// (3) PA7 - NC
// (4) PA1 - NC
// (5) PA2 - NC
// (6) PA0 - UPDI
// (7) PA3 - SPKR
// (8) GND

void setup(void) {
  ticks_init();
  tone_init();
  key_init();
}

int main(void) {
  setup();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  state_reset();
  sei();

  while (1) {
    sleep_mode();
    // Woken up by the PIT
    state_tick();
  }
}
