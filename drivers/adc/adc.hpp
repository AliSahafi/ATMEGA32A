/*
 * ATmega32A Driver — ADC Module (Analog-to-Digital Converter)
 * Developed by Ali Sahafi <ali.sahafi@gmail.com> with help from Claude AI.
 *
 * Full usage guide: readme.pdf (this folder). Worked example: example.cpp.
 *
 * Quick usage (call these on the shared `ADC` object below):
 *
 *   ADC.enable();             // switch the ADC on (it does not measure yet):
 *                             // reference = AREF (5 V on the course board),
 *                             // ADC clock 50..200 kHz, chosen from F_CPU
 *   ADC.disable();            // switch the ADC off (saves power)
 *
 *   ADC.read(0);              // channel 0..7 = PA0..PA7 -> 0..1023 (10-bit)
 *   ADC.read8(0);             // -> 0..255 (upper 8 bits), for LEDs and PWM
 *   ADC.readMillivolts(0);    // -> 0..4995 mV
 *
 *   ADC.onComplete(myFunction);  // interrupt: myFunction(value) is called
 *   ADC.start(0);                // when this conversion is finished;
 *                                // start() does not wait (interrupt lecture)
 *
 * Every read waits for one conversion (about 100 us at 8 MHz). The driver
 * makes the pin an input without pull-up for you. While the ADC is off
 * (before enable() or after disable()), every read returns 0.
 *
 * The potentiometer of the course board is on PA0 (channel 0).
 * On the course board VCC, AVCC and AREF are all connected to +5 V, so the
 * driver always uses AREF as the reference (never the internal 2.56 V).
 */

#ifndef ATMEGA32A_ADC_HPP
#define ATMEGA32A_ADC_HPP

#include "../common/common.hpp"

// <avr/io.h> uses the name ADC for the 16-bit result register. We need the
// name for the driver object; the register is still available as ADCW.
#undef ADC

// Reference voltage in millivolts (AREF pin). 5 V on the course board.
#ifndef ADC_VREF_MV
#define ADC_VREF_MV 5000UL
#endif

// ---------------------------------------------------------
// ADC Driver
// ---------------------------------------------------------
class ADC_Driver {
public:
  // Switch the ADC on: reference = AREF, result right-adjusted (10-bit),
  // ADC clock = F_CPU / prescaler, the smallest prescaler that gives
  // 200 kHz or less (8 MHz / 64 = 125 kHz).
  static inline void enable() {
    ADMUX = 0; // REFS1:0 = 00 -> AREF, ADLAR = 0, channel 0
    ADCSRA = (1 << ADEN) | (ADCSRA & (1 << ADIE)) | prescalerBits(); // keep ADIE
  }

  // Switch the ADC off (ADEN = 0). It then uses no power. Call enable() to
  // switch it on again.
  static inline void disable() { ADCSRA &= ~(1 << ADEN); }

  // Measure the voltage on PA<channel> and return 0..1023.
  // value = Vin * 1024 / Vref  (0 V -> 0, 2.5 V -> 512, 5 V -> 1023)
  // Returns 0 if the ADC is off (a conversion can not start then).
  static inline uint16_t read(uint8_t channel) {
    if (!(ADCSRA & (1 << ADEN)))
      return 0;
    channel &= 0x07;
    DDRA &= ~(1 << channel);  // pin = input
    PORTA &= ~(1 << channel); // no pull-up (it would change the voltage)
    ADMUX = (ADMUX & 0xE0) | channel; // keep REFS1:0 and ADLAR, select channel
    ADCSRA |= (1 << ADSC);            // start one conversion
    while (ADCSRA & (1 << ADSC))      // ADSC goes back to 0 when it is done
      ;
    return ADCW; // reads ADCL first, then ADCH
  }

  // Start one conversion on PA<channel> and return at once (no waiting).
  // When it is finished, the function given to onComplete() gets the value.
  // Does nothing while the ADC is off.
  static inline void start(uint8_t channel) {
    if (!(ADCSRA & (1 << ADEN)))
      return;
    channel &= 0x07;
    DDRA &= ~(1 << channel);
    PORTA &= ~(1 << channel);
    ADMUX = (ADMUX & 0xE0) | channel;
    ADCSRA |= (1 << ADSC);
  }

  // Interrupts (covered in the interrupt lecture): call `callback(value)`
  // (value = 0..1023) when a conversion is finished. Enables global
  // interrupts (sei). Pass nullptr to switch it off. Use either read() or
  // start() + onComplete(), not both at the same time.
  static void (*completeCallback)(uint16_t);
  static inline void onComplete(void (*callback)(uint16_t)) {
    completeCallback = callback;
    if (callback) ADCSRA |= (1 << ADIE);
    else ADCSRA &= ~(1 << ADIE);
    sei();
  }

  // Same measurement, only the upper 8 bits: 0..255.
  // (The same as ADLAR = 1 and reading only ADCH.)
  static inline uint8_t read8(uint8_t channel) { return read(channel) >> 2; }

  // Same measurement in millivolts: value * 5000 / 1024 -> 0..4995 mV.
  static inline uint16_t readMillivolts(uint8_t channel) {
    return (uint32_t)read(channel) * ADC_VREF_MV / 1024UL;
  }

private:
  // ADPS2:0 for the smallest division factor with F_CPU / factor <= 200 kHz
  // (bits 1 -> /2, 2 -> /4, ... 7 -> /128).
  static inline uint8_t prescalerBits() {
    uint8_t bits = 1;
    uint32_t factor = 2;
    while (F_CPU / factor > 200000UL && bits < 7) {
      factor *= 2;
      bits++;
    }
    return bits;
  }
};

__attribute__((weak)) void (*ADC_Driver::completeCallback)(uint16_t) = nullptr;

static ADC_Driver ADC __attribute__((unused));

// Conversion-complete interrupt: only does something after onComplete()
ISR(ADC_vect) {
  if (ADC.completeCallback) ADC.completeCallback(ADCW);
}

#endif // ATMEGA32A_ADC_HPP
