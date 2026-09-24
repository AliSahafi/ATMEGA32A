/*
 * ATmega32A Driver — Timer Module (Timer0 / Timer1 / Timer2)
 * Developed by Ali Sahafi <ali.sahafi@gmail.com> with help from Claude AI.
 *
 * Full usage guide: readme.pdf (this folder). Worked example: example.cpp.
 *
 * Quick usage (Timer0, Timer1, Timer2 are ready-made objects):
 *
 *   Timer0.begin(mode, prescaler);           // start the timer
 *   Timer0.begin(mode, prescaler, pinMode);  // ...and let it drive its OC pin
 *
 *     mode      : TIMER_NORMAL    count 0..MAX, then overflow and wrap to 0
 *                 TIMER_CTC       count 0..compare value, then restart at 0
 *                 TIMER_FAST_PWM  count 0..MAX, OC pin produces a PWM signal
 *
 *     prescaler : the clock divider itself -- write the number you want:
 *                 Timer0, Timer1 : 1, 8, 64, 256, 1024
 *                 Timer2         : 1, 8, 32, 64, 128, 256, 1024
 *                 (Timer0/Timer1 also accept EXT_FALLING / EXT_RISING to
 *                  count pulses on pin T0 = PB0 / T1 = PB1 instead)
 *
 *     pinMode   : what the OC pin does (optional, default OC_OFF)
 *                 NORMAL / CTC : OC_OFF, OC_TOGGLE, OC_CLEAR, OC_SET
 *                 FAST_PWM     : OC_OFF, PWM_NON_INVERTING, PWM_INVERTING
 *                 OC pins: Timer0 = PB3, Timer1 = PD5 (A) / PD4 (B),
 *                          Timer2 = PD7. They become outputs automatically.
 *
 *   Timer0.setCount(156);          // TCNTn  -- preload the counter
 *   Timer0.getCount();             // TCNTn  -- read the counter
 *   Timer0.setCompare(100);        // OCRn   -- compare value / PWM level
 *   Timer0.setDutyCycle(25);       // PWM only: 0..100 percent
 *   Timer0.overflowed();           // TOVn   -- true once per overflow
 *   Timer0.compareMatched();       // OCFn   -- true once per compare match
 *   Timer0.stop();  Timer0.start();
 *
 *   Timer0.onOverflow(myFunction);      // optional: call a function on every
 *   Timer0.onCompareMatch(myFunction);  // overflow / compare match (interrupt)
 *
 * Timer1 is 16-bit (counts up to 65535) and has two OC pins (A and B), so it
 * has setCompareA/B, setDutyCycleA/B, compareMatchedA/B and begin() takes a
 * second pinMode for channel B. See readme.pdf for the details.
 */

#ifndef ATMEGA32A_TIMER_HPP
#define ATMEGA32A_TIMER_HPP

#include "../common/common.hpp"

// ---------------------------------------------------------
// Options
// ---------------------------------------------------------
// Modes of operation
#define TIMER_NORMAL 0
#define TIMER_CTC 1
#define TIMER_FAST_PWM 2

// What the OC pin does (value = the COMn1:COMn0 bits in the datasheet)
#define OC_OFF 0            // pin is a normal GPIO pin
#define OC_TOGGLE 1         // NORMAL/CTC: toggle pin on compare match
#define OC_CLEAR 2          // NORMAL/CTC: drive pin LOW on compare match
#define OC_SET 3            // NORMAL/CTC: drive pin HIGH on compare match
#define PWM_NON_INVERTING 2 // FAST_PWM: HIGH from 0 until compare match
#define PWM_INVERTING 3     // FAST_PWM: LOW  from 0 until compare match

// External clock source (count pulses on T0 = PB0 / T1 = PB1)
#define EXT_FALLING 0xFFFE
#define EXT_RISING 0xFFFF

// ---------------------------------------------------------
// Helpers shared by all three timers
// ---------------------------------------------------------
namespace timer_detail {
// Translate a divider (1, 8, 64, ...) into the 3 clock-select (CS) bits.
// Returns 0 (= timer stopped) for a value the timer does not support.
static inline uint8_t csBits01(uint16_t prescaler) { // Timer0 / Timer1
  switch (prescaler) {
  case 1: return 1;
  case 8: return 2;
  case 64: return 3;
  case 256: return 4;
  case 1024: return 5;
  case EXT_FALLING: return 6;
  case EXT_RISING: return 7;
  default: return 0;
  }
}

static inline uint8_t csBits2(uint16_t prescaler) { // Timer2
  switch (prescaler) {
  case 1: return 1;
  case 8: return 2;
  case 32: return 3;
  case 64: return 4;
  case 128: return 5;
  case 256: return 6;
  case 1024: return 7;
  default: return 0;
  }
}

// Duty cycle 0..100 % -> compare value for an 8-bit (TOP = 255) PWM.
// In both PWM modes the pin is "active" for (compare + 1) / 256 of the
// period: HIGH for PWM_NON_INVERTING, LOW for PWM_INVERTING.
static inline uint16_t dutyToCompare(uint8_t percent, uint16_t top) {
  if (percent > 100) percent = 100;
  return (uint16_t)(((uint32_t)percent * (top + 1UL)) / 100UL) - 1;
}
} // namespace timer_detail

