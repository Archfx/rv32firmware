DIY simple firmware
==========

In the previous article, we looked at the different steps involved in the process of compiling firmware that is ready to run on actual hardware. For that, we have used the default firmware given in the [picoRV](https://github.com/YosysHQ/picorv32) repo. In this article, we write a very simple program from scratch to run on the picoRV processor.


The main program that our processor can understand is written in assembly. `start.S` contains the basic structure for this purpose. 

Understanding the assembly file.
--------

Before starting to execute a program, the processor needs to be in a known state. Therefore this `start.S` file is responsible to make that happen. There are different register configurations available in the RISC-V architecture. Based on that it will configure all the internal registers of the processor.

```c
reset_vec:
	// no more than 16 bytes here !
	picorv32_waitirq_insn(zero)
	picorv32_maskirq_insn(zero, zero)
	j start
...

/* Main program
 **********************************/
start:
	/* zero-initialize all registers */

	addi x1, zero, 0
	addi x2, zero, 0
	addi x3, zero, 0
	addi x4, zero, 0
	addi x5, zero, 0
	addi x6, zero, 0
	addi x7, zero, 0
	addi x8, zero, 0
	addi x9, zero, 0
...

```

After all the initial configurations are done we can start implementing our own code. Interestingly, if you are capable of writing code in assembly you can write the code directly at the `start.S` itself. Luckily we don't need to do that because the compiler takes care of this. In c programs, the reason that we need to have a main function is that this initial file contains the main() function definition by default. So that we don't need to worry about the assembly-level startup sequence. You can find the main definition in our case in the below lines of the `start.S`

```c
	/* set stack pointer */
	lui sp,(128*1024)>>12
	/* call main C code */
	jal ra,main
```

This will jump the code into our main() method.

Hello World on picoRV
------

Let's write a tiny program to print "hello world". Print hello world means we write the corresponding character values to the internal registers of the processor. For this, we need to select a output register port and from the string write one character at a time. The example c code will look like below.

```c
#include <stdint.h>
#include <stdbool.h>

#define OUTPORT 0x10000000

// irq.c
uint32_t *irq(uint32_t *regs, uint32_t irqs);


void print_str(const char *p)
{
	while (*p != 0)
		*((volatile uint32_t*)OUTPORT) = *(p++);
}

void main(void)
{
	print_str("hello world\n");
}

```

clone the full version of the above code from GitHub. Contents related to this tutorial are available in the folder **tinyfirmware** of the repository
```shell
git clone https://github.com/Archfx/rv32firmware
cd rv32firmware/overview
```

Now let's compile the code to get the binary. Detailed description about compiling the code is available [here](https://archfx.github.io/posts/2023/02/firmware1/)

```shell
$ riscv32-unknown-elf-gcc tiny.c irq.c print.c -c -mabi=ilp32 -march=rv32ic -Os --std=c99 -ffreestanding -nostdlib

$ riscv32-unknown-elf-gcc start.S -c -mabi=ilp32 -march=rv32ic -o start.o

$ riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o hello.elf -Wl,--build-id=none,-Bstatic,-T,sections.lds,-Map,hello.map,--strip-debug start.o tiny.o irq.o print.o -lgcc

$ riscv32-unknown-elf-objcopy hello.elf -O binary firmware.bin
```
