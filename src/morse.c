#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "morse.h"

#define WPM 20
// 50 dits per 'standard word', so
// 50 * WPM dits per minute
// => each dit takes 60/(50 * WPM) seconds
// => 60 * 1000 / (50 * WPM) msec
// => 1200 / WPM msec.
// each tick is also 1ms, so this should be the same
// as the number of ticks.
#define DIT_TICKS ((1200 / WPM))

// For space efficiency, we represent dits as 0s and dahs as 1s in an
// 8-bit value. In order to know when the encoding begins, we prefix
// the encoding with a 1. For example:

// E = 00000010
// T = 00000011
// Z = 00011100

static const uint8_t ENCODING[] = {
  0b00000101, // A
  0b00011000, // B
  0b00011010, // C
  0b00001100, // D
  0b00000010, // E
  0b00010010, // F
  0b00001110, // G
  0b00010000, // H
  0b00000100, // I
  0b00010111, // J
  0b00001101, // K
  0b00010100, // L
  0b00000111, // M
  0b00000110, // N
  0b00001111, // O
  0b00010110, // P
  0b00011101, // Q
  0b00001010, // R
  0b00001000, // S
  0b00000011, // T
  0b00001001, // U
  0b00010001, // V
  0b00001011, // W
  0b00011001, // X
  0b00011011, // Y
  0b00011100, // Z
  0b00111111, // 0
  0b00101111, // 1
  0b00100111, // 2
  0b00100011, // 3
  0b00100001, // 4
  0b00100000, // 5
  0b00110000, // 6
  0b00111000, // 7
  0b00111100, // 8
  0b00111110, // 9
};

// Holds encoded morse values that we want to send.
uint8_t morse_buf[5] = {0, 0, 0, 0, 0};

// How many morse letters are in the buffer.
uint8_t morse_buf_len = 0;

// How many morse letters have been sent.
static uint8_t morse_buf_sent = 0;

// Current (encoded) morse letter.
static uint8_t morse_letter = 0;

// How many elements does the current letter have.
static uint8_t morse_letter_len = 0;

// How many elements in the current letter have been sent.
static uint8_t morse_letter_sent = 0;

// How many ticks to wait before checking for the next action.
static uint16_t tick_countdown = 0;

// If we're currently sending a mark.
static bool in_mark = false;

// Additional dit delays for character spacing.
static uint8_t extra_dit_spacing = 0;

void morse_reset(void) {
  morse_buf_len = 0;
  morse_buf_sent = 0;
  morse_letter = 0;
  morse_letter_len = 0;
  morse_letter_sent = 0;
  tick_countdown = 0;
  in_mark = false;
  extra_dit_spacing = 0;
}

void morse_rewind(void) {
  morse_buf_sent = 0;
  morse_letter = 0;
  morse_letter_len = 0;
  morse_letter_sent = 0;
  in_mark = false;

  // Set up an initial 5 dit wait when rewinding. When advance()
  // starts working on the buffer, another 3 dit wait is added,
  // forming a word-level wait at this point.
  tick_countdown = 5 * DIT_TICKS;
}



bool morse_is_dah(uint8_t encoded, uint8_t pos) {
  return (encoded & (1 << pos));
}

uint8_t morse_num_elements(uint8_t encoded) {
  uint8_t pos = 7;
  // find the first set bit
  while (!(encoded & (1 << pos))) {
    pos--;
  }
  return pos;
}

static void advance_element(void) {
  // Set up the dit/dah duration and send the next element in our
  // letter.
  if (morse_is_dah(morse_letter, morse_letter_len - 1 - morse_letter_sent)) {
    tick_countdown = 3 * DIT_TICKS;
  } else {
    tick_countdown = DIT_TICKS;
  }
  morse_letter_sent++;
}

static morse_action_t advance(void) {
  // Check if our current letter is fully sent.
  if (morse_letter_sent >= morse_letter_len) {
    // Check if we have more characters to send.
    if (morse_buf_sent >= morse_buf_len) {
      // All done.
      return MORSE_NONE;
    }
    // Advance our buffer, and set up the current character to send.
    morse_letter = morse_buf[morse_buf_sent++];
    morse_letter_len = morse_num_elements(morse_letter);
    morse_letter_sent = 0;
    // Add extra letter spacing for (3 + farnsworth) * DIT ticks
    tick_countdown = DIT_TICKS * (3  + extra_dit_spacing);
    return MORSE_HOLD;
  }
  advance_element();
  in_mark = true;
  return MORSE_START_MARK;
}

void morse_flush(void) {
  morse_buf_sent = morse_buf_len;
  morse_letter_sent = morse_letter_len;
  tick_countdown = 0;
  in_mark = false;
}

void morse_set(uint8_t char_idx) {
  morse_reset();
  if (char_idx >= sizeof(ENCODING)) {
    char_idx = 0;
  }
  morse_buf[0] = ENCODING[char_idx];
  morse_buf_len = 1;
  tick_countdown = 5 * DIT_TICKS;
}

void morse_random_generate(uint8_t nchars, uint8_t extra) {
  morse_reset();
  extra_dit_spacing = extra;
  if (nchars > sizeof(morse_buf)) {
    nchars = sizeof(morse_buf);
  }
  for (uint8_t i = 0; i < nchars; i++) {
    morse_buf[i] = ENCODING[rand() % 26];
  }
  morse_buf_len = nchars;

  // Start off with a word space. Note that letters begin with
  // 3 extra dit ticks, so we add 5 more here to make an 8
  // dit word space.
  tick_countdown = 5 * DIT_TICKS;
}

morse_action_t morse_tick(void) {
  // Fast check when nothing is happening.
  if (!tick_countdown) {
    return MORSE_NONE;
  }

  // decrement our countdown and check for any state
  // updates only when it runs out.
  tick_countdown--;
  if (tick_countdown) {
    return MORSE_HOLD;
  }

  // Handle a common case - when we get out of a mark
  // mode, we always pause for a dit duration.
  if (in_mark) {
    tick_countdown = DIT_TICKS;
    in_mark = false;
    return MORSE_START_SPACE;
  }

  return advance();
}
