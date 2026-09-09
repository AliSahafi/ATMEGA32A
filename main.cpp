#define F_CPU 8000000UL

// Include only the driver modules you need -- unused peripherals then cost
// no flash memory and no interrupt vectors.
#include "drivers/gpio/gpio.hpp"

int main() {

  while (true) {
  }

  return 0;
}
