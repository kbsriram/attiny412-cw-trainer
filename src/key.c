#include <stdint.h>

#include "hal_key.h"
#include "key.h"

#define DEBOUNCE_WAIT_TICKS 2
#define LONG_PRESS_TICKS 1000

static uint8_t debounce_ticks = 0;

// raw state of the key.
static bool raw_pressed = false;

// debounced state of the key.
static bool debounced_pressed = false;

// counter for a long press.
static uint16_t pressed_ticks = 0;

void key_init(void) {
  hal_key_init();
}

key_state_t key_update(void) {
  // Update pressed_ticks if key is currently pressed.
  if (debounced_pressed && (pressed_ticks < LONG_PRESS_TICKS)) {
    pressed_ticks++;
  }

  // Read the current state of our key.
  bool is_pressed = hal_key_pressed();

  if (is_pressed != raw_pressed) {
    // Change in raw key state - start up the debounce counter.
    raw_pressed = is_pressed;
    debounce_ticks = DEBOUNCE_WAIT_TICKS;
    return KEY_NO_CHANGE;
  }

  if (debounce_ticks) {
    // We're debouncing. Decrement countdown until we're at 0.
    debounce_ticks--;
    if (debounce_ticks) {
      // not yet done.
      return KEY_NO_CHANGE;
    }
    // countdown completed.
    debounced_pressed = raw_pressed;
    if (debounced_pressed) {
      pressed_ticks = 0;
      return KEY_DOWN;
    } else {
      if (pressed_ticks >= LONG_PRESS_TICKS) {
        return KEY_UP_LONG;
      } else {
        return KEY_UP;
      }
    }
  }

  return KEY_NO_CHANGE;
}
