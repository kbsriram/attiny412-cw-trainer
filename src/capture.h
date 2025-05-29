#pragma once

// This library records (up to 50) "ticks" counts representing marks
// and spaces sent by the user.
//
// It can further grade the recorded sequence against an expected
// morse code sequence.

#include <stdbool.h>

void capture_reset(void);
void capture_tick(void);
void capture_push(void);
bool capture_match(void);
