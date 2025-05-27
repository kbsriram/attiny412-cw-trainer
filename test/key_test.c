#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "key.h"

extern void set_hal_key_pressed(bool v);

void test_debounce(int nbounce) {
  printf("Test: key_debounce: %d bounces\n", nbounce);

  set_hal_key_pressed(false);
  // Have 10 updates with pressed false - we should
  // see no change.
  for (int i = 0; i < 10; i++) {
    assert(key_update() == KEY_NO_CHANGE);
  }

  // Bounce the key as many times as requested.
  for (int i = 0; i < nbounce; i++) {
    set_hal_key_pressed(true);
    // We shouldn't see a key change yet.
    assert(key_update() == KEY_NO_CHANGE);
    set_hal_key_pressed(false);
    assert(key_update() == KEY_NO_CHANGE);
  }

  // Now let the key settle at high.
  set_hal_key_pressed(true);

  // Wait two ticks to settle.
  for (int i = 0; i < 2; i++) {
    assert(key_update() == KEY_NO_CHANGE);
  }
  // Next update should be a keydown.
  assert(key_update() == KEY_DOWN);

  // 10 more updates with the key down, should have no change.
  for (int i = 0; i < 10; i++) {
    assert(key_update() == KEY_NO_CHANGE);
  }

  // Bounce for the key down
  for (int i = 0; i < nbounce; i++) {
    set_hal_key_pressed(false);
    // We shouldn't see a key change yet.
    assert(key_update() == KEY_NO_CHANGE);
    set_hal_key_pressed(true);
    assert(key_update() == KEY_NO_CHANGE);
  }

  // Now let the key settle at low.
  set_hal_key_pressed(false);

  // Wait two ticks to settle.
  for (int i = 0; i < 2; i++) {
    assert(key_update() == KEY_NO_CHANGE);
  }
  // Next update should be a keyup.
  assert(key_update() == KEY_UP);

  // 10 more updates with the key up, should have no change.
  for (int i = 0; i < 10; i++) {
    assert(key_update() == KEY_NO_CHANGE);
  }
}

void test_long_press(int press_ticks) {
  printf("Test: key_long_press: %d ticks\n", press_ticks);

  set_hal_key_pressed(false);
  // Have 10 updates with pressed false - we should
  // see no change.
  for (int i = 0; i < 10; i++) {
    assert(key_update() == KEY_NO_CHANGE);
  }

  // Now set the key settle at high.
  set_hal_key_pressed(true);

  // Wait two ticks to settle.
  for (int i = 0; i < 2; i++) {
    assert(key_update() == KEY_NO_CHANGE);
  }
  // Next update should be a keydown.
  assert(key_update() == KEY_DOWN);

  // Note that we count up 3 more ticks because
  // of the debouncing on the key-up.
  for (int i = 0; i < press_ticks - 3; i++) {
    assert(key_update() == KEY_NO_CHANGE);
  }

  // Now let the key settle to the released state.
  set_hal_key_pressed(false);

  for (int i = 0; i < 2; i++) {
    assert(key_update() == KEY_NO_CHANGE);
  }

  // Check for a keyup or key_up_long based on the amount of ticks
  // we kept everything pressed.
  assert(key_update() == ((press_ticks >= 1000) ? KEY_UP_LONG: KEY_UP));
}


int main(void) {
  key_init();

  test_debounce(0);
  test_debounce(10);

  test_long_press(999);
  test_long_press(1000);

  return 0;
}
