Hello World! Firmware
========

In the previous article, we looked at the different steps involved in the process of compiling firmware that is ready to run on actual hardware. For that, we have used the default firmware given in the [picoRV](https://github.com/YosysHQ/picorv32) repo. In this article, we write a very simple program from scratch to print "HELLO WORLD" to the UART output of the picoRV processor.


The main program that our processor can understand is written in assembly. `start.S` contains the basic structure for this purpose. 

Understanding the assembly file.
--------

Before starting to execute a program, the processor needs to be in a known state. Therefore this `start.S` file is responsible to make that happen. There are different register configurations available in the RISC-V architecture. Based on that it will configure all the internal registers of the processor.

```c
.section .text

start:

# zero-initialize register file
addi x1, zero, 0
# x2 (sp) is initialized by reset
addi x3, zero, 0
addi x4, zero, 0
addi x5, zero, 0
addi x6, zero, 0
addi x7, zero, 0
addi x8, zero, 0
addi x9, zero, 0
addi x10, zero, 0
addi x11, zero, 0
addi x12, zero, 0
addi x13, zero, 0
addi x14, zero, 0
addi x15, zero, 0
addi x16, zero, 0
addi x17, zero, 0
addi x18, zero, 0
addi x19, zero, 0
...

```

After all the initial configurations are done we can start implementing our own code. Interestingly, if you are capable of writing code in assembly you can write the code directly at the `start.S` itself. Luckily we don't need to do that because the compiler takes care of this. In c programs, the reason that we need to have a main function is that this initial file contains the main() function definition by default. So that we don't need to worry about the assembly-level startup sequence. You can find the main definition in our case in the below lines of the `start.S`

```c
	addi a0, a0, 4
	blt a0, a1, loop_init_bss
	end_init_bss:

	// call main
	call main
	loop:
		j loop
```

This will jump the code into our main() method.

Hello World on picoRV
------

Let's write a tiny program to print "hello world". Print hello world means we write the corresponding character values to the internal registers of the processor. For this, we need to select an output register port and from the string write 4 bytes (a Word) at a time. The example c code will look like below.

```c
#define reg_uart_data (*(volatile uint32_t*)0x02000008)

void putchar(char c)
{
	if (c == '\n')
		putchar('\r');
	reg_uart_data = c;
}

void print_str(const char *p)
{
	while (*p)
		putchar(*(p++));
}


void main(void)
{
	(*(volatile uint32_t*)0x02000004) = 104; // Set UART clock rate
	print_str("HELLO WORLD\n");	
}
```

Here `(volatile uint32_t *)0x02000008` is dedicated memory address for the UART interface. This means we are asking the compiler to not to do any optimizations and write this value to exact location at each time. Any thing that gets written to this memory address will be communucated by the UART interface.

clone the full version of the above code from GitHub. Contents related to this tutorial are available in the folder `tutorial2` of the repository
```shell
git clone https://github.com/Archfx/rv32firmware
cd rv32firmware/tutorial2
```

Now let's compile the code to get the binary. Detailed description about compiling the code is available [here](https://archfx.github.io/posts/2023/02/firmware1/)

```shell
# get the object files from c
$ riscv32-unknown-elf-gcc firmware.c -c -mabi=ilp32 -march=rv32ic -Os --std=c99 -ffreestanding -nostdlib
# get the object file from assembly
$ riscv32-unknown-elf-gcc start.S -c -mabi=ilp32 -march=rv32ic -o start.o
# link them
$ riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o firmware.elf -Wl,--build-id=none,-Bstatic,-T,sections.lds,-Map,firmware.map,--strip-debug start.o firmware.o -lgcc
# get the hex
$ riscv32-unknown-elf-objcopy -O verilog firmware.elf firmware.hex
```

Function Addresses
-------

Let's look at the addresses assigned to each function during each stage

in object files

```shell
$ riscv32-unknown-elf-nm firmware.o
00000000 T main
00000026 T print_str
00000000 T putchar
```
Both main() and putchar() functions don't have addresses assigned yet.

Now let's observe the function addresses after the linking process is completed

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
Now we can observe that our functions now have valid addresses.

Simulation
--------

Now let's see whether our firmware is working. For that we will simulate the processor with the firmware that we just compiled. Detailed description about compiling the code is available [here](https://archfx.github.io/posts/2023/02/firmware1/).

```shell
$ cd ../hw #Navigate to hw folder
$ iverilog -s testbench -o ice.vvp  icebreaker_tb.v icebreaker.v ice40up5k_spram.v spimemio.v simpleuart.v picosoc.v picorv32.v spiflash.v -DNO_ICE40_DEFAULT_ASSIGNMENTS  cells_sim.v # complile the hardware
$ vvp -N ice.vvp ../fw/firmware.hex +firmware=../fw/firmware.hex # run the simulation
```

Wallah! UART will print hello world to the simulation output
```shell
VCD info: dumpfile testbench.vcd opened for output.
0000000
+50000 cycles
Serial data: 'H'
Serial data: 'E'
Serial data: 'L'
Serial data: 'L'
Serial data: 'O'
Serial data: ' '
Serial data: 'W'
Serial data: 'O'
Serial data: 'R'
+50000 cycles
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
