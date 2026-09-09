/*
 * GPIO Example -- Button-controlled LED on ATmega32A
 * Build & flash: make gpio
 */

#define F_CPU 8000000UL
#include "gpio.hpp"

int main() {
  GPIO.setDirection(PORTD, PD2, INPUT_PULLUP); // Push button S11
  GPIO.setDirection(PORTB, PB0, OUTPUT);       // LED D0 (active-low)
  GPIO.setDirection(PORTB, PB1, OUTPUT);       // LED D1 (blinking)

  while (true) {
    // Button S11 controls LED D0 (active-low: LOW = pressed / ON)
    if (GPIO.read(PORTD, PD2) == LOW) {
      GPIO.write(PORTB, PB0, LOW);  // LED ON
    } else {
      GPIO.write(PORTB, PB0, HIGH); // LED OFF
    }

    GPIO.toggle(PORTB, PB1); // Heartbeat blink
    _delay_ms(200);
  }
}
