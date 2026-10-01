/*
 * ADC Example -- a small voltmeter: read the potentiometer and send the
 * result to the PC two times per second.
 * Build & flash: make adc
 *
 * Open a serial terminal on the PC: 9600 baud, 8N1. Turn the potentiometer
 * and watch the numbers, for example:
 *     ADC = 759   (3706 mV)
 *
 * The potentiometer of the board is on PA0 (ADC channel 0).
 */

#define F_CPU 8000000UL
#include "../uart/uart.hpp"
#include "adc.hpp"

int main() {
  UART.begin(9600);
  ADC.enable(); // reference AREF = 5 V, ADC clock 125 kHz

  while (true) {
    uint16_t value = ADC.read(0);             // 0..1023
    uint16_t mv = ADC.readMillivolts(0);      // 0..4995 mV

    UART.print("ADC = ");
    UART.print(value);
    UART.print("   (");
    UART.print(mv);
    UART.println(" mV)");

    _delay_ms(500);
  }
}
