riscv32-unknown-elf-gcc boot.c -c -mabi=ilp32 -march=rv32ic -Os --std=c99 -ffreestanding -nostdlib

riscv32-unknown-elf-gcc start.S -c -mabi=ilp32 -march=rv32ic -o start.o

riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o boot.elf -Wl,--build-id=none,-Bstatic,-T,boot.lds,-Map,boot.map,--strip-debug boot.o start.o -lgcc




riscv32-unknown-elf-gcc app.c -c -mabi=ilp32 -march=rv32ic -Os --std=c99 -ffreestanding -nostdlib

# riscv32-unknown-elf-gcc start.S -c -mabi=ilp32 -march=rv32ic -o start.o

riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o app.elf -Wl,--build-id=none,-Bstatic,-T,app.lds,-Map,app.map,--strip-debug app.o start.o -lgcc

# riscv32-unknown-elf-gcc -Os -mabi=ilp32 -march=rv32imc -ffreestanding -nostdlib -o app.elf -Wl,--build-id=none,-Bstatic,-T,app.lds,-Map,app.map,--strip-debug app.o -lgcc




riscv32-unknown-elf-objcopy boot.elf --pad-to=0x4000 --gap-fill=0x00 -O verilog  boot.hex

riscv32-unknown-elf-objcopy app.elf -O verilog  app.hex

cat boot.hex app.hex > bootapp.hex
