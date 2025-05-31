#include <assert.h>
#include <stdio.h>

#include "capture.h"
#include "morse.h"

#define TIMEOUT_TICKS 2000

void test_timeout(void) {
  printf("Test: capture_timeout\n");
  capture_reset();

  // Just keep incrementing the ticks till we expect
  // to see it timeout.
  for (int i = 0; i < TIMEOUT_TICKS - 1; i++) {
    capture_increment();
    assert(capture_timeout() == false);
  }
  // This tick should push us into timeout.
  capture_increment();
  assert(capture_timeout());
}

void test_single(void) {
  printf("Test: capture_single\n");
  capture_reset();
  morse_reset();
  // Set the morse machine to P
  morse_set('P' - 'A');

  // Feed the output of the morse machine into the capture device.
  // This should represent a perfect echo.
  bool finished = false;
  while (!finished) {
    capture_increment();
    switch (morse_tick()) {
      case MORSE_START_MARK:
        // The ticks till now have been space ticks.
        capture_push_space();
        break;
      case MORSE_START_SPACE:
        // The ticks till now have been mark ticks.
        capture_push_mark();
        break;
      case MORSE_HOLD:
        break;
      case MORSE_NONE:
        finished = true;
        break;
    }
  }

  // Capture should match.
  assert(capture_match());
}        

int main(void) {
  test_timeout();
  test_single();
}
