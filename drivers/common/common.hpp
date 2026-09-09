/*
 * ATmega32A Driver — Common Definitions
 * Developed by Ali Sahafi <ali.sahafi@gmail.com> with help from Claude AI.
 *
 * Shared by every driver module. Include a module header (e.g.
 * "../gpio/gpio.hpp") and this file comes with it automatically.
 */

#ifndef ATMEGA32A_COMMON_HPP
#define ATMEGA32A_COMMON_HPP

#ifndef __AVR_ATmega32A__
#define __AVR_ATmega32A__
#endif

#ifndef F_CPU
#warning                                                                       \
    "F_CPU not defined! Please define it before including this driver or in your Makefile. Defaulting to 8MHz (8000000UL)."
#define F_CPU 8000000UL // Default to 8MHz if not defined
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

// Arduino-like constants
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define LOW 0
#define HIGH 1
#define ALL 0xFF

#endif // ATMEGA32A_COMMON_HPP
