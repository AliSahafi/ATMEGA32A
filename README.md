# ATMEGA32A C++ Driver

A clean, lightweight, header-only C++ driver for the **ATMEGA32A** microcontroller.  
Designed for students and educators to get an Arduino-like development experience without any external frameworks.

The driver is split into **independent modules**, organized under [`drivers/`](drivers/). Each module contains:

- `<module>.hpp` — the driver (header-only, include just what you need)
- `example.cpp` — a minimal lecture example for that peripheral (`make <module>` flashes it)
- `readme.pdf` / `readme.tex` — printable handout for students (theory, registers, API, wiring, exercises)

---

## Getting Started

### Step 1: Install the AVR Toolchain

#### Ubuntu / Debian
To install the AVR GCC toolchain, `avrdude`, and `make`:
```bash
sudo apt-get update && sudo apt-get install -y gcc-avr binutils-avr avr-libc avrdude make
```

#### macOS
Using [Homebrew](https://brew.sh/):
```bash
brew tap osx-cross/avr
brew install avr-gcc avrdude make
```

#### Windows
Using [Scoop](https://scoop.sh/) (Recommended):
```powershell
scoop install avr-gcc avrdude make
```

Or using [Chocolatey](https://chocolatey.org/):
```powershell
choco install avr-gcc avrdude make
```

> 💡 **Windows USBasp Users:** If using a USBasp programmer, Windows will not recognize it out of the box. You have two options:
> 1. **Use the bundled driver (easiest):** Extract [`USBasp-win-driver-x86-x64-v3.0.7.zip`](USBasp-win-driver-x86-x64-v3.0.7.zip) included in this repo and run `InstallDriver.exe` (matching your CPU architecture).
> 2. **Use Zadig:** Download [Zadig](https://zadig.akeo.ie/), plug in your USBasp, and replace its driver with `libusb-win32`.

---

### Step 2: VS Code Setup (Recommended)

1. Download and install [Visual Studio Code](https://code.visualstudio.com/).
2. When you open this folder in VS Code, install the recommended extensions (or search for them in the Extensions tab `Ctrl+Shift+X` / `Cmd+Shift+X`):
   - **C/C++** (`ms-vscode.cpptools`) — for code navigation, syntax highlighting, and auto-completion.
   - **Makefile Tools** (`ms-vscode.makefile-tools`) — for Makefile syntax and integration.
3. **IntelliSense is pre-configured:** The included `.vscode/c_cpp_properties.json` ensures that AVR-specific headers (`<avr/io.h>`, `<util/delay.h>`, etc.) resolve cleanly without red squiggly error marks.
4. **One-Click Build & Flash:** Press `Ctrl+Shift+B` (or `Cmd+Shift+B` on macOS) to build and flash directly from within VS Code.

---

### Step 3: Build & Flash (Command Line)

Navigate to the project directory and run:
```bash
make            # compile and flash main.cpp
```

Or flash the GPIO lecture example:
```bash
make gpio       # flash drivers/gpio/example.cpp (button + LED)
```

To compile without flashing:
```bash
make build SRC=path/to/file.cpp
```

---

## Modules

| Module | Status | Capabilities |
|---|---|---|
| [**GPIO**](drivers/gpio/) | **Active** | `OUTPUT`, `INPUT`, `INPUT_PULLUP` — per-pin or whole port at once |

Shared constants and `F_CPU` handling live in [`drivers/common/`](drivers/common/) and are included automatically.

---

## Driver Reference

### Setup

Include only the modules you use — unused peripherals then cost no flash and no interrupt vectors:

```cpp
#define F_CPU 8000000UL     // Define BEFORE including any driver!
#include "drivers/gpio/gpio.hpp"
```

> ⚠️ If `F_CPU` is not defined, the driver defaults to 8MHz and shows a compiler warning.

---

### GPIO

| Method | Description |
|---|---|
| `GPIO.setDirection(DDRx, pin, OUTPUT)` | Set single pin direction (`INPUT`, `OUTPUT`, `INPUT_PULLUP`) |
| `GPIO.setDirection(DDRx, ALL, INPUT_PULLUP)` | Set whole port direction |
| `GPIO.write(PORTx, pin, HIGH/LOW)` | Write single pin |
| `GPIO.write(PORTx, ALL, 0xF0)` | Write raw byte to whole port |
| `GPIO.write(PORTx, 0xF0)` | Shorthand whole-port write |
| `GPIO.read(PINx, pin)` | Read single pin — returns `HIGH` or `LOW` |
| `GPIO.read(PINx, ALL)` | Read whole port — returns 0–255 |
| `GPIO.read(PINx)` | Shorthand whole-port read |
| `GPIO.toggle(PORTx, pin)` | Toggle single pin |
| `GPIO.toggle(PORTx, ALL)` | Toggle whole port |

**Constants:** `INPUT`, `OUTPUT`, `INPUT_PULLUP`, `HIGH`, `LOW`, `ALL`

#### Quick Example
```cpp
#define F_CPU 8000000UL
#include "drivers/gpio/gpio.hpp"

int main() {
  GPIO.setDirection(DDRD, PD2, INPUT_PULLUP); // Button input with pull-up
  GPIO.setDirection(DDRC, PC0, OUTPUT);       // LED output

  while (true) {
    if (GPIO.read(PIND, PD2) == LOW) {
      GPIO.write(PORTC, PC0, HIGH);           // Turn ON when button pressed
    } else {
      GPIO.write(PORTC, PC0, LOW);
    }
  }
}
```

See [`drivers/gpio/example.cpp`](drivers/gpio/example.cpp) for a full runnable example and [`drivers/gpio/readme.pdf`](drivers/gpio/readme.pdf) for the student handout.

---

## Fuse Configuration (USBasp)

| Target | Description |
|---|---|
| `make ext-xtal` | External crystal (default: 8–16 MHz) |
| `make int-8mhz` | Internal RC oscillator @ 8 MHz |
| `make int-4mhz` | Internal RC oscillator @ 4 MHz |
| `make int-2mhz` | Internal RC oscillator @ 2 MHz |
| `make int-1mhz` | Internal RC oscillator @ 1 MHz (factory default) |
