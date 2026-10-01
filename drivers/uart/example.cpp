/*
 * UART Example -- echo every character together with its ASCII code
 * Build & flash: make uart
 *
 * Open a serial terminal on the PC: 9600 baud, 8 data bits, no parity,
 * 1 stop bit (8N1). Type a character, for example A, and the board answers:
 *     You typed: A  (ASCII 65)
 */

#define F_CPU 8000000UL
#include "uart.hpp"

int main() {
  UART.begin(9600); // 8N1

  UART.println("ATmega32A ready. Type something!");

  while (true) {
    if (UART.available()) {         // a byte has arrived (RXC flag)
      uint8_t c = UART.read();      // read it from UDR

      UART.print("You typed: ");
      UART.write(c);                // the byte itself -> shown as a character
      UART.print("  (ASCII ");
      UART.print(c);                // the same byte as a number -> "65"
      UART.println(")");
    }
  }
}
