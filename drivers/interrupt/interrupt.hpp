/*
 * ATmega32A Driver — External Interrupts (INT0, INT1, INT2)
 * Developed by Ali Sahafi <ali.sahafi@gmail.com> with help from Claude AI.
 *
 * Full usage guide: readme.pdf (this folder). Worked example: example.cpp.
 *
 * Quick usage (call these on the shared `ExternalInterrupt` object below):
 *
 *   void onPress() { ... }                    // your function = the ISR
 *
 *   ExternalInterrupt.enable(INT0, INT_RISING, onPress);
 *   ExternalInterrupt.enable(INT0, INT_FALLING, onPress, 20); // + 20 ms debounce
 *   ExternalInterrupt.disable(INT0);
 *
 *     pin  : INT0 = PD2 (push button S11 on the board)
 *            INT1 = PD3
 *            INT2 = PB2 (LED D2 on the board!)
 *     when : INT_FALLING    HIGH -> LOW  (button pressed)
 *            INT_RISING     LOW -> HIGH  (button released)
 *            INT_ANY_EDGE   both edges            (INT0, INT1 only)
 *            INT_LOW_LEVEL  again and again while the pin is LOW (INT0, INT1 only)
 *
 * enable() makes the pin an input with pull-up and calls sei() for you.
 * A wrong pin or edge (written as a constant) is a compile error.
 *
 * Debounce: the buttons on the board have no hardware debouncer, so one
 * press can make several edges. With a debounce time (in ms, 1..255) the
 * driver waits that long after the first edge, forgets the extra edges, and
 * calls your function only if the pin is still at the new level (for
 * INT_FALLING: still pressed). One press -> one call. While it waits, all
 * other interrupts wait too: use it for buttons, not for fast signals.
 *
 * Interrupts of the other peripherals are in their own drivers:
 *   Timer0.onOverflow(f), Timer0.onCompareMatch(f)   (timer.hpp)
 *   ADC.onComplete(f)                                (adc.hpp)
 *   UART.onReceive(f)                                (uart.hpp)
 */

#ifndef ATMEGA32A_INTERRUPT_HPP
#define ATMEGA32A_INTERRUPT_HPP

#include "../common/common.hpp"

// ---------------------------------------------------------
// Options: when the interrupt comes (value = ISCn1:ISCn0 bits)
// ---------------------------------------------------------
#define INT_LOW_LEVEL 0
#define INT_ANY_EDGE 1
#define INT_FALLING 2
#define INT_RISING 3

namespace extint_detail {
// Compile-time checks (see the prescaler check in timer.hpp for the idea).
void pinError() __attribute__((error(
    "ExternalInterrupt: the pin must be INT0 (PD2), INT1 (PD3) or INT2 (PB2)")));
void edgeError() __attribute__((error(
    "ExternalInterrupt: use INT_FALLING, INT_RISING, INT_ANY_EDGE or INT_LOW_LEVEL")));
void int2EdgeError() __attribute__((error(
    "ExternalInterrupt: INT2 can only use INT_FALLING or INT_RISING")));

// Debounce: wait `ms`, forget the edges that came during the wait (write 1
// to the INTFn flag), then check that the pin is still at the new level.
static inline bool settled(uint8_t ms, uint8_t when, uint8_t intfBit,
                           volatile uint8_t &pinReg, uint8_t pinBit) {
  for (uint8_t i = 0; i < ms; i++)
    _delay_ms(1);
  GIFR = (1 << intfBit);
  bool high = pinReg & (1 << pinBit);
  if (when == INT_RISING) return high;           // still released
  if (when == INT_ANY_EDGE) return true;         // any level is fine
  return !high;                                  // INT_FALLING / LOW_LEVEL: still pressed
}
} // namespace extint_detail

// ---------------------------------------------------------
// External Interrupt Driver
// ---------------------------------------------------------
class ExternalInterrupt_Driver {
public:
  static void (*callback0)();
  static void (*callback1)();
  static void (*callback2)();
  static uint8_t debounce[3]; // ms, 0 = off   (index 0/1/2 = INT0/1/2)
  static uint8_t edge[3];

