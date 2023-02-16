// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

`timescale 1 ns / 1 ps


module testbench #(
	parameter FIRMWARE_FILE = "../fw/firmware.hex",
    parameter ROM_ADDR_BITS = 12, // 4K ROM
    parameter RAM_ADDR_BITS = 12, // 4K RAM
    parameter FRAM_ADDR_BITS = 12 // 4k FRAM
);

	reg clk = 1;
	reg resetn = 0;

	always #5 clk = ~clk;

	initial begin
		repeat (100) @(posedge clk);
		resetn <= 1;
	end

	initial begin
		if ($test$plusargs("vcd")) begin
			$dumpfile("testbench.vcd");
			$dumpvars(0, testbench);
		end
		repeat (100000) @(posedge clk);
		$display("TIMEOUT");
		$finish;
	end




	soc #(
		.FIRMWARE_FILE (FIRMWARE_FILE),
		.ROM_ADDR_BITS (ROM_ADDR_BITS),
		.RAM_ADDR_BITS (RAM_ADDR_BITS),
		.FRAM_ADDR_BITS (FRAM_ADDR_BITS)
	) top (
		.clk(clk),
		.resetn(resetn)
	);
endmodule

