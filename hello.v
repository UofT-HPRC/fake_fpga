`timescale 1ns / 1ps

module fake_fpga(
    input wire [7:0] leds,
    output wire [7:0] buttons
);
    initial $my_task(buttons, leds);
endmodule

module main;
    reg [7:0] leds = 0;
    wire [7:0] buttons;

    always @(buttons[0]) leds[0] <= buttons[0];
    
    genvar i;
    generate for(i = 1; i < 8; i = i + 1) begin
        always @(buttons[i], leds[i-1]) leds[i] <= leds[i-1] ^ buttons[i];
    end endgenerate
    
    fake_fpga phony(leds, buttons);
    
    //initial #80000 $finish;
    
endmodule
