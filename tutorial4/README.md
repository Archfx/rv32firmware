Bootloader at Home?
========

So far we looked at implementing firmware for bare metal use cases. But you may have seen that in most of the hardware SDK's, they suggest using a bootloader to load your programs.

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
/* memory_map.lds */
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

Let's create a simple c program for the bootloader. In order to visually see our bootloader is working we can add some print statements to the UART. In this case, we have added the characters of the word `Boot`.

```c
/* boot.c */
#include <inttypes.h>
#include "memory_map.h"


int main(void) {
  (*(volatile uint32_t*)0x02000004) = 104; // Set UART clock rate
  (*(volatile uint32_t*)0x02000008) = 0x42; // Write B to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write o to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write o to UART
  (*(volatile uint32_t*)0x02000008) = 0x74; // Write t to UART

   /* Do some bootloader stuff. */
 
  while (1) {}
}

```

Next from the bootloader, we need to set the program counter to the app starting address. For this, we will use assembly instructions. Specifically, since we are using the RISC-V based picoRV core we need to use the correct jump instruction from the RISC-V ISA. In this case the we need to jump to the app starting point with `jal ra, __approm_start__`


```c
/* boot.c */
#include <inttypes.h>
#include "memory_map.h"


int main(void) {
  (*(volatile uint32_t*)0x02000004) = 104; // Set UART clock rate
  (*(volatile uint32_t*)0x02000008) = 0x42; // Write B to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write o to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write o to UART
  (*(volatile uint32_t*)0x02000008) = 0x74; // Write t to UART


   /* Do some bootloader stuff. */
  
  
  uint32_t app_start = (uint32_t)&__approm_start__; // Where is the application
 
  __asm__ ("jal ra, %[prog_count]" /* __asm__ ("jal ra, 0x00104000") */
                    : /* No outputs. */
                    : [prog_count] "i" (app_start) ); // Jump  to the application
 
  while (1) {}
}

```

More information about writing assembly with c can be found in [Extended-Asm](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html).

Finally, let's create a Linker script for the bootloader informing the compiler to put out Bootloader to the BOOTROM. We can copy the Linker script from the [tutorial3](), and replace the `>`rom` with `>BOOTROM`.

```c
/* boot.lds */
INCLUDE memory_map.lds

SECTIONS {
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.srodata)
        *(.srodata*)
        . = ALIGN(4);
        _etext = .;
        _sidata = _etext;
    } >BOOTROM
    
    .....
```


### Application code

Now for the application side, we need to create a seperate Linker Script. For that, we can copy the same Linker script and replace `>rom` with `>APPROM`.
```c
/* app.lds */
INCLUDE memory_map.lds

SECTIONS {
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.srodata)
        *(.srodata*)
        . = ALIGN(4);
        _etext = .;
        _sidata = _etext;
    } >APPROM
    
    .....
```
Now we need to compile the application and the bootloader separately.


### Compile Bootloader

Next, we compile the bootloader and app separately. Let's compile the bootloader first.

```shell
$ riscv32-unknown-elf-gcc boot.c -c -mabi=ilp32 -march=rv32ic -Os --std=c99 -ffreestanding -nostdlib
$ riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o boot.elf -Wl,--build-id=none,-Bstatic,-T,boot.lds,-Map,boot.map,--strip-debug boot.o -lgcc

```
Then let's compile the application. For this, we can use the simple "hello world" application that we used in [tutorial2](https://archfx.github.io/posts/2023/02/firmware2/) 
```shell

$ riscv32-unknown-elf-gcc app.c -c -mabi=ilp32 -march=rv32ic -Os --std=c99 -ffreestanding -nostdlib
$ riscv32-unknown-elf-gcc start.S -c -mabi=ilp32 -march=rv32ic -o start.o
$ riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o app.elf -Wl,--build-id=none,-Bstatic,-T,app.lds,-Map,app.map,--strip-debug start.o app.o -lgcc
```

Next, we have to combine the two elf files into one bin/hex file. For that we use `objcopy`. Alternatively, You can simply copy the two hex files into one hex file following the order of `boot.hex` `app.hex`.

```shell
$ riscv32-unknown-elf-objcopy boot.elf --pad-to=0x4000 --gap-fill=0x00 -O verilog  boot.hex
$ riscv32-unknown-elf-objcopy app.elf -O verilog  app.hex
$ cat boot.hex app.hex > bootapp.hex
```

You can find all the code related to this tutorial in `tutorial4` folder of the following GitHub repository.

[![Archfx/rv32firmware - GitHub](https://gh-card.dev/repos/Archfx/rv32firmware.svg?fullname=)](https://github.com/Archfx/rv32firmware)


### Simulate

Now, let's see our bootloader in action.

Compile the hardware in the `hw` folder.

```shell
iverilog -s testbench -o ice.vvp  icebreaker_tb.v icebreaker.v ice40up5k_spram.v spimemio.v simpleuart.v picosoc.v picorv32.v spiflash.v -DNO_ICE40_DEFAULT_ASSIGNMENTS  `yosys-config --datdir/ice40/cells_sim.v`
```
Simulate the hardware with the bootloader and our app.

```shell
vvp -N ice.vvp ../fw/bootapp.hex +firmware=../fw/bootapp.hex
```
This should produce the following output
```shell
0000000
Serial data: 'B'
Serial data: 'o'
Serial data: 'o'
Serial data: 't'
+50000 cycles
Serial data: 'H'
Serial data: 'E'
Serial data: 'L'
Serial data: 'L'
Serial data: 'O'
+50000 cycles
Serial data: ' '
Serial data: 'W'
Serial data: 'O'
Serial data: 'R'
Serial data: 'L'
Serial data: 'D'
Serial data:  13
Serial data:  10
+50000 cycles
+50000 cycles
+50000 cycles
+50000 cycles
icebreaker_tb.v:37: $finish called at 3000000000 (1ps)
```

Wallah! We have successfully created a bootloader at home!. In future tutorials, we will discuss how to do more bootloader stuff with our simple bootloader.
