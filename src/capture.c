#include <stdbool.h>
#include <stdint.h>

#include "capture.h"
#include "morse.h"

#define TIMING_BUF_MAX 50

// Only bother to record times up to this many ticks.
#define TIMING_TICKS_MAX 2000

// We allow a slop for mark and space timings within
// characters of this many ticks.
#define ELEMENT_SLOP_TICKS 50

// Captured mark/space timings in ticks.
// Negative values are spaces, positive values
// are marks.
static int16_t timing[TIMING_BUF_MAX] = {0};

// length of captured timings.
static uint8_t timing_len = 0;

// are we accumulating a mark?
bool in_mark = false;

static bool is_close(uint16_t actual, bool is_dah) {
  // For a dit:
  // Anything that's at least DIT_TICKS // 2 and not more
  // than DIT_TICKS + DIT_TICKS // 2
  //
  // For a dah:
  // At least 2 * DIT_TICKS and not more than 4 * DIT_TICKS
  // Handle uint comparisons without overflowing.
  if (is_dah) {
    return ((actual >= (2 * DIT_TICKS)) && (actual <= (4 * DIT_TICKS)));
  }
  return ((actual >= (DIT_TICKS / 2)) && (actual <= ((DIT_TICKS * 3) / 2)));
}

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
  // Current position in the timing[] array.
  uint8_t timing_idx = 0;

  // Step through morse buffer.
  for (uint8_t i = 0; i < morse_buf_len; i++) {
    uint8_t morse_encoded = morse_buf[i];
    uint8_t start_bit_pos = morse_num_elements(morse_encoded);
    for (int8_t pos = start_bit_pos - 1; pos >= 0; pos--) {
      // Did we run out of timing elements too soon?
      if (timing_idx >= timing_len) {
        return false;
      }

      // Verify mark duration.
      bool is_dah = morse_is_dah(morse_encoded, pos);
      if (!is_close(timing[timing_idx], is_dah)) {
        // Mark duration failed.
        return false;
      }

      // Verify space duration.
      bool is_last_element = (pos == 0);
      bool is_last_char_element = (i == (morse_buf_len - 1));
      if (is_last_element && is_last_char_element) {
        // We don't record the final char's last space, so
        // just increment and move on.
        timing_idx++;
        continue;
      }

      // Check the key-up duration.
      timing_idx++;
      if (timing_idx >= timing_len) {
        // Stopped too soon.
        return false;
      }

      // note that spaces are stored as negative.
      uint16_t actual = -timing[timing_idx];
      // We'll be flexible about inter letter space, just requiring
      // that we have at least 4 dits overall.
      if (is_last_element) {
        if (actual < 4 * DIT_TICKS - ELEMENT_SLOP_TICKS) {
          return false;
        }
      } else {
        if (!is_close(actual, /* is_dah */ false)) {
          return false;
        }
      }
      // Move on to the next timing element.
      timing_idx++;
    }
  }

  // After checking all characters, we should have consumed all timing
  // entries. Otherwise, the user sent too many elements.
  return (timing_idx == timing_len);
}
