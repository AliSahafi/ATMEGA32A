# ATMEGA32A C++ Driver

A clean, lightweight, header-only C++ driver for the **ATMEGA32A** microcontroller.  
Designed for students and educators to get an Arduino-like development experience without any external frameworks.

The driver is split into **independent modules**, organized under [`drivers/`](drivers/). Each module contains:

- `<module>.hpp` — the driver (header-only, include just what you need)
- `example.cpp` — a minimal lecture example for that peripheral (`make <module>` flashes it)
- `readme.pdf` / `readme.tex` — printable handout for students (theory, registers, API, wiring, exercises)

---

## Getting Started

### Step 1: Clone or Download the Repository

You can obtain the project repository in either of two ways:

- **Option A — Clone with Git (Recommended, Optional):**  
  Installing Git is optional, but strongly recommended. It allows you to easily pull future course updates, bug fixes, and new driver modules as they are released using `git pull`:
  ```bash
  git clone https://github.com/AliSahafi/ATMEGA32A.git
  cd ATMEGA32A
  ```
- **Option B — Download ZIP:**  
  If you prefer not to use Git, download the repository as a ZIP archive from the green **Code** button on the [GitHub repository](https://github.com/AliSahafi/ATMEGA32A), then extract it to your working folder.

---

### Step 2: Install the AVR Toolchain & Git (Optional)

#### Ubuntu / Debian
To install the AVR GCC toolchain, `avrdude`, `make`, and optionally `git`:
```bash
sudo apt-get update && sudo apt-get install -y gcc-avr binutils-avr avr-libc avrdude make git
```
*(You can omit `git` if you do not plan to use Git).*

#### macOS
Using [Homebrew](https://brew.sh/):
```bash
brew tap osx-cross/avr
brew install avr-gcc avrdude make git
```
*(You can omit `git` if you do not plan to use Git).*

#### Windows
Using [Scoop](https://scoop.sh/) (Recommended):
```powershell
scoop install avr-gcc avrdude make git
```

Or using [Chocolatey](https://chocolatey.org/):
```powershell
choco install avr-gcc avrdude make git
```

> 💡 **Git on Windows:** Alternatively, you can download the official Git installer from [git-scm.com](https://git-scm.com/).
> 
> 💡 **Windows USBasp Users:** If using a USBasp programmer, Windows will not recognize it out of the box. You have two options:
> 1. **Use the bundled driver (easiest):** Extract [`USBasp-win-driver-x86-x64-v3.0.7.zip`](USBasp-win-driver-x86-x64-v3.0.7.zip) included in this repo and run `InstallDriver.exe` (matching your CPU architecture).
> 2. **Use Zadig:** Download [Zadig](https://zadig.akeo.ie/), plug in your USBasp, and replace its driver with `libusb-win32`.

---

### Step 3: VS Code Setup (Recommended)

1. Download and install [Visual Studio Code](https://code.visualstudio.com/).
2. When you open this folder in VS Code, install the recommended extensions (or search for them in the Extensions tab `Ctrl+Shift+X` / `Cmd+Shift+X`):
   - **C/C++** (`ms-vscode.cpptools`) — for code navigation, syntax highlighting, and auto-completion.
   - **Makefile Tools** (`ms-vscode.makefile-tools`) — for Makefile syntax and integration.
3. **IntelliSense is pre-configured:** The included `.vscode/c_cpp_properties.json` ensures that AVR-specific headers (`<avr/io.h>`, `<util/delay.h>`, etc.) resolve cleanly without red squiggly error marks.
4. **One-Click Build & Flash:** Press `Ctrl+Shift+B` (or `Cmd+Shift+B` on macOS) to build and flash directly from within VS Code.
5. *(Optional)* **Source Control:** If Git is installed, VS Code's built-in Source Control tab (`Ctrl+Shift+G` / `Cmd+Shift+G`) lets you pull updates and view changes with a single click.

---

### Step 4: Build & Flash (Command Line)

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

> 💡 **Beginner-Friendly:** You can pass `PORTx`, `PINx`, or `DDRx` to any function interchangeably (e.g. `GPIO.read(PORTC, PC0)` or `GPIO.read(PINC, PC0)`). The driver automatically selects the correct hardware register under the hood.

| Method | Description |
|---|---|
| `GPIO.setDirection(PORTx, pin, OUTPUT)` | Set single pin direction (`INPUT`, `OUTPUT`, `INPUT_PULLUP`) |
| `GPIO.setDirection(PORTx, ALL, INPUT_PULLUP)` | Set whole port direction |
| `GPIO.write(PORTx, pin, HIGH/LOW)` | Write single pin |
| `GPIO.write(PORTx, ALL, 0xF0)` | Write raw byte to whole port |
| `GPIO.write(PORTx, 0xF0)` | Shorthand whole-port write |
| `GPIO.read(PORTx, pin)` | Read single pin — returns `HIGH` or `LOW` (`PINx` also accepted) |
| `GPIO.read(PORTx, ALL)` | Read whole port — returns 0–255 |
| `GPIO.read(PORTx)` | Shorthand whole-port read |
| `GPIO.toggle(PORTx, pin)` | Toggle single pin |
| `GPIO.toggle(PORTx, ALL)` | Toggle whole port |

**Constants:** `INPUT`, `OUTPUT`, `INPUT_PULLUP`, `HIGH`, `LOW`, `ALL`

#### Quick Example
```cpp
#define F_CPU 8000000UL
#include "drivers/gpio/gpio.hpp"

int main() {
  // Push button S11 on PD2, LED D0 on PB0 (active-low: LOW = ON)
  GPIO.setDirection(PORTD, PD2, INPUT_PULLUP);
  GPIO.setDirection(PORTB, PB0, OUTPUT);

  while (true) {
    if (GPIO.read(PORTD, PD2) == LOW) {
      GPIO.write(PORTB, PB0, LOW);  // Turn ON when button pressed
    } else {
      GPIO.write(PORTB, PB0, HIGH); // Turn OFF when released
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