// ---------------------------------------------------------
// Timer0 and Timer2 (8-bit) -- same features, different registers
// ---------------------------------------------------------
template <uint8_t N> class Timer8_Driver {
private:
  // Register lookup: N = 0 -> Timer0, N = 2 -> Timer2
  static inline volatile uint8_t &TCCR() { return N == 0 ? TCCR0 : TCCR2; }
  static inline volatile uint8_t &TCNT() { return N == 0 ? TCNT0 : TCNT2; }
  static inline volatile uint8_t &OCR() { return N == 0 ? OCR0 : OCR2; }
  static inline uint8_t TOV() { return N == 0 ? TOV0 : TOV2; }
  static inline uint8_t OCF() { return N == 0 ? OCF0 : OCF2; }
  static inline uint8_t TOIE() { return N == 0 ? TOIE0 : TOIE2; }
  static inline uint8_t OCIE() { return N == 0 ? OCIE0 : OCIE2; }
  static inline uint8_t ocBit() { return N == 0 ? PB3 : PD7; }
  static inline volatile uint8_t &ocDdr() { return N == 0 ? DDRB : DDRD; }
  static inline volatile uint8_t &ocPort() { return N == 0 ? PORTB : PORTD; }

  static inline void setOutputMode(uint8_t pinMode) {
    TCCR() = (TCCR() & ~0x30) | ((pinMode & 0x03) << 4); // COMn1:COMn0
  }

public:
  static uint8_t mode, pinMode, cs;
  static void (*overflowCallback)();
  static void (*compareCallback)();

  // Configure and start the timer. See the top of this file for options.
  static inline void begin(uint8_t timerMode, uint16_t prescaler,
                           uint8_t ocPinMode = OC_OFF) {
    mode = timerMode;
    pinMode = ocPinMode;
    cs = (N == 0) ? timer_detail::csBits01(prescaler)
                  : timer_detail::csBits2(prescaler);

    uint8_t wgm = 0;                                  // TIMER_NORMAL
    if (mode == TIMER_CTC) wgm = (1 << 3);            // WGMn1
    else if (mode == TIMER_FAST_PWM) wgm = (1 << 6) | (1 << 3); // WGMn0+WGMn1

    if (pinMode != OC_OFF) ocDdr() |= (1 << ocBit()); // OC pin -> output

    TCNT() = 0;
    TIFR = (1 << TOV()) | (1 << OCF()); // clear old flags (write 1 to clear)
    TCCR() = wgm | ((pinMode & 0x03) << 4) | cs;
  }

  // Counter value (TCNTn): preload it, or read how far it has counted.
  static inline void setCount(uint8_t value) { TCNT() = value; }
  static inline uint8_t getCount() { return TCNT(); }

  // Compare value (OCRn): CTC "count-to" value, or PWM level (0..255).
  // In PWM mode this also re-connects the pin after setDutyCycle(0).
  static inline void setCompare(uint8_t value) {
    OCR() = value;
    if (mode == TIMER_FAST_PWM) setOutputMode(pinMode);
  }
  static inline uint8_t getCompare() { return OCR(); }

  // PWM only: percent (0..100) of the period the pin is ACTIVE --
  // HIGH for PWM_NON_INVERTING, LOW for PWM_INVERTING.
  // (Active-low LEDs: use PWM_INVERTING so duty cycle = brightness.)
  static inline void setDutyCycle(uint8_t percent) {
    if (percent == 0) {
      // The hardware can't reach a true 0 %, so park the pin instead.
      setOutputMode(OC_OFF);
      if (pinMode == PWM_INVERTING) ocPort() |= (1 << ocBit());
      else ocPort() &= ~(1 << ocBit());
      return;
    }
    OCR() = (uint8_t)timer_detail::dutyToCompare(percent, 255);
    setOutputMode(pinMode);
  }

  // Flags: return true once per event and clear the flag automatically,
  // so a simple `while (!Timer0.overflowed());` waits for the next one.
  static inline bool overflowed() {
    if (TIFR & (1 << TOV())) {
      TIFR = (1 << TOV()); // write 1 to clear
      return true;
    }
    return false;
  }
  static inline bool compareMatched() {
    if (TIFR & (1 << OCF())) {
      TIFR = (1 << OCF());
      return true;
    }
    return false;
  }

  // Pause / resume counting (the configuration is kept).
  static inline void stop() { TCCR() &= ~0x07; }
  static inline void start() { TCCR() = (TCCR() & ~0x07) | cs; }

  // Optional interrupts: call `callback` on every overflow / compare match.
  // Enables global interrupts (sei). Pass nullptr to switch it off again.
  // Callbacks run in interrupt context: keep them short, and declare
  // variables shared with main() as `volatile`.
  static inline void onOverflow(void (*callback)()) {
    overflowCallback = callback;
    if (callback) TIMSK |= (1 << TOIE());
    else TIMSK &= ~(1 << TOIE());
    sei();
  }
  static inline void onCompareMatch(void (*callback)()) {
    compareCallback = callback;
    if (callback) TIMSK |= (1 << OCIE());
    else TIMSK &= ~(1 << OCIE());
    sei();
  }
};