  // Call `function` every time the chosen edge comes on the pin.
  // `pin` is INT0, INT1 or INT2 (the names from <avr/io.h>).
  // debounceMs: 0 = off (default), 1..255 = wait this long (for buttons).
  static inline __attribute__((always_inline)) void
  enable(uint8_t pin, uint8_t when, void (*function)(), uint8_t debounceMs = 0) {
    if (__builtin_constant_p(pin) && pin != INT0 && pin != INT1 && pin != INT2)
      extint_detail::pinError();
    if (__builtin_constant_p(when) && when > INT_RISING)
      extint_detail::edgeError();
    if (__builtin_constant_p(pin) && __builtin_constant_p(when) && pin == INT2 &&
        when < INT_FALLING)
      extint_detail::int2EdgeError();

    if (pin == INT0) {
      callback0 = function;
      debounce[0] = debounceMs;
      edge[0] = when;
      DDRD &= ~(1 << PD2);                          // input
      PORTD |= (1 << PD2);                          // pull-up
      MCUCR = (MCUCR & ~0x03) | (when & 0x03);      // ISC01:ISC00
      GIFR = (1 << INTF0);                          // clear an old request
      GICR |= (1 << INT0);                          // enable INT0
    } else if (pin == INT1) {
      callback1 = function;
      debounce[1] = debounceMs;
      edge[1] = when;
      DDRD &= ~(1 << PD3);
      PORTD |= (1 << PD3);
      MCUCR = (MCUCR & ~0x0C) | ((when & 0x03) << 2); // ISC11:ISC10
      GIFR = (1 << INTF1);
      GICR |= (1 << INT1);
    } else if (pin == INT2) {
      callback2 = function;
      debounce[2] = debounceMs;
      edge[2] = when;
      DDRB &= ~(1 << PB2);
      PORTB |= (1 << PB2);
      GICR &= ~(1 << INT2);              // datasheet: disable INT2 while ISC2 changes
      if (when == INT_RISING) MCUCSR |= (1 << ISC2);
      else MCUCSR &= ~(1 << ISC2);       // INT_FALLING
      GIFR = (1 << INTF2);
      GICR |= (1 << INT2);
    }
    sei();
  }

  // Stop the interrupt on this pin (the pin stays an input).
  static inline void disable(uint8_t pin) {
    if (pin == INT0 || pin == INT1 || pin == INT2)
      GICR &= ~(1 << pin); // INT0/INT1/INT2 are the bit numbers in GICR
  }
};

__attribute__((weak)) void (*ExternalInterrupt_Driver::callback0)() = nullptr;
__attribute__((weak)) void (*ExternalInterrupt_Driver::callback1)() = nullptr;
__attribute__((weak)) void (*ExternalInterrupt_Driver::callback2)() = nullptr;
__attribute__((weak)) uint8_t ExternalInterrupt_Driver::debounce[3] = {0, 0, 0};
__attribute__((weak)) uint8_t ExternalInterrupt_Driver::edge[3] = {0, 0, 0};

static ExternalInterrupt_Driver ExternalInterrupt __attribute__((unused));

ISR(INT0_vect) {
  ExternalInterrupt_Driver &d = ExternalInterrupt;
  if (d.debounce[0] && !extint_detail::settled(d.debounce[0], d.edge[0], INTF0, PIND, PD2))
    return;
  if (d.callback0) d.callback0();
}
ISR(INT1_vect) {
  ExternalInterrupt_Driver &d = ExternalInterrupt;
  if (d.debounce[1] && !extint_detail::settled(d.debounce[1], d.edge[1], INTF1, PIND, PD3))
    return;
  if (d.callback1) d.callback1();
}
ISR(INT2_vect) {
  ExternalInterrupt_Driver &d = ExternalInterrupt;
  if (d.debounce[2] && !extint_detail::settled(d.debounce[2], d.edge[2], INTF2, PINB, PB2))
    return;
  if (d.callback2) d.callback2();
}

#endif // ATMEGA32A_INTERRUPT_HPP
