#include <avr/io.h>
#include <stdbool.h>

#include "hal_key.h"

void hal_key_init(void) {
  // Pin configured as input with pullup enabled.
  PORTA.DIRCLR = PIN2_bm;
  PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
}

bool hal_key_pressed(void) {
  return !(PORTA.IN & PIN2_bm);
}
