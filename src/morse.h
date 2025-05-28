#pragma once

// This is a library for a state machine that generates random morse
// characters. It further "plays" these characters by keeping track of
// the internal state of mark and space durations needed to generate
// these characters.
//
// The caller first sets a new random sequence via
// morse_random_generate() Each subsequent call to morse_tick()
// returns one of START_MARK, START_SPACE, HOLD or NONE to represent
// an action to be taken at that tick. HOLD is used to indicate the
// last action is to be maintained, and NONE means there are no pending
// morse characters waiting to be sent.

#include <stdbool.h>
#include <stdint.h>

typedef enum _morse_action_t {
  MORSE_NONE,
  MORSE_HOLD,
  MORSE_START_MARK,
  MORSE_START_SPACE,
} morse_action_t;

void morse_reset(void);

void morse_random_generate(uint8_t nchars, uint8_t farnsworth_dit_spacing);

morse_action_t morse_tick(void);
