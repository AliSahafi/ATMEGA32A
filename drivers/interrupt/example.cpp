/*
 * External Interrupt Example -- an up counter on the LEDs, button S11
 * Build & flash: make interrupt
 *
 * Every time the push button S11 (PD2 = INT0) is released, the interrupt
 * adds 1 to the counter. The main loop does nothing: the counting happens
 * only in the interrupt function.
 *
 * Watch out: a mechanical button "bounces". One press can make more than
 * one edge, so the counter sometimes jumps by 2 or 3.
 */

#define F_CPU 8000000UL
#include "../gpio/gpio.hpp"
#include "interrupt.hpp"

volatile uint8_t count = 0; // volatile: changed in the interrupt function

void onRelease() {          // runs automatically on every rising edge of PD2
  count++;
  GPIO.write(PORTB, ~count); // LEDs are active-low
}

int main() {
  GPIO.setDirection(PORTB, ALL, OUTPUT);
  GPIO.write(PORTB, 0xFF); // all LEDs off

  ExternalInterrupt.enable(INT0, INT_RISING, onRelease); // also calls sei()

  while (true) {
    // nothing to do here -- the interrupt does the work
  }
}
