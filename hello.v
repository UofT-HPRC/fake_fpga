`timescale 1ns / 1ps

module fake_fpga(
    input wire [7:0] leds,
    output wire [7:0] buttons
);
    initial $my_task(buttons, leds);
endmodule


//Enter your code here

module main;
    reg [7:0] leds = 0;
    wire [7:0] buttons;

    always #5 leds[7] <= ~leds[7];
    //always @(*) leds[7] <= buttons[7];
    
    genvar i;
    generate for(i = 0; i < 7; i = i + 1) begin
        always @(buttons[i], leds[i+1]) leds[i] <= leds[i+1] ^ buttons[i];
    end endgenerate
    
    fake_fpga phony(leds, buttons);
endmodule
