#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "key.h"

extern void set_hal_key_pressed(bool v);

static void verify_state(int count, key_state_t state) {
  for (int i = 0; i < count; i++) {
    assert(key_tick() == state);
  }
}

void test_debounce(int nbounce) {
  printf("Test: key_debounce: %d bounces\n", nbounce);

  set_hal_key_pressed(false);
  // Have 10 updates with pressed false - we should
  // see no change.
  verify_state(10, KEY_NO_CHANGE);

  // Bounce the key as many times as requested.
  for (int i = 0; i < nbounce; i++) {
    set_hal_key_pressed(true);
    // We shouldn't see a key change yet.
    assert(key_tick() == KEY_NO_CHANGE);
    set_hal_key_pressed(false);
    assert(key_tick() == KEY_NO_CHANGE);
  }

  // Now let the key settle at high.
  set_hal_key_pressed(true);

  // Wait two ticks to settle.
  verify_state(2, KEY_NO_CHANGE);

  // Next update should be a keydown.
  assert(key_tick() == KEY_DOWN);

  // 10 more updates with the key down, should have no change.
  verify_state(10, KEY_NO_CHANGE);

  // Bounce for the key down
  for (int i = 0; i < nbounce; i++) {
    set_hal_key_pressed(false);
    // We shouldn't see a key change yet.
    assert(key_tick() == KEY_NO_CHANGE);
    set_hal_key_pressed(true);
    assert(key_tick() == KEY_NO_CHANGE);
  }

  // Now let the key settle at low.
  set_hal_key_pressed(false);

  // Wait two ticks to settle.
  verify_state(2, KEY_NO_CHANGE);

  // Next update should be a keyup.
  assert(key_tick() == KEY_UP);

  // 10 more updates with the key up, should have no change.
  verify_state(10, KEY_NO_CHANGE);
}

void test_long_press(int press_ticks) {
  printf("Test: key_long_press: %d ticks\n", press_ticks);

  set_hal_key_pressed(false);
  // 10 ticks with pressed false - we should
  // see no change.
  verify_state(10, KEY_NO_CHANGE);

  // Now set the key settle at high.
  set_hal_key_pressed(true);

  // Wait two ticks to settle.
  verify_state(2, KEY_NO_CHANGE);

  // Next update should be a keydown.
  assert(key_tick() == KEY_DOWN);

  // Note that we count up 3 fewer ticks because
  // of the debouncing on the key-up.
  verify_state(press_ticks - 3, KEY_NO_CHANGE);

  // Now let the key settle to the released state.
  set_hal_key_pressed(false);

  verify_state(2, KEY_NO_CHANGE);

  // Check for a keyup or key_up_long based on the amount of ticks
  // we kept everything pressed.
  assert(key_tick() == ((press_ticks >= 1000) ? KEY_UP_LONG: KEY_UP));
}


int main(void) {
  key_init();

  test_debounce(0);
  test_debounce(10);

  test_long_press(999);
  test_long_press(1000);

  return 0;
}
