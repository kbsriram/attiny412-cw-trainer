#include <avr/interrupt.h>

#include "ticks.h"

void ticks_init(void) {
  // The code gets into power-down mode in the main loop, which shuts
  // down all clocks except for the internal low power 32Khz clock.
  //
  // The low power clock is used to drive the RTC PIT, which is
  // configured to wake up the system via an interrupt at 1.024 KHz.
  //
  // When active, the device is configured to use the main clock at
  // full speed. The interrupt is a no-op and just used to wake up the
  // main loop, where all the work happens.

  // Disable the prescalar and run the device at full speed on the
  // main clock.
  _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);

  // Enable the internal ULP 32k oscillator. This will be used to feed
  // the RTC PIT.
  _PROTECTED_WRITE(CLKCTRL.OSC32KCTRLA, CLKCTRL_RUNSTDBY_bm);

  // Wait for all RTC registers to be synchronized
  while (RTC.STATUS > 0) {
  }

  // Configure the RTC to use the ULP oscillator
  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;

  // Enable the PIT, running at 32 cycles. This will result in
  // interrupts occurring at a 1Khz rate (32k / 32) = 1024Hz
  RTC.PITCTRLA = RTC_PERIOD_CYC32_gc | RTC_PITEN_bm;

  // Enable interrupts from the PIT
  RTC.PITINTCTRL = RTC_PI_bm;

  // wait for RTC.PITCTRLA synchronization to be achieved
  while (RTC.PITSTATUS > 0) {
  }
}

// The purpose of the PIT is simply to wake the device up. All the
// work happens in the main loop, which waits to be woken up at 1ms
// intervals.
ISR(RTC_PIT_vect) {
  // clear the interrupt flag
  RTC.PITINTFLAGS = RTC_PI_bm;
}
