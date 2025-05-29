#include <stdbool.h>
#include <stdint.h>

#include "capture.h"
#include "morse.h"

#define TIMING_BUF_MAX 50

// Only bother to record times up to this many ticks.
#define TIMING_TICKS_MAX 2000

// Captured mark/space timings in ticks.
// Negative values are spaces, positive values
// are marks.
static int16_t timing[TIMING_BUF_MAX] = {0};

// length of captured timings.
static uint8_t timing_len = 0;

// are we accumulating a mark?
bool in_mark = false;

void capture_reset(void) {
  timing_len = 0;
  for (int i = 0; i < TIMING_BUF_MAX; i++) {
    timing[i] = 0;
  }
  in_mark = false;
}

void capture_increment(void) {
  if (timing[timing_len] < TIMING_TICKS_MAX) {
    timing[timing_len]++;
  }
}

void capture_push_mark(void) {
  if (timing_len < TIMING_BUF_MAX) {
    timing_len++;
  }
  // Having pushed a mark, we're now capturing a space.
  in_mark = false;
}

void capture_push_space(void) {
  if (timing_len == 0) {
    // Skip capturing the space if that's the first timing we have. We
    // can't really make use of it. But make sure we reset the tick
    // counter for the upcoming mark.
    timing[0] = 0;
  } else if (timing_len < TIMING_BUF_MAX) {
    // Record it as a space.
    timing[timing_len] = -timing[timing_len];
    timing_len++;
  }
  // Having pushed a space, we're now capturing a mark.
  in_mark = true;
}

bool capture_timeout(void) {
  return (!in_mark && timing[timing_len] >= TIMING_TICKS_MAX);
}

bool capture_match(void) {
  // For now, just check that the lengths of the
  // timings and the sum of the elements from the morse
  // buffer match.
  uint8_t expected_elements = 0;
  for (uint8_t i = 0; i < morse_buf_len; i++) {
    expected_elements += morse_num_elements(morse_buf[i]);
  }
  return (expected_elements == timing_len);
}
