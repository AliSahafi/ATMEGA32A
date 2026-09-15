/*
 * ATmega32A Driver — Seven Segment Display Module
 * Developed by Ali Sahafi <ali.sahafi@gmail.com> with help from Claude AI.
 *
 * Full usage guide: ../common/readme.pdf (Section: Seven-Segment Display).
 *
 * Quick usage:
 *   GPIO.setDirection(PORTB, ALL, OUTPUT);  // configure PORTB as output first
 *   SevenSeg(digit);                        // digit = 0..9, writes to PORTB
 *
 * Not included automatically by other modules -- include it explicitly.
 * Drives an active-low, common-anode display wired to all 8 bits of PORTB,
 * so PORTB must not be used for anything else while this is active.
 */

#ifndef ATMEGA32A_SEVENSEG_HPP
#define ATMEGA32A_SEVENSEG_HPP

#include "common.hpp"

// ---------------------------------------------------------
// Seven Segment Display
// ---------------------------------------------------------
// Displays `digit` (0-9) on the seven-segment display wired to PORTB and
// returns the raw segment pattern written. Any value outside 0-9 falls
// back to a dash-like pattern instead of a digit.
static inline uint8_t SevenSeg(uint8_t digit) {
  uint8_t seg_value;
  switch (digit) {
  case 0:
    seg_value = 0b10100000;
    break;
  case 1:
    seg_value = 0b11110011;
    break;
  case 2:
    seg_value = 0b10010100;
    break;
  case 3:
    seg_value = 0b10010001;
    break;
  case 4:
    seg_value = 0b11000011;
    break;
  case 5:
    seg_value = 0b10001001;
    break;
  case 6:
    seg_value = 0b10001000;
    break;
  case 7:
    seg_value = 0b10110011;
    break;
  case 8:
    seg_value = 0b10000000;
    break;
  case 9:
    seg_value = 0b10000001;
    break;
  default:
    seg_value = 0b10001100;
    break;
  }
  PORTB = seg_value;
  return seg_value;
}

#endif // ATMEGA32A_SEVENSEG_HPP
