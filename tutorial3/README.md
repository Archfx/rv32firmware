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

1. Memory Layout - How much memory is available on our hardware. Where we can find this memory?
2. Section Definition - Where should the compiler place the functions and methods with uninitialized addresses
3. Options - Specify entry points and architecture-related information
4. Symbols - Link time variable that should be introduced to the program


Memory Layout
-------

Let's look at the memory layout that we had in the [tutorial2](https://archfx.github.io/posts/2023/02/firmware2/).
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