template <uint8_t N> uint8_t Timer8_Driver<N>::mode = TIMER_NORMAL;
template <uint8_t N> uint8_t Timer8_Driver<N>::pinMode = OC_OFF;
template <uint8_t N> uint8_t Timer8_Driver<N>::cs = 0;
template <uint8_t N> void (*Timer8_Driver<N>::overflowCallback)() = nullptr;
template <uint8_t N> void (*Timer8_Driver<N>::compareCallback)() = nullptr;

// ---------------------------------------------------------
// Timer1 (16-bit, two OC pins: A = PD5, B = PD4)
// ---------------------------------------------------------
// In TIMER_FAST_PWM mode the PWM period is set by setTop() (ICR1), default
// 255 -- so by default it behaves exactly like the 8-bit timers. A larger
// top gives finer steps and a slower PWM (e.g. 20 ms for a servo).
class Timer1_Driver {
private:
  static inline void setOutputModes(uint8_t a, uint8_t b) {
    TCCR1A = (TCCR1A & 0x0F) | ((a & 0x03) << 6) | ((b & 0x03) << 4);
  }
  static inline void setDutyCycle(uint8_t percent, volatile uint16_t &ocr,
                                  uint8_t pin, bool channelA) {
    uint8_t pm = channelA ? pinModeA : pinModeB;
    uint8_t a = channelA ? OC_OFF : currentA();
    uint8_t b = channelA ? currentB() : OC_OFF;
    if (percent == 0) {
      setOutputModes(a, b);
      if (pm == PWM_INVERTING) PORTD |= (1 << pin);
      else PORTD &= ~(1 << pin);
      return;
    }
    ocr = timer_detail::dutyToCompare(percent, ICR1);
    if (channelA) a = pm;
    else b = pm;
    setOutputModes(a, b);
  }
  static inline uint8_t currentA() { return (TCCR1A >> 6) & 0x03; }
  static inline uint8_t currentB() { return (TCCR1A >> 4) & 0x03; }

public:
  static uint8_t mode, pinModeA, pinModeB, cs;
  static void (*overflowCallback)();
  static void (*compareCallbackA)();
  static void (*compareCallbackB)();

  // Configure and start Timer1. pinModeA drives OC1A (PD5), pinModeB OC1B (PD4).
  // CTC counts up to the channel-A compare value (setCompareA).
  static inline void begin(uint8_t timerMode, uint16_t prescaler,
                           uint8_t ocPinModeA = OC_OFF,
                           uint8_t ocPinModeB = OC_OFF) {
    mode = timerMode;
    pinModeA = ocPinModeA;
    pinModeB = ocPinModeB;
    cs = timer_detail::csBits01(prescaler);

    uint8_t wgmA = 0, wgmB = 0;                     // TIMER_NORMAL (mode 0)
    if (mode == TIMER_CTC) wgmB = (1 << WGM12);     // mode 4, TOP = OCR1A
    else if (mode == TIMER_FAST_PWM) {              // mode 14, TOP = ICR1
      wgmA = (1 << WGM11);
      wgmB = (1 << WGM13) | (1 << WGM12);
      ICR1 = 255;
    }

    if (pinModeA != OC_OFF) DDRD |= (1 << PD5);
    if (pinModeB != OC_OFF) DDRD |= (1 << PD4);

    TCCR1B = 0; // stop while configuring
    TCNT1 = 0;
    TIFR = (1 << TOV1) | (1 << OCF1A) | (1 << OCF1B);
    TCCR1A = wgmA | ((pinModeA & 0x03) << 6) | ((pinModeB & 0x03) << 4);
    TCCR1B = wgmB | cs;
  }

