#include <avr/io.h>
#include <stdbool.h>

#include "hal_key.h"

void hal_key_init(void) {
  // Pin configured as input with pullup enabled.
  PORTA.DIRCLR = PIN6_bm;
  PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
}

bool hal_key_pressed(void) {
  return !(PORTA.IN & PIN6_bm);
}
