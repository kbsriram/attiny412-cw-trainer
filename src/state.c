#include <stdlib.h>

#include "capture.h"
#include "key.h"
#include "morse.h"
#include "state.h"
#include "tone.h"

typedef enum _state_mode_t {
  STRAIGHT_KEY,
  PRACTICE,
} state_mode_t;

typedef enum _straight_key_state_t {
  STRAIGHT_KEY_ANNOUNCING,
  STRAIGHT_KEY_READY,
} straight_key_state_t;

typedef enum _practice_state_t {
  PRACTICE_ANNOUNCING,
  PRACTICE_SENDING,
  PRACTICE_WAITING,
} practice_state_t;

static state_mode_t mode = STRAIGHT_KEY;
practice_state_t practice_state = PRACTICE_ANNOUNCING;
static straight_key_state_t straight_key_state = STRAIGHT_KEY_ANNOUNCING;
static uint8_t practice_nchars = 2;
static uint8_t practice_farnsworth_dits = MAX_FARNSWORTH_DITS;
#define MAX_ATTEMPTS 3
static uint8_t practice_attempts = 0;

static uint16_t tick_counter = 0;

static void mode_reset(state_mode_t new_mode) {
  tone_enable(false);
  morse_reset();
  capture_reset();
  practice_attempts = 0;
  practice_nchars = 2;
  practice_farnsworth_dits = MAX_FARNSWORTH_DITS;
  mode = new_mode;
  if (new_mode == STRAIGHT_KEY) {
    morse_set('S' - 'A');
    straight_key_state = STRAIGHT_KEY_ANNOUNCING;
  } else {
    // randomize based on when mode is switched.
    srandom(tick_counter);
    morse_set('P' - 'A');
    practice_state = PRACTICE_ANNOUNCING;
  }    
}

static void straight_key_handle_ready(key_state_t key_state) {
  switch (key_state) {
    case KEY_DOWN:
      tone_enable(true);
      break;

    case KEY_UP:
      tone_enable(false);
      break;

    case KEY_UP_LONG:
      // This should not happen, as we handle this in the main tick
      // handler. But do the right thing anyway.
      mode_reset(PRACTICE);
      break;

    case KEY_NO_CHANGE:
      break;
  }
}

static void practice_start(bool is_new) {
  tone_enable(false);
  capture_reset();
  if (is_new) {
    morse_random_generate(practice_nchars, practice_farnsworth_dits);
  } else {
    morse_rewind();
  }
  practice_state = PRACTICE_SENDING;
}

static bool morse_send_finished(morse_action_t morse_action) {
  bool finished = false;
  switch (morse_action) {
    case MORSE_HOLD:
      // still in progress.
      break;

    case MORSE_NONE:
      // Morse machine is done.
      tone_enable(false);
      finished = true;
      break;

    case MORSE_START_MARK:
      tone_enable(true);
      break;

    case MORSE_START_SPACE:
      tone_enable(false);
      break;
  }
  return finished;
}

static void straight_key_handle(key_state_t key_state, morse_action_t morse_action) {
  switch (straight_key_state) {
    case STRAIGHT_KEY_READY:
      straight_key_handle_ready(key_state);
      return;

    case STRAIGHT_KEY_ANNOUNCING:
      if (key_state != KEY_NO_CHANGE) {
        // User did something during announcement.
        // Reset the morse machine and skip to ready.
        morse_reset();
        straight_key_state = STRAIGHT_KEY_READY;
        straight_key_handle_ready(key_state);
        return;
      }

      if (morse_send_finished(morse_action)) {
        straight_key_state = STRAIGHT_KEY_READY;
        return;
      }
  }
}

static void make_more_difficult(void) {
  // First try to reduce the farnsworth spacing.
  if (practice_farnsworth_dits > 0) {
    practice_farnsworth_dits--;
    return;
  }

  // Then try to increase the number of characters,
  // and resetting the farnsworth spacing.
  if (practice_nchars < 5) {
    practice_nchars++;
    practice_farnsworth_dits = MAX_FARNSWORTH_DITS;
    return;
  }
  // Reached the limit, just keep going.
}

static void make_easier(void) {
  // First try to increase the farnsworth spacing.
  if (practice_farnsworth_dits < MAX_FARNSWORTH_DITS) {
    practice_farnsworth_dits++;
    return;
  }

  // Then try to decrease the number of characters,
  // and resetting the farnsworth spacing.
  if (practice_nchars > 2) {
    practice_nchars--;
    practice_farnsworth_dits = 0;
    return;
  }
  // Reached the limit, just keep retrying.
}

static void practice_grade(void) {
  tone_enable(false);
  if (capture_match() || (practice_attempts >= MAX_ATTEMPTS)) {
    if (practice_attempts < MAX_ATTEMPTS) {
      // Yay, passed the test
      make_more_difficult();
    } else {
      // Ran out of retries.
      make_easier();
    }
    practice_attempts = 0;
    practice_start(/* is_new */ true);
  } else {
    make_easier();
    practice_attempts++;
    practice_start(/* is_new */ false);
  }
}

static void practice_handle_waiting(key_state_t key_state) {
  capture_increment();
  switch (key_state) {
    case KEY_NO_CHANGE:
      if (capture_timeout()) {
        practice_grade();
      }
      break;

    case KEY_UP:
      tone_enable(false);
      capture_push_mark();
      break;

    case KEY_DOWN:
      tone_enable(true);
      capture_push_space();
      break;

    case KEY_UP_LONG:
      // This should not happen, as we handle this in the main tick
      // handler. But do the right thing anyway.
      mode_reset(STRAIGHT_KEY);
      break;
  }
}

static void practice_handle_sending(key_state_t key_state, morse_action_t morse_action) {
  if (key_state != KEY_NO_CHANGE) {
    // Our user has keyed something in SENDING mode. Flush the
    // morse machine, switch to WAITING and handle this tick in
    // WAITING mode.
    morse_flush();
    capture_reset();
    practice_state = PRACTICE_WAITING;
    practice_handle_waiting(key_state);
    return;
  }

  if (morse_send_finished(morse_action)) {
    // Morse has finished sending, switch to waiting mode.
    capture_reset();
    practice_state = PRACTICE_WAITING;
  }
}


static void practice_handle(key_state_t key_state, morse_action_t morse_action) {
  switch (practice_state) {
    case PRACTICE_ANNOUNCING:
      if (key_state != KEY_NO_CHANGE) {
        // User did something during announcing. Skip to
        // starting a new practice.
        morse_reset();
        practice_start(/* is_new */ true);
        return;
      }
      if (morse_send_finished(morse_action)) {
        // Announce is finished, start a new practice block.
        practice_start(/* is_new */ true);
      }
      return;

    case PRACTICE_SENDING:
      practice_handle_sending(key_state, morse_action);
      break;

    case PRACTICE_WAITING:
      practice_handle_waiting(key_state);
      break;
      
  }
}

void state_reset(void) {
  mode_reset(STRAIGHT_KEY);
}

void state_tick(void) {
  tick_counter++;
  tone_tick();
  key_state_t key_state = key_tick();
  morse_action_t morse_action = morse_tick();

  // Handle a mode switch early.
  if (key_state == KEY_UP_LONG) {
    mode_reset((mode == PRACTICE) ? STRAIGHT_KEY: PRACTICE);
    return;
  }

  switch (mode) {
    case STRAIGHT_KEY:
      straight_key_handle(key_state, morse_action);
      break;

    case PRACTICE:
      practice_handle(key_state, morse_action);
      break;
  }
}
