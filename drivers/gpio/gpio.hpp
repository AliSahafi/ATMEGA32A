/*
 * ATmega32A Driver — GPIO Module
 * Developed by Ali Sahafi <ali.sahafi@gmail.com> with help from Claude AI.
 */

#ifndef ATMEGA32A_GPIO_HPP
#define ATMEGA32A_GPIO_HPP

#include "../common/common.hpp"

// ---------------------------------------------------------
// GPIO Driver
// ---------------------------------------------------------
class GPIO_Driver {
public:
  static inline void setDirection(volatile uint8_t &ddr, uint8_t pin,
                                  uint8_t dir) {
    volatile uint8_t *actual_ddr = &ddr;

    // Auto-correct if user passed PORT or PIN instead of DDR
    if (actual_ddr == &PORTA || actual_ddr == &PINA) actual_ddr = &DDRA;
    else if (actual_ddr == &PORTB || actual_ddr == &PINB) actual_ddr = &DDRB;
    else if (actual_ddr == &PORTC || actual_ddr == &PINC) actual_ddr = &DDRC;
    else if (actual_ddr == &PORTD || actual_ddr == &PIND) actual_ddr = &DDRD;

    volatile uint8_t *port = actual_ddr + 1; // In AVR, PORT is always DDR + 1

    if (pin == ALL) {
      if (dir == OUTPUT) {
        *actual_ddr = 0xFF;
      } else if (dir == INPUT_PULLUP) {
        *actual_ddr = 0x00;
        *port = 0xFF; // Enable all pull-ups
      } else {
        *actual_ddr = 0x00;
        *port = 0x00; // Disable all pull-ups
      }
    } else {
      if (dir == OUTPUT) {
        *actual_ddr |= (1 << pin);
      } else if (dir == INPUT_PULLUP) {
        *actual_ddr &= ~(1 << pin);
        *port |= (1 << pin); // Enable pull-up
      } else {
        *actual_ddr &= ~(1 << pin);
        *port &= ~(1 << pin); // Disable pull-up
      }
    }
  }

  static inline void write(volatile uint8_t &port, uint8_t pin, uint8_t state) {
    volatile uint8_t *actual_port = &port;

    // Auto-correct if user passed DDR or PIN instead of PORT
    if (actual_port == &DDRA || actual_port == &PINA) actual_port = &PORTA;
    else if (actual_port == &DDRB || actual_port == &PINB) actual_port = &PORTB;
    else if (actual_port == &DDRC || actual_port == &PINC) actual_port = &PORTC;
    else if (actual_port == &DDRD || actual_port == &PIND) actual_port = &PORTD;

    if (pin == ALL) {
      *actual_port = state;
    } else {
      if (state == HIGH) {
        *actual_port |= (1 << pin);
      } else {
        *actual_port &= ~(1 << pin);
      }
    }
  }

  static inline void write(volatile uint8_t &port, uint8_t value) {
    write(port, ALL, value);
  }

  static inline uint8_t read(volatile uint8_t &pin_reg, uint8_t pin) {
    volatile uint8_t *actual_pin = &pin_reg;

    // Auto-correct if user passed PORT or DDR instead of PIN
    if (actual_pin == &PORTA || actual_pin == &DDRA) actual_pin = &PINA;
    else if (actual_pin == &PORTB || actual_pin == &DDRB) actual_pin = &PINB;
    else if (actual_pin == &PORTC || actual_pin == &DDRC) actual_pin = &PINC;
    else if (actual_pin == &PORTD || actual_pin == &DDRD) actual_pin = &PIND;

    if (pin == ALL)
      return *actual_pin;
    return (*actual_pin & (1 << pin)) ? HIGH : LOW;
  }

  static inline uint8_t read(volatile uint8_t &pin_reg) {
    return read(pin_reg, ALL);
  }

  static inline void toggle(volatile uint8_t &port, uint8_t pin) {
    volatile uint8_t *actual_port = &port;

    // Auto-correct if user passed DDR or PIN instead of PORT
    if (actual_port == &DDRA || actual_port == &PINA) actual_port = &PORTA;
    else if (actual_port == &DDRB || actual_port == &PINB) actual_port = &PORTB;
    else if (actual_port == &DDRC || actual_port == &PINC) actual_port = &PORTC;
    else if (actual_port == &DDRD || actual_port == &PIND) actual_port = &PORTD;

    if (pin == ALL) {
      *actual_port ^= 0xFF;
    } else {
      *actual_port ^= (1 << pin);
    }
  }
};

static GPIO_Driver GPIO __attribute__((unused));

#endif // ATMEGA32A_GPIO_HPP
