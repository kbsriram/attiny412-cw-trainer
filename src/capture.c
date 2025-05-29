#include <stdbool.h>
#include <stdint.h>

#include "capture.h"
#include "morse.h"

#define TIMING_BUF_MAX 50

// Captured mark/space timings in ticks.
static uint16_t timing[TIMING_BUF_MAX] = {0};

static uint8_t timing_len = 0;

void capture_reset(void) {
  timing_len = 0;
}

void capture_tick(void) {
  if (timing[timing_len] < UINT16_MAX) {
    timing[timing_len]++;
  }
}

void capture_push(void) {
  if (timing_len < TIMING_BUF_MAX) {
    timing_len++;
  }
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
