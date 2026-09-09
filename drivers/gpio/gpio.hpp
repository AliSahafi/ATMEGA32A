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

    // Auto-correct if user accidentally passed PORT instead of DDR
    if (actual_ddr == &PORTA) actual_ddr = &DDRA;
    else if (actual_ddr == &PORTB) actual_ddr = &DDRB;
    else if (actual_ddr == &PORTC) actual_ddr = &DDRC;
    else if (actual_ddr == &PORTD) actual_ddr = &DDRD;

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
    if (pin == ALL) {
      port = state;
    } else {
      if (state == HIGH) {
        port |= (1 << pin);
      } else {
        port &= ~(1 << pin);
      }
    }
  }

  static inline void write(volatile uint8_t &port, uint8_t value) {
    port = value;
  }

  static inline uint8_t read(volatile uint8_t &pin_reg, uint8_t pin) {
    if (pin == ALL)
      return pin_reg;
    return (pin_reg & (1 << pin)) ? HIGH : LOW;
  }

  static inline uint8_t read(volatile uint8_t &pin_reg) { return pin_reg; }

  static inline void toggle(volatile uint8_t &port, uint8_t pin) {
    if (pin == ALL) {
      port ^= 0xFF;
    } else {
      port ^= (1 << pin);
    }
  }
};

static GPIO_Driver GPIO __attribute__((unused));

#endif // ATMEGA32A_GPIO_HPP
