#pragma once

// This is a library for a machine that generates random morse
// characters. It "plays" these characters by keeping track of the
// internal state of mark and space durations needed to generate these
// characters.
//
// The caller first sets a new random sequence via
// morse_random_generate() Each subsequent call to morse_tick()
// returns one of START_MARK, START_SPACE, HOLD or NONE to represent
// an action to be taken at that tick. HOLD is used to indicate the
// last action is to be maintained, and NONE means there are no
// pending morse characters waiting to be sent.
//
// The machine can be restarted with morse_rewind() or flushed (meaning
// it's as though the machine finished sending all the characters, but
// other state is retained.)
//
// morse_reset() resets the machine entirely, so it returns to an
// empty state.

#include <stdbool.h>
#include <stdint.h>

typedef enum _morse_action_t {
  MORSE_NONE,
  MORSE_HOLD,
  MORSE_START_MARK,
  MORSE_START_SPACE,
} morse_action_t;

void morse_reset(void);

void morse_set(uint8_t char_idx);

void morse_flush(void);

void morse_rewind(void);

void morse_random_generate(uint8_t nchars, uint8_t farnsworth_dit_spacing);

morse_action_t morse_tick(void);

bool morse_is_dah(uint8_t encoded, uint8_t pos);

uint8_t morse_num_elements(uint8_t encoded);

extern uint8_t morse_buf[];
extern uint8_t morse_buf_len;

#define WPM 20
// 50 dits per 'standard word', so
// 50 * WPM dits per minute
// => each dit takes 60/(50 * WPM) seconds
// => 60 * 1000 / (50 * WPM) msec
// => 1200 / WPM msec.
// each tick is also 1ms, so this should be the same
// as the number of ticks.
#define DIT_TICKS ((1200 / WPM))
