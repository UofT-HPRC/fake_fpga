`timescale 1ns / 1ps
`default_nettype none

//This testbench is designed to hide the details of using the VPI code

module tb();

reg CLOCK_50 = 0;        // 50 MHz clock, scaled down for simulation
reg [9:0] SW = 0;        // Switches
reg [3:0] KEY = 4'b1111; // Push buttons
wire [(8*6) -1:0] HEX;   // HEX displays
wire [9:0] LED;          // LEDs
wire [7:0] x;            // VGA pixel coordinates
wire [6:0] y;
wire [2:0] colour;       // VGA pixel colour (0-7)
wire plot;               // Pixel drawn when this is pulsed
wire vga_resetn;         // VGA reset to black when this is pulsed

initial $fake_fpga(CLOCK_50, SW, KEY, LED, HEX, x, y, colour, plot, vga_resetn);

//Easiest way to generate the clock is in the verilog rather than in the VPI
//As promised, this simulates a 50 MHz clock.
always #10 CLOCK_50 <= ~CLOCK_50;

main DUT (
    .CLOCK_50(CLOCK_50),
    .SW(SW),
    .KEY(KEY),
    .HEX0(HEX[8*1 - 2 -: 7]),
    .HEX1(HEX[8*2 - 2 -: 7]),
    .HEX2(HEX[8*3 - 2 -: 7]),
    .HEX3(HEX[8*4 - 2 -: 7]),
    .HEX4(HEX[8*5 - 2 -: 7]),
    .HEX5(HEX[8*6 - 2 -: 7]),
    .LEDR(LED),
    .x(x),
    .y(y),
    .colour(colour),
    .plot(plot),
    .vga_resetn(vga_resetn)
);

genvar i;
generate for (i = 1; i <= 6; i = i + 1) begin : assign_unused
	assign HEX[8*i - 1] = 0;
end endgenerate

endmodule
