#pragma once

// This is the orchestrator for managing user input and sending out
// and grading practice characters.
//
// The overall code is organized around a tick timer interrupt that
// wakes up a main loop at 1ms intervals.
//
// This library is called on each timer tick.

void state_reset(void);
void state_tick(void);