  static inline void setCount(uint16_t value) { TCNT1 = value; }
  static inline uint16_t getCount() { return TCNT1; }

  static inline void setCompareA(uint16_t value) {
    OCR1A = value;
    if (mode == TIMER_FAST_PWM) setOutputModes(pinModeA, currentB());
  }
  static inline void setCompareB(uint16_t value) {
    OCR1B = value;
    if (mode == TIMER_FAST_PWM) setOutputModes(currentA(), pinModeB);
  }
  // Shorthand for channel A, so Timer1 reads like Timer0/Timer2.
  static inline void setCompare(uint16_t value) { setCompareA(value); }

  // PWM period in counts (FAST_PWM only): period = (top + 1) * prescaler / F_CPU
  static inline void setTop(uint16_t top) { ICR1 = top; }

  static inline void setDutyCycleA(uint8_t percent) {
    setDutyCycle(percent, OCR1A, PD5, true);
  }
  static inline void setDutyCycleB(uint8_t percent) {
    setDutyCycle(percent, OCR1B, PD4, false);
  }
  static inline void setDutyCycle(uint8_t percent) { setDutyCycleA(percent); }

  static inline bool overflowed() {
    if (TIFR & (1 << TOV1)) {
      TIFR = (1 << TOV1);
      return true;
    }
    return false;
  }
  static inline bool compareMatchedA() {
    if (TIFR & (1 << OCF1A)) {
      TIFR = (1 << OCF1A);
      return true;
    }
    return false;
  }
  static inline bool compareMatchedB() {
    if (TIFR & (1 << OCF1B)) {
      TIFR = (1 << OCF1B);
      return true;
    }
    return false;
  }
  static inline bool compareMatched() { return compareMatchedA(); }

  static inline void stop() { TCCR1B &= ~0x07; }
  static inline void start() { TCCR1B = (TCCR1B & ~0x07) | cs; }

  static inline void onOverflow(void (*callback)()) {
    overflowCallback = callback;
    if (callback) TIMSK |= (1 << TOIE1);
    else TIMSK &= ~(1 << TOIE1);
    sei();
  }
  static inline void onCompareMatchA(void (*callback)()) {
    compareCallbackA = callback;
    if (callback) TIMSK |= (1 << OCIE1A);
    else TIMSK &= ~(1 << OCIE1A);
    sei();
  }
  static inline void onCompareMatchB(void (*callback)()) {
    compareCallbackB = callback;
    if (callback) TIMSK |= (1 << OCIE1B);
    else TIMSK &= ~(1 << OCIE1B);
    sei();
  }
  static inline void onCompareMatch(void (*callback)()) {
    onCompareMatchA(callback);
  }
};

__attribute__((weak)) uint8_t Timer1_Driver::mode = TIMER_NORMAL;
__attribute__((weak)) uint8_t Timer1_Driver::pinModeA = OC_OFF;
__attribute__((weak)) uint8_t Timer1_Driver::pinModeB = OC_OFF;
__attribute__((weak)) uint8_t Timer1_Driver::cs = 0;
__attribute__((weak)) void (*Timer1_Driver::overflowCallback)() = nullptr;
__attribute__((weak)) void (*Timer1_Driver::compareCallbackA)() = nullptr;
__attribute__((weak)) void (*Timer1_Driver::compareCallbackB)() = nullptr;

static Timer8_Driver<0> Timer0 __attribute__((unused));
static Timer1_Driver Timer1 __attribute__((unused));
static Timer8_Driver<2> Timer2 __attribute__((unused));

// ---------------------------------------------------------
// Interrupt service routines (only do something after onOverflow /
// onCompareMatch has been called)
// ---------------------------------------------------------
ISR(TIMER0_OVF_vect) { if (Timer0.overflowCallback) Timer0.overflowCallback(); }
ISR(TIMER0_COMP_vect) { if (Timer0.compareCallback) Timer0.compareCallback(); }
ISR(TIMER1_OVF_vect) { if (Timer1.overflowCallback) Timer1.overflowCallback(); }
ISR(TIMER1_COMPA_vect) { if (Timer1.compareCallbackA) Timer1.compareCallbackA(); }
ISR(TIMER1_COMPB_vect) { if (Timer1.compareCallbackB) Timer1.compareCallbackB(); }
ISR(TIMER2_OVF_vect) { if (Timer2.overflowCallback) Timer2.overflowCallback(); }
ISR(TIMER2_COMP_vect) { if (Timer2.compareCallback) Timer2.compareCallback(); }

#endif // ATMEGA32A_TIMER_HPP
