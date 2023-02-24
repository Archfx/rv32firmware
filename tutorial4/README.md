Bootloader at Home?
========

So far we looked at implementing firmware for bare metal use case. But you may have seen that in most of the hardware SDK's they suggest you use a bootloader to load your programs.

- [atmega328p bootloader](https://www.circuito.io/blog/atmega328p-bootloader/)
- [STM32 bootloader](https://akospasztor.github.io/stm32-bootloader/)
- [ESP32 bootloader](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/bootloader.html)

Bootloader is a simple program, which can load and start your application. The simplest analogy is that the bootloader has a placeholder for your application to boot successfully inside the processor once you update it in the correct place. This could be either from an external flash or EEPROM. Bootloader's job is to copy the content from the read-only memory (ROM) to the executable memory (RAM).

Bootladers can isolate critical components from each other and security primitives such as confidentiality, integrity, and authenticity can be introduced to the firmware with the use of bootloaders. Bootloaders also support recovery from bad application updates.

Simple Bootloader
------

In this tutorial, we will write a simple bootloader to work with our [picoRV](https://github.com/YosysHQ/picorv32) processor. This bootloader will start during the power-up of the processor and will make the program counter jump into the application code. Finally, we will simulate the design with our homemade bootloader with the picoRV processor.

### Memory Map

We need to make sure that the bootloader is isolated from the application. The easiest way to achieve this is via the linker script. Note that the bootloader is a small program and does not need much space. Let's give it about 4kB of memory. 

```c
MEMORY
{
    BOOTROM (rx) : ORIGIN = 0x00100000, LENGTH = 0x004000
    APPROM (rx) : ORIGIN = 0x00104000, LENGTH = 0x3fc000
    RAM (xrw) : ORIGIN = 0x00000000, LENGTH = 0x20000
}

__bootrom_start__ = ORIGIN(BOOTROM);
__bootrom_size__ = LENGTH(BOOTROM);
__approm_start__ = ORIGIN(APPROM);
__approm_size__ = LENGTH(APPROM);
```

Bootloader needs to find the application. Therefore we have to share this information with the bootloader We can simply use a header file for that.

```c
/* memory_map.h */
#pragma once

extern int __bootrom_start__;
extern int __bootrom_size__;
extern int __approm_start__;
extern int __approm_size__;
```

### Bootloader Code


```c
/* bootloader.c */
#include <inttypes.h>
#include "memory_map.h"

int main(void) {
  uint32_t *app_code = (uint32_t *)__approm_start__;
  uint32_t app_sp = app_code[0];
  uint32_t app_start = app_code[1];
  /* TODO: Start app */
  /* Not Reached */
  while (1) {}
}
```

Next from the bootloader, we need to set the program counter to the app starting address. For this, we will use assembly instructions. Specifically, since we are using the RISC-V based picoRV core we need to use the correct jump instruction from the RISC-V ISA.


```c
/* bootloader.c */
#include <inttypes.h>
#include "memory_map.h"

static void start_app(uint32_t pc, uint32_t sp) __attribute__((naked)) {
    __asm("           \n\
          la x2, a1 /* load a1 into SP */\n\
          jr a0       /* jump to the address at a0 */\n\
    ");
}

int main(void) {
  uint32_t *app_code = (uint32_t *)__approm_start__;
  uint32_t app_sp = app_code[0];
  uint32_t app_start = app_code[1];
  start_app(app_start, app_sp);
  /* Not Reached */
  while (1) {}
}

```

More information about writing assembly with c can be found from [Extended-Asm](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html).

### Compile Bootloader

Next, we compile the bootloader and app separately. Let's compile the bootloader first.

```shell
riscv32-unknown-elf-gcc bootloader.c -c -mabi=ilp32 -march=rv32ic -Os --std=c99 -ffreestanding -nostdlib

riscv32-unknown-elf-gcc start.S -c -mabi=ilp32 -march=rv32ic -o start.o

riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o boot.elf -Wl,--build-id=none,-Bstatic,-T,boot.lds,-Map,boot.map,--strip-debug bootloader.o start.o -lgcc

```
Then let's compile the application. For this, we can use the simple firmware that we used in [tutorial2](https://archfx.github.io/posts/2023/02/firmware2/) 
```shell
riscv32-unknown-elf-gcc bootload-app.c -c -mabi=ilp32 -march=rv32ic -Os --std=c99 -ffreestanding -nostdlib

riscv32-unknown-elf-gcc start.S -c -mabi=ilp32 -march=rv32ic -o start.o

riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o app.elf -Wl,--build-id=none,-Bstatic,-T,app.lds,-Map,app.map,--strip-debug bootload-app.o start.o -lgcc

```

Next, we have to combine the two elf files into one bin/hex file. For that we use `objcopy`.

```shell
riscv32-unknown-elf-objcopy boot.elf --pad-to=0x4000 --gap-fill=0xFF -O verilog  boot.hex

riscv32-unknown-elf-objcopy firmware.elf -O verilog  firmware.hex

cat bootloader-16.hex firmware.hex > bootv.hex
```