Build a Linker Script
========

In this episode of the firmware tutorial, let us build a firmware linker script from scratch for picoRV processor. You may remember in the previous tutorials we have used `sections.lds` to get the elf file. This linker file contains information about how much memory is available in our system and where and where specific hardware components use the memory so that the compiler can place different functions appropriately. 

As an example, during the previous [hello world program](https://archfx.github.io/posts/2023/02/firmware2/), we had the below line in the `firmware.c` file which we asked the compiler to put values to the specific memory address. 

```c
#define reg_uart_data (*(volatile uint32_t*)0x02000008)
```
which means all the communications related to specific hardware components should write and read through specific registers in the memory.

Linker
-----


To generate one binary file to be run on the processor, Linker requires compiled object file(s).  During the previous step of generating the machine code (object file) from the c or assembly files, the compiler keeps the placeholder for each of the methods that we had in the source files. Linker will look at the Linker script and assign appropriate addresses. In more layman's terms, the Linker script is the blueprint of the entire firmware and we fill in with different function calls or methods.

Let's see some examples from the previous tutorial

If we look at the assigned addresses of the firmware object file main() and putchar() methods don't have initialized addresses. print_str() method has a temporary address since it has a relative connection with main().
```shell
$ riscv32-unknown-elf-nm firmware.o
00000000 T main
00000026 T print_str
00000000 T putchar
```

Now let's observe the function addresses after the linking process is completed where each function has its own address now. Assembly file `start.s` has also further introduced more processor routine functions into the final firmware. 

```shell
$ riscv32-unknown-elf-nm firmware.elf 
...
0010005a t loop_init_data
0010013e T main
00100122 T print_str
00100102 T putchar
0010003e t setmemloop
00100000 t start

```

Linker also has the ability to generate debug information, program optimization, garbage collection, etc.

Linker Script Components
========

There are four components that we need to identify when talking about the Linker script

1. Memory Layout - How much memory is available on our hardware? Where we can find this memory?
2. Section Definition - Where should the compiler place the functions and methods with uninitialized addresses
3. Options - Specify entry points and architecture-related information
4. Symbols - Link time variable that should be introduced to the program


Memory Layout
-------

Let's look at the memory layout that we had in [tutorial2](https://archfx.github.io/posts/2023/02/firmware2/).
```c
MEMORY
{
    FLASH (rx) : ORIGIN = 0x00100000, LENGTH = 0x400000
    RAM (xrw) : ORIGIN = 0x00000000, LENGTH = 0x20000
}
```

Summary of the above memory layout is as below,

| Memory Type | Starting address | Size  |
| ----------- | ---------------- | ----- |
| FLASH       | 0x00100000       | 512Kb |
| RAM         | 0x00000000       | 16Kb  |

This information should be extracted from your hardware specification. For the picoRV processor-based SoC above information can be found [here](https://github.com/YosysHQ/picorv32/tree/master/picosoc).

More details about Linker script command syntaxes are available in [binutils](https://sourceware.org/binutils/docs/ld/MEMORY.html#:~:text=The%20syntax%20for%20MEMORY%20is,outside%20of%20the%20linker%20script.).

The general syntax for writing the memory layout is as follows,

```shell
MEMORY
  {
    name [(attr)] : ORIGIN = origin, LENGTH = len
    …
  }
```
"name" attribute does not carry any meaning outside the linker script. attributes string should contain the following characters `r` (read only), `w` (read/write section), `x` (executable section), `a` (allocatable section), `i/l` (initialized section) or `!` with any of the above attributes to sense the invert.

Sections
------

Next, we have to define sections. The intention is to separate the code from data and put them in contiguous areas of memory. Usually, there is no correct way of naming each of the sections. But as the [best practice](https://refspecs.linuxbase.org/elf/elf.pdf) sections are named as follows,

1. `.text` for code and constants
2. `.bss` for uninitialized data
3. `.stack` or `.heap` for our stack/heap
5. `.data` for initialized data 

### Empty Section

Let's start with an empty section as follows,

```shell
MEMORY
{
    FLASH (rx) : ORIGIN = 0x00100000, LENGTH = 0x400000
    RAM (xrw) : ORIGIN = 0x00000000, LENGTH = 0x20000
}
SECTIONS {
    /* empty! */
}
```

This will throw a bunch of errors about missing references for the functions in the `start.s` file. Let's perform an `object_dump` on the object files of both the assembly file and the c code to see the available sections. 

```shell
$ riscv32-unknown-elf-objdump -h start.o 

start.o:     file format elf32-littleriscv

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         0000011c  00000000  00000000  00000034  2**2
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  1 .data         00000000  00000000  00000000  00000150  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  2 .bss          00000000  00000000  00000000  00000150  2**0
                  ALLOC
  3 .riscv.attributes 0000001f  00000000  00000000  00000150  2**0
                  CONTENTS, READONLY
```

```shell
$ riscv32-unknown-elf-objdump -h firmware.o

firmware.o:     file format elf32-littleriscv

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         00000048  00000000  00000000  00000034  2**1
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  1 .data         00000000  00000000  00000000  0000007c  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  2 .bss          00000000  00000000  00000000  0000007c  2**0
                  ALLOC
  3 .rodata.str1.4 0000000d  00000000  00000000  0000007c  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  4 .text.startup 0000001a  00000000  00000000  0000008a  2**1
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  5 .comment      0000001c  00000000  00000000  000000a4  2**0
                  CONTENTS, READONLY
  6 .riscv.attributes 00000021  00000000  00000000  000000c0  2**0
                  CONTENTS, READONLY
```


Now let's connect the dots with SECTIONS part of the original linker file that we had.

```c
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
    } >FLASH
    .data : AT ( _sidata )
    {
        . = ALIGN(4);
        _sdata = .;
        _ram_start = .;
        . = ALIGN(4);
        *(.data)
        *(.data*)
        *(.sdata)
        *(.sdata*)
        . = ALIGN(4);
        _edata = .;
    } >RAM
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(.sbss)
        *(.sbss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >RAM
    .heap :
    {
        . = ALIGN(4);
        _heap_start = .;
    } >RAM
}
```

More information is available at [bitutils](https://sourceware.org/binutils/docs/ld/).