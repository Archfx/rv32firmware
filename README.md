rv32firmware
========

This is a simple tutorial based on the sample program provided for picoRV processor at  https://github.com/YosysHQ/picorv32 for beginners.

Find the full post at https://archfx.github.io/posts/2023/02/firmware1/


Firmware at Bare metal
=======

In this tutorial we are going to look at writing a firmware for an embedded harware device. This tutorial is solely focused on simulations and how ever can be used with synthesized hardware designs on FPGA.

We are using following setup,

1. [picoRV](https://github.com/YosysHQ/picorv32) processor as the hardware 
2. [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) to compile the firmware


Firmware usually written with c and assembly. Additionaly program need to have a memory map so that the it can map the firmware to correct places to start the boot process properly.

sample c progrm to run on our setup
```cpp
#include "firmware.h"
void hello(void)
{
	print_str("hello world\n");
}
```

Actually do we need to have a main {  } function here.  Actually where to start in a c code should be ackknowledge to the processor. This should be implemented in assembly language.

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

Although we have multiple objects like different c programms multiple assembly programms, we need to compile them to a one single program. For that we need to have a linker script.

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

## Now lets start with the real deal with identifying each step

clone the full version of the above code from github
```shell
git clone archfx/rv32firmware
```

Setting up the toolchain
-------

Instead of setting up the tool chain in your local mashine you can use the following docker container and mounting the firmware src to it

```shell
docker pull archfx/rv32i
docker run -t -p 6080:6080 -v "${PWD}/:/rv32firmware" -w /rv32firmware --name rv32i archfx/rv32i
```

This will get you to a docker continer with the toolchain. Now all we got to do is the compile the firmware

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

3. Using the linker to link with the machine code to single program
```shell
/opt/riscv32i/bin/riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o firmware.elf -Wl,--build-id=none,-Bstatic,-T,sections.lds,-Map,firmware.map,--strip-debug start.o irq.o print.o hello.o sieve.o stats.o -lgcc
```

4. Generate the binary from elf
```shell
/opt/riscv32i/bin/riscv32-unknown-elf-objcopy firmware.elf -O binary firmware.bin
```

Now outside the docker container we can analys the binary file
5. Get the hex for analyse
```shell
python makehex.py firmware.bin 920 firmware.hex
```


Simulating the Firmware in picoRV
---------

Here comes the fun part. Now we have the compiled firmware which can be run on the actual hardware. Instead we are going to simulate the firmware on the hardware implementation as an simulation. For this we are using iverilog simulator. (Iverilog)[https://iverilog.fandom.com/wiki/Main_Page] is a opensource compiled simulator, therefore this process involves in two simple steps. First we need to compile the hardware design combined with the firmware. 

First we need to put the firmware in to the design for that we need to modify the design. For that will modify the test bench

```shell
cd hw
nano testbench.v
```
and find the following lines in the test bench. Modify the firmware file location to the correct firmware that you just compiled.

```verilog
	reg [1023:0] firmware_file;
	initial begin
		if (!$value$plusargs("firmware=%s", firmware_file))
			firmware_file = "../firmware.hex";
		$readmemh(firmware_file, mem.memory);
	end
```

Now lets simulate. First we compile the design
```shell
 iverilog testbench.v picorv32.v  -o picorv.vvp
```

Run the simulation
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

In the next post let us look at the firmware in detailed manner. 