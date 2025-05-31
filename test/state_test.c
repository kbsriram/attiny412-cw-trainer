#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include "morse.h"
#include "state.h"

#define DIT_TICKS 60

extern bool tone_enabled;
extern void set_hal_key_pressed(bool v);

void verify_tone(int ticks, bool value) {
  for (int i = 0; i < ticks; i++) {
    state_tick();
    if (tone_enabled != value) {
      printf("verify_tone: count=%d: expected tone is %d, but is %d\n",
             i, value, tone_enabled);
      assert(false);
    }
  }
}

void verify_mark_space_dits(int* expected, int count_pairs) {
  for (int i = 0; i < count_pairs; i++) {
    verify_tone(expected[i * 2] * DIT_TICKS, true);
    verify_tone(expected[i * 2 + 1] * DIT_TICKS, false);
  }
}  

void long_press_and_verify_in_practice(void) {
  // Interrupt by pressing our key.
  set_hal_key_pressed(true);

  // Tone should remain silent for the couple of ticks needed
  // to debounce.
  verify_tone(2, false);

  // Tone should be enabled while we keep the key pressed.
  // Let's press it long enough so the key-up becomes a
  // long press.
  verify_tone(1000, true);

  // lift our key.
  set_hal_key_pressed(false);

  // Tone should continue to sound for a couple more ticks,
  // waiting for the debounce to finish.
  verify_tone(2, true);

  // We should now hear the practice announce (didadadit) after
  // a pause of 8 * dit_ticks
  verify_tone(8 * DIT_TICKS, false);

  // di-da-da-dit
  int expected[] = {
    1, 1,
    3, 1,
    3, 1,
    1, 1,
  };
  verify_mark_space_dits(expected, 4);
}

void send_key_down_up(int* sequence, int count) {
  for (int i = 0; i < count; i++) {
    // Alternate key down, key up
    bool desired_key_state = (i % 2) == 0;
    set_hal_key_pressed(desired_key_state);

    // Tone should remain at the previous state for the couple of
    // ticks needed to debounce.
    verify_tone(2, !desired_key_state);

    // Continue for duration after account for the debounce ticks.
    verify_tone(sequence[i] * DIT_TICKS - 2, desired_key_state);
  }
}

void test_reset(void) {
  printf("Test: state_reset\n");
  state_reset();

  // We should be in straight key mode, which should start with
  // word pause followed by a di-di-dit (S) announce.

  // word pause
  verify_tone(8 * DIT_TICKS - 1, false);

  // di-di-dit mark/space timing.
  int expected[] = {
    1, 1,
    1, 1,
    1, 1,
  };
  verify_mark_space_dits(expected, 3);

  // silence after that.
  verify_tone(1000, false);
}

void test_straight_key(void) {
  printf("Test: state_straight_key\n");
  state_reset();

  // The tone should remain silent.
  verify_tone(100, false);

  // Interrupt the announce by pressing our key.
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
  verify_tone(1000, false);
}

void test_straight_key_long_press(void) {
  printf("Test: state_straight_key_long_press\n");
  state_reset();

  // The tone should remain silent.
  verify_tone(100, false);

  long_press_and_verify_in_practice();
}

void test_practice_sending(void) {
  printf("Test: state_practice_sending\n");
  state_reset();

  // The tone should remain silent.
  verify_tone(100, false);

  // Get into practice mode.
  long_press_and_verify_in_practice();

  // The next tick should initialize the morse generator.
  state_tick();

  // We should have generated two characters in the morse machine.
  assert(morse_buf_len == 2);

  // Reset buffer to 'N' 'R' (dadit, didadit)
  morse_buf[0] = 0b00000110;
  morse_buf[1] = 0b00001010;

  // expected_farnsworth_dits = 3;
  // We should get a word + farnsworth amount of dit silence
  // before we get to the sending.
  verify_tone((8 + 3) * DIT_TICKS - 1, false);

  // We expect a N R sequence of marks and spaces.
  int expected[] = {
    3, 1,
    1, 4 + 3, // letter + farnsworth spacing after N
    1, 1,
    3, 1,
    1, 1,
  };
  verify_mark_space_dits(expected, 5);
}

void test_practice_sending_timeout(void) {
  printf("Test: state_practice_sending_timeout\n");
  state_reset();

  // The tone should remain silent.
  verify_tone(100, false);

  // Get into practice mode.
  long_press_and_verify_in_practice();

  // The next tick should initialize the morse generator.
  state_tick();

  // We should have generated two characters in the morse machine.
  assert(morse_buf_len == 2);

  // Reset buffer to 'E' 'T' (dit, dah)
  morse_buf[0] = 0b00000010;
  morse_buf[1] = 0b00000011;

  // Check 3 cycles of timeouts.
  for (int i = 0; i < 3; i++) {
    // expected_farnsworth_dits = 3;
    // We should get a word + farnsworth amount of dit silence
    // before we get to the sending.
    verify_tone((8 + 3) * DIT_TICKS - 1, false);

    // We expect a E T sequence of marks and spaces.
    int expected[] = {
      1, 4 + 3, // letter + farnsworth spacing after E
      3, 1,
    };
    verify_mark_space_dits(expected, 2);

    // We should timeout in 2000 ticks.
    verify_tone(2000, false);

    // Morse machine should rewind back in this tick, with a word +
    // farnsworth amount of delay.
    state_tick();
  }
}

void test_practice_sending_correct(void) {
  printf("Test: state_practice_sending_correct\n");
  state_reset();

  // The tone should remain silent.
  verify_tone(100, false);

  // Get into practice mode.
  long_press_and_verify_in_practice();

  // The next tick should initialize the morse generator.
  state_tick();

  // We should have generated two characters in the morse machine.
  assert(morse_buf_len == 2);

  // Reset buffer to 'E' 'T' (dit, dah)
  morse_buf[0] = 0b00000010;
  morse_buf[1] = 0b00000011;

  // expected_farnsworth_dits = 3;
  // We should get a word + farnsworth amount of dit silence
  // before we get to the sending.
  verify_tone((8 + 3) * DIT_TICKS - 1, false);

  // We expect a E T sequence of marks and spaces.
  int expected[] = {
    1, 4 + 3, // letter + farnsworth spacing after E
    3, 1,
  };
  verify_mark_space_dits(expected, 2);

  // Wait a few ticks
  verify_tone(100, false);

  // Send out a perfect dit <letter> dah
  int key_sequence[] = {
    1, 4, 3, 0
  };
  send_key_down_up(key_sequence, 4);

  // We should timeout in 2000 ticks.
  verify_tone(2000, false);

  // We should get graded on this tick.
  state_tick();

  // We should see something other than E T in the buffer now.
  assert((morse_buf[0] != 0b00000010) || (morse_buf[1] != 0b00000011));
}

int main(void) {
  test_reset();
  test_straight_key();
  test_straight_key_long_press();
  test_practice_sending();
  test_practice_sending_timeout();
  test_practice_sending_correct();
  return 0;
}
