/*
 * Timer Example -- one timer per mode, all running at the same time
 * Build & flash: make timer
 *
 *   LED D0 (PB0): blinks every 0.5 s   -- Timer1, CTC mode, flag polling
 *   LED D3 (PB3): half brightness      -- Timer0, Fast PWM on its OC0 pin
 *
 * Timer1 CTC period = prescaler * (compare + 1) / F_CPU
 *                   = 256 * (15624 + 1) / 8 MHz = 0.5 s
 */

#define F_CPU 8000000UL
#include "../gpio/gpio.hpp"
#include "timer.hpp"

int main() {
  GPIO.setDirection(PORTB, PB0, OUTPUT); // LED D0 (active-low)

  // Timer0: Fast PWM, prescaler 64 -> 8 MHz / (64 * 256) = 488 Hz, no flicker.
  // PWM_INVERTING because the LEDs are active-low (duty cycle = brightness).
  Timer0.begin(TIMER_FAST_PWM, 64, PWM_INVERTING); // PB3 becomes an output
  Timer0.setDutyCycle(50);

  // Timer1: CTC, restarts every 15625 counts = 0.5 s
  Timer1.begin(TIMER_CTC, 256);
  Timer1.setCompare(15624);

  while (true) {
    if (Timer1.compareMatched()) { // true once every 0.5 s
      GPIO.toggle(PORTB, PB0);
    }
    // The CPU is free here -- the PWM on D3 needs no code at all.
  }
}
