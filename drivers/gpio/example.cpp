/*
 * GPIO Example -- button-controlled LED
 *
 * Wiring:
 *   - Push button between PD2 and GND (internal pull-up is used)
 *   - LED + 330R resistor from PC0 to GND
 *   - LED + 330R resistor from PC1 to GND (blinks continuously)
 *
 * Build & flash:  make gpio
 */

#define F_CPU 8000000UL
#include "gpio.hpp"

int main() {
  GPIO.setDirection(DDRD, PD2, INPUT_PULLUP); // button (reads LOW when pressed)
  GPIO.setDirection(DDRC, PC0, OUTPUT);       // LED follows the button
  GPIO.setDirection(DDRC, PC1, OUTPUT);       // LED blinks on its own

  while (true) {
    // Button is active-low: pressed = LOW -> turn LED on
    if (GPIO.read(PIND, PD2) == LOW) {
      GPIO.write(PORTC, PC0, HIGH);
    } else {
      GPIO.write(PORTC, PC0, LOW);
    }

    GPIO.toggle(PORTC, PC1);
    _delay_ms(200);
  }

  return 0;
}
