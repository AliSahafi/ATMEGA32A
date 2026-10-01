/*
 * ATmega32A Driver — UART Module (serial communication with the PC)
 * Developed by Ali Sahafi <ali.sahafi@gmail.com> with help from Claude AI.
 *
 * Full usage guide: readme.pdf (this folder). Worked example: example.cpp.
 *
 * Quick usage (call these on the shared `UART` object below):
 *
 *   UART.begin(9600);                         // 9600 baud, 8 data bits,
 *                                             // no parity, 1 stop bit (8N1)
 *   UART.begin(9600, UART_PARITY_EVEN, 2);    // with parity and 2 stop bits
 *
 *     parity   : UART_PARITY_NONE (default), UART_PARITY_EVEN, UART_PARITY_ODD
 *     stopBits : 1 (default) or 2
 *
 *   UART.write(65);            // send one byte as it is (65 = 'A' in ASCII)
 *   UART.print("Hello");       // send text
 *   UART.print(count);         // send a number as text: 42 -> "42"
 *   UART.print('A');           // send one character
 *   UART.println(...);         // same as print(), then a new line
 *
 *   UART.available();          // true if a received byte is waiting (RXC)
 *   UART.read();               // wait for a byte and return it (UDR)
 *
 *   UART.onReceive(myFunction);   // call myFunction(byte) for every received
 *                                 // byte (interrupt lecture)
 *
 * Pins: RXD = PD0, TXD = PD1 (connected to the USB-UART chip on the board).
 */

#ifndef ATMEGA32A_UART_HPP
#define ATMEGA32A_UART_HPP

#include "../common/common.hpp"
#include <stdlib.h> // itoa, utoa, ltoa, ultoa, dtostrf

// ---------------------------------------------------------
// Options
// ---------------------------------------------------------
// Parity (value = the UPM1:UPM0 bits in UCSRC)
#define UART_PARITY_NONE 0
#define UART_PARITY_EVEN 2
#define UART_PARITY_ODD 3

// ---------------------------------------------------------
// UART Driver
// ---------------------------------------------------------
class UART_Driver {
public:
  static void (*receiveCallback)(uint8_t);

  // Start the UART: baud rate, parity and stop bits. Always 8 data bits.
  // The driver calculates UBRR = F_CPU / (16 * baud) - 1 (rounded). Only if
  // that baud rate is more than 1 % wrong, and double-speed mode (U2X,
  // UBRR = F_CPU / (8 * baud) - 1) is closer, it uses double speed.
  static inline void begin(uint32_t baudRate, uint8_t parity = UART_PARITY_NONE,
                           uint8_t stopBits = 1) {
    uint16_t ubrrNormal = (F_CPU + 8UL * baudRate) / (16UL * baudRate) - 1;
    uint16_t ubrrDouble = (F_CPU + 4UL * baudRate) / (8UL * baudRate) - 1;
    uint32_t realNormal = F_CPU / (16UL * (ubrrNormal + 1UL));
    uint32_t realDouble = F_CPU / (8UL * (ubrrDouble + 1UL));
    bool normalTooFar = distance(realNormal, baudRate) * 100UL > baudRate;
    bool useDouble = normalTooFar &&
                     distance(realDouble, baudRate) < distance(realNormal, baudRate);
    uint16_t ubrr = useDouble ? ubrrDouble : ubrrNormal;

    UCSRB = 0; // switch off while configuring
    if (useDouble) UCSRA |= (1 << U2X);
    else UCSRA &= ~(1 << U2X);
    UBRRH = (uint8_t)(ubrr >> 8) & 0x0F; // URSEL (bit 7) must be 0 here
    UBRRL = (uint8_t)ubrr;

    // URSEL = 1 selects UCSRC; UCSZ1:0 = 11 -> 8 data bits
    UCSRC = (1 << URSEL) | ((parity & 0x03) << UPM0) |
            ((stopBits == 2) ? (1 << USBS) : 0) | (1 << UCSZ1) | (1 << UCSZ0);
    UCSRB = (1 << RXEN) | (1 << TXEN);
  }

  // ---- Send ----
  // Send one byte exactly as it is (no conversion to text).
  static inline void write(uint8_t data) {
    while (!(UCSRA & (1 << UDRE))) // wait until UDR is empty
      ;
    UDR = data;
  }

  // Send text, a single character, or a number as decimal text.
  static inline void print(const char *text) {
    while (*text) write(*text++);
  }
  static inline void print(const uint8_t *text) { print((const char *)text); }
  static inline void print(char c) { write(c); }
  static inline void print(uint8_t value) { print((uint16_t)value); }
  static inline void print(int8_t value) { print((int16_t)value); }
  static inline void print(int16_t value) {
    char buf[7];
    print(itoa(value, buf, 10));
  }
  static inline void print(uint16_t value) {
    char buf[6];
    print(utoa(value, buf, 10));
  }
  static inline void print(int32_t value) {
    char buf[12];
    print(ltoa(value, buf, 10));
  }
  static inline void print(uint32_t value) {
    char buf[11];
    print(ultoa(value, buf, 10));
  }
  static inline void print(double value, uint8_t decimals = 2) {
    char buf[16];
    print(dtostrf(value, 0, decimals, buf));
  }

  // Same as print(), followed by a new line ("\r\n").
  static inline void println() { print("\r\n"); }
  template <typename T> static inline void println(T value) {
    print(value);
    println();
  }
  static inline void println(double value, uint8_t decimals) {
    print(value, decimals);
    println();
  }

  // ---- Receive ----
  // true if a received byte is waiting to be read (RXC flag).
  static inline bool available() { return UCSRA & (1 << RXC); }

  // Wait until a byte arrives, then return it. Reading UDR clears RXC.
  static inline uint8_t read() {
    while (!available())
      ;
    return UDR;
  }

  // Interrupts (covered in the interrupt lecture): call `callback(byte)` for
  // every received byte. Enables global interrupts (sei). Pass nullptr to
  // switch it off. Keep the callback short; use `volatile` for variables
  // shared with main().
  static inline void onReceive(void (*callback)(uint8_t)) {
    receiveCallback = callback;
    if (callback) UCSRB |= (1 << RXCIE);
    else UCSRB &= ~(1 << RXCIE);
    sei();
  }

private:
  static inline uint32_t distance(uint32_t a, uint32_t b) {
    return a > b ? a - b : b - a;
  }
};

__attribute__((weak)) void (*UART_Driver::receiveCallback)(uint8_t) = nullptr;

static UART_Driver UART __attribute__((unused));

// Receive interrupt: only does something after onReceive() has been called
ISR(USART_RXC_vect) {
  uint8_t data = UDR; // reading UDR clears the interrupt flag
  if (UART.receiveCallback) UART.receiveCallback(data);
}

#endif // ATMEGA32A_UART_HPP
