#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include "state.h"

#define DIT_TICKS 60

extern bool tone_enabled;
extern void set_hal_key_pressed(bool v);

void verify_tone(int ticks, bool value) {
  for (int i = 0; i < ticks; i++) {
    state_tick();
    assert(tone_enabled == value);
  }
}

void test_reset(void) {
  printf("Test: state_reset\n");
  state_reset();

  // The tone should remain silent.
  verify_tone(100, false);
}

void test_straight_key(void) {
  printf("Test: state_straight_key\n");
  state_reset();

  // The tone should remain silent.
  verify_tone(100, false);

  // press our key.
  set_hal_key_pressed(true);

  // Tone should remain silent for the couple of ticks needed
  // to debounce.
  verify_tone(2, false);

  // Tone should be enabled while we keep the key pressed.
  verify_tone(100, true);

  // lift our key.
  set_hal_key_pressed(false);

  // Tone should continue to sound for a couple more ticks,
  // waiting for the debounce to finish.
  verify_tone(2, true);

  // Tone should be disabled from here on.
  verify_tone(100, false);
}

void test_straight_key_long_press(void) {
  printf("Test: state_straight_key_long_press\n");
  state_reset();

  // The tone should remain silent.
  verify_tone(100, false);

  // press our key.
  set_hal_key_pressed(true);

  // Tone should remain silent for the couple of ticks needed
  // to debounce.
  verify_tone(2, false);

  // Tone should be enabled while we keep the key pressed.
  // Let's press it long enough so the key-up is actually a
  // long press.
  verify_tone(1000, true);

  // lift our key.
  set_hal_key_pressed(false);

  // Tone should continue to sound for a couple more ticks,
  // waiting for the debounce to finish.
  verify_tone(2, true);

  // We should now hear the practice announce (didadadit) after
  // a pause of 8 * dit_ticks - 1
  verify_tone(8 * DIT_TICKS, false);

  // di-da-da-dit
  int expected[] = {
    1, 1,
    3, 1,
    3, 1,
    1, 1,
  };

  for (int i = 0; i < sizeof(expected) / sizeof(int); i+=2) {
    verify_tone(expected[i] * DIT_TICKS, true);
    verify_tone(expected[i+1] * DIT_TICKS, false);
  }
}

int main(void) {
  test_reset();
  test_straight_key();
  test_straight_key_long_press();
}
