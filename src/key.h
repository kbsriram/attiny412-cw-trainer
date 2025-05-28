#pragma once

typedef enum _key_state_t {
  KEY_NO_CHANGE,
  KEY_DOWN,
  KEY_UP,
  KEY_UP_LONG,
} key_state_t;

void key_init(void);
key_state_t key_tick(void);
