#include <stdbool.h>
#include "tone.h"

bool tone_enabled = false;

void tone_init(void) {
}

void tone_enable(bool enable) {
  tone_enabled = enable;
}

void tone_tick(void) {
}
