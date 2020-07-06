`timescale 1ns / 1ps

module fake_fpga(
    input wire [7:0] leds,
    output wire [7:0] buttons
);
    initial $my_task(buttons, leds);
endmodule


//Enter your code here

module main;
    /*
    initial begin
        $dumpfile("fakefpga.vcd");
        $dumpvars;
        $dumplimit(51200);
    end
    */
    wire [7:0] leds;
    wire [7:0] buttons;

	/*
    //always #5 leds[0] <= ~leds[1];
    always @(*) leds[0] <= buttons[0];
    
    genvar i;
    generate for(i = 1; i < 8; i = i + 1) begin
        always @(buttons[i], leds[i-1]) leds[i] <= leds[i-1] ^ buttons[i];
    end endgenerate
    */

	parameter KEEP_WIDTH = 8;
	parameter NUM_LEVELS = 3; // = CLOG2(KEEP_WIDTH)

    //Wire which stores the one-hot encoding of where the last TKEEP bit
    //is situated. This is "right-aligned".
	wire [2**NUM_LEVELS -1:0] one_hot;
	genvar i;
    //Pad with zeroes up to power of two (TKEEP is not constriained to have
    //a power-of-two width)
	for (i = 2**NUM_LEVELS -1; i >= KEEP_WIDTH - 1; i = i - 1) begin
        assign one_hot[i] = 0;
    end
    
    //Use XORs between adjacent bits to find last set bit
	assign one_hot[0] = buttons[0];
	for (i = 1; i < KEEP_WIDTH - 1; i = i + 1) begin
		assign one_hot[i] = buttons[i] ^ buttons[i - 1];
	end
    
    //Subsets of bits which have a '1' in a particular position of their 
    //index. Specifically, subsets[i] is the concatenation of
    //
    //  {one_hot[j] | the (NUM_LEVELS)-bit binary representation of j has a 
    //                zero in bit i}
    //
    //(hopefully that's clear)
	wire [2**(NUM_LEVELS-1) -1:0]subsets[NUM_LEVELS -1:0];
	genvar level;
	for (level = 0; level < NUM_LEVELS; level = level + 1) begin
        //The idea is we will now sweep out the set of indices that have a
        //one in bit LEVEL of their index. 
		for (i = 0; i < 2**(NUM_LEVELS - 1); i = i + 1) begin
			wire [NUM_LEVELS - 1 -1:0] other_bits = i;
            `define left_bits ((NUM_LEVELS -1) - level)
            `define right_bits (level)
            
            wire [NUM_LEVELS -1:0] idx;
            if (`right_bits > 0) begin
                if (`left_bits > 0) begin
                    assign idx = {
                        other_bits[NUM_LEVELS -1 -1 -: `left_bits],
                        1'b0,
                        other_bits[`right_bits -1 -: `right_bits]
                    };
                end else begin
                    assign idx = {
                        1'b0,
                        other_bits[`right_bits -1 -: `right_bits]
                    };
                end
            end else begin
                //Assume it's impossible for both to be zero
                assign idx = {
                    other_bits[NUM_LEVELS -1 -1 -: `left_bits],
                    1'b0
                };
            end
            
            `undef left_bits
            `undef right_bits
            assign subsets[level][i] = one_hot[idx];
		end
	end
    
    //Finally, take the OR-reduction of the subsets to get the length value
    wire [NUM_LEVELS -1:0] len;
    for (i = 0; i < NUM_LEVELS; i = i + 1) begin
        assign len[i] = |subsets[i];
    end
    
    //And assign to the LEDs
    assign leds = len;
    
    fake_fpga phony(leds, buttons);
endmodule
