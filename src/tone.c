#include <stdbool.h>
#include <avr/io.h>

#include "tone.h"

static bool tone_enabled = false;

void tone_init(void) {
  // Set up PA6 as an output, we'll be using this to directly create a
  // square wave.
  PORTA.DIRSET = PIN6_bm;
  PORTA.OUTCLR = PIN6_bm;
}

void tone_enable(bool enable) {
  if (enable == tone_enabled) {
    return;
  }
  tone_enabled = enable;
  if (!enable) {
    PORTA.OUTCLR = PIN6_bm;
  }
}

void tone_tick(void) {
  if (tone_enabled) {
    // At 1ms, we'll have a frequency of 50% or around
    // 500Hz
    PORTA.OUTTGL = PIN6_bm;
  }
}
