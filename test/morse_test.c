#include <assert.h>
#include <stdio.h>

#include "morse.h"

static void verify_ticks(char* msg, int count, morse_action_t action) {
  for (int i = 0; i < count; i++) {
    morse_action_t actual = morse_tick();
    if (actual != action) {
      printf("%s: At count %d: expected %d, but got %d\n",
             msg, i, action, actual);
      assert(false);
    }
  }
}

void test_action_when_reset(void) {
  printf("Test: morse_action_when_reset\n");
  morse_reset();
  verify_ticks("reset", 10, MORSE_NONE);
}

void test_random_generate(void) {
  printf("Test: morse_random_generate\n");
  morse_reset();
  morse_random_generate(5, 0);
  // Reset to E, T, A, N, R (dit, dah, di-dah, da-dit, di-da-dit)
  morse_buf[0] = 0b00000010;
  morse_buf[1] = 0b00000011;
  morse_buf[2] = 0b00000101;
  morse_buf[3] = 0b00000110;
  morse_buf[4] = 0b00001010;

  // Start with 8 dit spaces before a test begins.
  verify_ticks("initial pause", DIT_TICKS * 8 - 1, MORSE_HOLD);

  // expected sequence of
  // [mark] [space] [mark] [space]...
  // durations as dit counts.
  // We sent E, T, A, N, R, and these pairs of numbers
  // represent mark/space dit counts.
  int expected[] = {
    1, 4, // E followed by letter space
    3, 4, // T followed by letter space
    1, 1,
    3, 4, // A followed by letter space
    3, 1,
    1, 4, // N followed by letter space
    1, 1,
    3, 1,
    1, 1, // Final R just ends with an element space.
  };

  // Step through two at a time.
  for (int i = 0; i < sizeof(expected)/sizeof(int); i += 2) {
    // we expect a mark starting here
    verify_ticks("mark switch", 1, MORSE_START_MARK);
    // mark duration - 1 ticks in hold
    verify_ticks("mark duration", DIT_TICKS * expected[i] - 1, MORSE_HOLD);

    // then a space
    verify_ticks("space switch", 1, MORSE_START_SPACE);
    // space duration - 1 ticks in hold
    verify_ticks("space duration", DIT_TICKS * expected[i + 1] - 1, MORSE_HOLD);
  }

  // We should be done at this point.
  verify_ticks("completed", 10, MORSE_NONE);
}

int main(void) {
  test_action_when_reset();
  test_random_generate();
  return 0;
}
