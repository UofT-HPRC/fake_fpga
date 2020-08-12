`timescale 1ns / 1ps
`default_nettype none

//This testbench is designed to hide the details of using the VPI code

`include "hello.v"

module tb();

reg CLOCK_50 = 0;       //On Board 50 MHz
reg [9:0] SW = 0;       // On board Switches
reg [3:0] KEY = 0;      // On board push buttons
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
    .HEX(HEX),
    .LED(LED),
    .x(x),
    .y(y),
    .colour(colour),
    .plot(plot),
    .vga_resetn(vga_resetn)
);

endmodule
