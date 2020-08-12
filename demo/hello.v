`timescale 1ns / 1ps
`default_nettype none

module main	(
		input wire CLOCK_50,            //On Board 50 MHz
        input wire [9:0] SW,            // On board Switches
		input wire [3:0] KEY,           // On board push buttons
        output wire [(8*6) -1:0] HEX,   // HEX displays
        output reg [9:0] LED,           // LEDs
		output wire [7:0] x,            // VGA pixel coordinates
        output wire [6:0] y,
        output wire [2:0] colour,       // VGA pixel colour (0-7)
        output wire plot,               // Pixel drawn when this is pulsed
        output wire vga_resetn          // VGA reset to black when this is pulsed
);    
    //Gray code thing
    always @(*) LED[9] <= SW[9];
    
    genvar i;
    generate for(i = 0; i < 9; i = i + 1) begin
        always @(SW[i], LED[i+1]) LED[i] <= LED[i+1] ^ SW[i];
    end endgenerate
    
    
    //Shift register for rotating segment demo
    reg [15:0] pattern = 16'h7FFF;
    
    //Slow down the clock a little
    reg [9:0] slow_clk = 0;
    
    always @(posedge CLOCK_50) begin
        if (slow_clk == 999) begin
            pattern <= {pattern[0], pattern[15:1]};
            slow_clk <= 0;
        end else begin
            slow_clk <= slow_clk + 1;
        end
    end
    
    function automatic [31:0] idx;
    input [2:0] digit;
    input [15:0] segment;
    //local variables
    reg   [2:0] seg_off;
    begin
        case (segment)
        "T": seg_off = 0;
        "TR": seg_off = 1;
        "BR": seg_off = 2;
        "B": seg_off = 3;
        "BL": seg_off = 4;
        "TL": seg_off = 5;
        "C": seg_off = 6;
        default: seg_off = 7;
        endcase
        idx = {(29'd0+digit), seg_off};
    end
    endfunction
    
    //Assign pattern to correct hex segments (this is guaranteed to be wrong,
    //but whatever)
    assign HEX[idx(0,"T")] = pattern[0];  
    assign HEX[idx(1,"T")] = pattern[1]; 
    assign HEX[idx(2,"T")] = pattern[2];  
    assign HEX[idx(3,"T")] = pattern[3];  
    assign HEX[idx(4,"T")] = pattern[4];  
    assign HEX[idx(5,"T")] = pattern[5];  
    assign HEX[idx(5,"TR")] = pattern[6]; 
    assign HEX[idx(5,"BR")] = pattern[7]; 
    assign HEX[idx(5,"B")] = pattern[8];  
    assign HEX[idx(4,"B")] = pattern[9];  
    assign HEX[idx(3,"B")] = pattern[10]; 
    assign HEX[idx(2,"B")] = pattern[11]; 
    assign HEX[idx(1,"B")] = pattern[12]; 
    assign HEX[idx(0,"B")] = pattern[13]; 
    assign HEX[idx(0,"BL")] = pattern[14];
    assign HEX[idx(0,"TL")] = pattern[15];
    
    //Clear out everything else except middle 4 segments
    assign HEX[idx(0,"TR")] = 1;
    assign HEX[idx(0,"BR")] = 1;
    assign HEX[idx(0,"C")] = 1;
    assign HEX[idx(0,"D")] = SW[0];
    generate for (i = 1; i < 5; i = i + 1) begin
        assign HEX[idx(i,"TR")] = 1;
        assign HEX[idx(i,"BR")] = 1;
        assign HEX[idx(i,"TL")] = 1;
        assign HEX[idx(i,"BL")] = 1;
        assign HEX[idx(i,"C")] = KEY[4 - i];
        assign HEX[idx(i,"D")] = 1;
    end endgenerate
    assign HEX[idx(5,"TL")] = 1;
    assign HEX[idx(5,"BL")] = 1;
    assign HEX[idx(5,"C")] = 1;
    assign HEX[idx(5,"D")] = 1;
    
    
    //Nice VGA demo
    
    reg [7:0] x_i = 0;
    reg [6:0] y_i = 0;
    reg [2:0] col_i = 3'd1;
    
    always @(posedge CLOCK_50) begin
        if (y_i == 119) begin
            y_i <= 0;
            if (x_i == 159) begin 
                x_i <= 0;
            end else begin
                x_i <= x_i + 1;
            end
        end else begin 
            y_i <= y_i + 1;
        end
    
        if (col_i < 7) begin
            col_i <= col_i + 1;
        end else begin
            col_i <= 3'd1;
        end
    end
    
    assign x = x_i;
    assign y = y_i;
    assign colour = col_i;
    assign plot = 1;
    assign vga_resetn = 1;
endmodule
