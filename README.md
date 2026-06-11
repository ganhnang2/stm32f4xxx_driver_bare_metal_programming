# STM32F4xx DRIVER DEVELOPEMENT

## Directory Structure

```text
Code_STM32/
├── build/                # Contains build outputs (.elf, object files...)
├── Code_ESP32/           # Source code for ESP32 (if used)
├── driver/               # Contains user-written driver libraries for the microcontroller
│   ├── inc/              # Contains driver header files (.h)
│   └── src/              # Contains driver source files (.c)
├── inc/                  # Contains application header files
├── src/                  # Contains application source files (projects/examples)
├── startup/              # Contains system startup code
│   └── startup_stm32f407.c
├── stm32f407.ld          # Linker script file specifying memory layout
├── Makefile.mak          # Makefile for building the project
├── Ghi chú.txt           # Personal project notes
└── README.md             # Instructions file (this file)
```
## How to use this repository

### 1. Clone the repository
Clone this project to your local machine:
```bash
git clone <repository_url>
cd <repository_directory>
```

### 2. Add your code and drivers
- **Application Code**: Create your application source files (e.g., `main.c`) and place them in the `src/` directory. Put any corresponding header files in the `inc/` directory.
- **Custom Drivers**: Place your peripheral driver source files (`.c`) in `driver/src/` and header files (`.h`) in `driver/inc/`. The Makefile will automatically compile all `.c` files found in the `driver/src/` directory.

### 3. Step-by-step compilation and flashing process
```bash
make -f Makefile.mak clean		
make -f Makefile.mak all APP_NAME=<file_name_to_run_in_src_directory> USE_SEMIHOSTING=<0 or 1>
make -f Makefile.mak flash APP_NAME=<file_name_to_run_in_src_directory> USE_SEMIHOSTING=<0 or 1>	
```

### 4. Step-by-step debugging process:
1. Open 2 terminals: Terminal 1 - OpenOCD Server, Terminal 2 - GDB Client
    - When opening terminal 1, run the following command: `openocd -f interface/stlink.cfg -f target/stm32f4x.cfg`
    - When opening terminal 2, run the following command: `arm-none-eabi-gdb build/<file_name>.elf`
2. Next, debugging operations will primarily be performed in terminal 2 - GDB Client
    - Run command: `target remote localhost:3333` : Connects GDB to the STM32 board via the OpenOCD port.
    - `monitor reset halt` : Commands the chip to reset from the beginning and immediately halt all operations, waiting for the next command.
    - `monitor arm semihosting enable` (Optional): If the user wants to use semihosting, execute this command, otherwise skip it.
    - `load` : Flashes the .elf file (already loaded into GDB) directly into the chip's Flash memory.
    - `monitor reset halt`: Resets from the beginning and jumps to the start of the main program.
3. Use commands for debugging such as: `b`, `c`, `n`, `s`, `p`, `info register`, ...

**NOTE: The "monitor" keyword is used to send commands directly to OpenOCD via GDB Commands**