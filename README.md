rv32firmware
========

This is a simple tutorial based on the sample program provided for [picoRV processors](https://github.com/YosysHQ/picorv32) for beginners.

Find the full post [here](https://archfx.github.io/posts/2023/02/firmware1/)](https://archfx.github.io/posts/2023/02/firmware1/)


Firmware at Bare metal
=======

In this tutorial, we are going to look at writing firmware for an embedded hardware device. This tutorial is solely focused on simulations. However, it can be synthesized to work on FPGAs.

We are using the following setups,

1. [picoRV](https://github.com/YosysHQ/picorv32) processor as the hardware 
2. [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) to compile the firmware
3. [Iverilog](https://iverilog.fandom.com/wiki/Main_Page) for simulation


Firmware is usually written with c and assembly. Additionally, a program needs to have a memory map so that it can map the firmware to the correct places to start the boot process properly.

sample c program to run on our setup
```cpp
#include "firmware.h"
void hello(void)
{
	print_str("hello world\n");
}
```


Do we need to have a main {  } function here?  Actually where to start in a c code should be acknowledged by the programmer to the processor. This should be implemented in assembly language.

sample assembly code
```c
reset_vec:
	// no more than 16 bytes here !
	picorv32_waitirq_insn(zero)
	picorv32_maskirq_insn(zero, zero)
	j start


/#ifdef ENABLE_HELLO
	/* set stack pointer */
	lui sp,(128*1024)>>12

	/* call hello C code */
	jal ra,hello
#endif

```

Although we have multiple objects like different c programs multiple assembly programs, we need to compile them into one single program. For that, we need to have a linker script.

sample memory map
```c
MEMORY {
	/* the memory in the testbench is 128k in size;
	 * set LENGTH=96k and leave at least 32k for stack */
	mem : ORIGIN = 0x00000000, LENGTH = 0x00018000
}

SECTIONS {
	.memory : {
		. = 0x000000;
		start*(.text);
		*(.text);
		*(*);
		end = .;
		. = ALIGN(4);
	} > mem
}
```

## Now let's start with the real deal by identifying each step

clone the full version of the above code from GitHub
```shell
git clone https://github.com/Archfx/rv32firmware
```

Setting up the toolchain
-------

Instead of setting up the toolchain in your local machine you can use the following docker container and mount the firmware directory to it

```shell
docker pull archfx/rv32i
docker run -t -p 6080:6080 -v "${PWD}/:/rv32firmware" -w /rv32firmware --name rv32i archfx/rv32i
```

This will get you to a docker container with the toolchain. Now all we got to do is compile the firmware

Compiling the Code
-------

producing the output file

1. compiling the c files to machine code
```shell
/opt/riscv32i/bin/riscv32-unknown-elf-gcc *.c -c -mabi=ilp32 -march=rv32ic -Os --std=c99 -ffreestanding -nostdlib
```

2. Compiling the assembly files to machine code
```shell
/opt/riscv32i/bin/riscv32-unknown-elf-gcc start.S -c -mabi=ilp32 -march=rv32ic -o start.o
```

3. Using the linker to link with the machine code to a single program
```shell
/opt/riscv32i/bin/riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o firmware.elf -Wl,--build-id=none,-Bstatic,-T,sections.lds,-Map,firmware.map,--strip-debug start.o irq.o print.o hello.o sieve.o stats.o -lgcc
```

4. Generate the binary from .elf
```shell
/opt/riscv32i/bin/riscv32-unknown-elf-objcopy firmware.elf -O binary firmware.bin
```

Now outside the docker container, we can have to generate the binary file
5. Generate the binary file
```shell
python makehex.py firmware.bin 920 firmware.hex
```


Simulating the Firmware in picoRV
---------

Here comes the fun part. Now we have the compiled firmware which can be run on the actual hardware. Instead, we are going to simulate the firmware on the hardware implementation as a simulation. For this, we are using Iverilog simulator. [Iverilog](https://iverilog.fandom.com/wiki/Main_Page) is an open-source compiled simulator, therefore this process involves two simple steps. First, we need to compile the hardware design combined with the firmware. Then run the compiled simulation. 


1. Let's connect the firmware to the hardware implementation. For that will modify the test bench
```shell
cd hw
nano testbench.v
```
and find the following lines on the test bench. Modify the firmware file location to the correct firmware that you just compiled.

```verilog
	reg [1023:0] firmware_file;
	initial begin
		if (!$value$plusargs("firmware=%s", firmware_file))
			firmware_file = "../firmware.hex";
		$readmemh(firmware_file, mem.memory);
	end
```

2. Now let's simulate. First, we compile the design
```shell
 iverilog testbench.v picorv32.v  -o picorv.vvp
```

3. Finally, run the simulation
```shell
vvp picorv.vvp +vcd +trace
```

This will produce the below output in the terminal
```shell
VCD info: dumpfile testbench.vcd opened for output.
RD: ADDR=00000000 DATA=0800400b INSN
RD: ADDR=00000004 DATA=0600600b INSN
RD: ADDR=00000008 DATA=0013a6e9 INSN
RD: ADDR=00000010 DATA=0200a10b INSN
RD: ADDR=00000014 DATA=0201218b INSN
RD: ADDR=00000018 DATA=15200093 INSN
RD: ADDR=0000001c DATA=0000410b INSN
RD: ADDR=00000020 DATA=0020a023 INSN
RD: ADDR=00000024 DATA=0001410b INSN
WR: ADDR=00000150 DATA=00000008 STRB=1111
Finished writing testbench.trace.
testbench.v:51: $finish called at 83240000 (1ps)
```

You can observe the internal signal patterns by looking at the testbench.trace and testbench.vcd files.

In the next post let us look at how to write firmware in a detailed manner. 