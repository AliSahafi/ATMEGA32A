# ATmega32A Driver — build & flash
#
#   make                     build + flash main.cpp (and set ext-xtal fuses)
#   make gpio                build + flash drivers/gpio/example.cpp
#   make build SRC=file.cpp  compile + hex only, no flashing
#
# Fuse targets: int-1mhz, int-2mhz, int-4mhz, int-8mhz, ext-xtal

SRC ?= main.cpp
MODULES = gpio

.PHONY: main build flash $(MODULES) int-1mhz int-2mhz int-4mhz int-8mhz ext-xtal

main: build flash
	avrdude -c usbasp -p m32 -B 8 -U lfuse:w:0xff:m -U hfuse:w:0xd9:m

build:
	avr-g++ -mmcu=atmega32 -Os -Dnullptr=0 -o main.bin $(SRC)
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex

flash:
	avrdude -p m32 -c usbasp -P usb -B 8 -U flash:w:main.hex:i

$(MODULES):
	$(MAKE) build flash SRC=drivers/$@/example.cpp

int-1mhz:
	avrdude -c usbasp -p m32 -B 8 -U lfuse:w:0xe1:m -U hfuse:w:0xd9:m

int-2mhz:
	avrdude -c usbasp -p m32 -B 8 -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m

int-4mhz:
	avrdude -c usbasp -p m32 -B 8 -U lfuse:w:0xe3:m -U hfuse:w:0xd9:m

int-8mhz:
	avrdude -c usbasp -p m32 -B 8 -U lfuse:w:0xe4:m -U hfuse:w:0xd9:m

ext-xtal:
	avrdude -c usbasp -p m32 -B 8 -U lfuse:w:0xff:m -U hfuse:w:0xd9:m
