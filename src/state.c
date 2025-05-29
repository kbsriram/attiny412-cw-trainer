#include "capture.h"
#include "key.h"
#include "morse.h"
#include "state.h"
#include "tone.h"

typedef enum _state_t {
  STRAIGHT_KEY_SETUP,
  STRAIGHT_KEY_READY,

  PRACTICE_SETUP,
  PRACTICE_CREATE,
} state_t;

static state_t state = STRAIGHT_KEY_SETUP;

static void handle_key_down(void) {
  tone_enable(true);
  switch (state) {
    case STRAIGHT_KEY_SETUP:
      // flush the morse machine and skip to ready.
      morse_flush();
      state = STRAIGHT_KEY_READY;
      break;
    case STRAIGHT_KEY_READY:
      // nothing to do
      break;

    case PRACTICE_SETUP:
      // TODO:
      break;
  }
}

static void handle_key_up(void) {
  tone_enable(false);
  switch (state) {
    case STRAIGHT_KEY_SETUP:
      // flush the morse machine and skip to ready.
      morse_flush();
      state = STRAIGHT_KEY_READY;
      break;
    case STRAIGHT_KEY_READY:
      // nothing to do
      break;

    case PRACTICE_SETUP:
      // TODO:
      break;
  }
}

static void state_reset_to(state_t new_state) {
  capture_reset();
  morse_reset();
  tone_enable(false);
  state = new_state;
  if (new_state == STRAIGHT_KEY_SETUP) {
    morse_set('S' - 'A');
  } else {
    morse_set('P' - 'A');
  }    
}

void state_reset(void) {
  state_reset_to(STRAIGHT_KEY_SETUP);
}

void state_tick(void) {
  // Update basic ticks.
  tone_tick();
  key_state_t key_state = key_tick();
  morse_action_t morse_action = morse_tick();

  // Check user input first.
  switch (key_state) {
    case KEY_UP_LONG:
      // Toggle straight/practice modes.
      if (state <= STRAIGHT_KEY_READY) {
        state_reset_to(PRACTICE_SETUP);
      } else {
        state_reset_to(STRAIGHT_KEY_SETUP);
      }
      return;

    case KEY_DOWN:
      handle_key_down();
      return;

    case KEY_UP:
      handle_key_up();
      return;

    case KEY_NO_CHANGE:
      break;
  }

  // No user input on this tick. Obey any morse machine instructions.
  if (morse_action == MORSE_START_MARK) {
    tone_enable(true);
  } else if (morse_action == MORSE_START_SPACE) {
    tone_enable(false);
  } else if (morse_action == MORSE_NONE) {
    // 
  }
}
