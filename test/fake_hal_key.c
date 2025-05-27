#include <stdbool.h>
#include "hal_key.h"

static bool pressed_state = false;

void hal_key_init(void) {

}

bool hal_key_pressed(void) {
  return pressed_state;
}

void set_hal_key_pressed(bool v) {
  pressed_state = v;
}
