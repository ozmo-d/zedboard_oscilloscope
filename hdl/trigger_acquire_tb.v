///////////////////////////////////////////////////////////////////////////////
//    
//    Company:          Xilinx
//    Engineer:         Jim Tatsukawa
//    Date:             2/11/2014
//    Design Name:      ug480
//    Module Name:      ug480_tb.v
//    Version:          2.1
//    Target Devices:   7 Series Family
//    Tool versions:    2014.1
//    Description:      This is a basic demonstration of the XADC 
// 
//    Disclaimer:  XILINX IS PROVIDING THIS DESIGN, CODE, OR
//                 INFORMATION "AS IS" SOLELY FOR USE IN DEVELOPING
//                 PROGRAMS AND SOLUTIONS FOR XILINX DEVICES.  BY
//                 PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
//                 ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
//                 APPLICATION OR STANDARD, XILINX IS MAKING NO
//                 REPRESENTATION THAT THIS IMPLEMENTATION IS FREE
//                 FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE
//                 RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY
//                 REQUIRE FOR YOUR IMPLEMENTATION.  XILINX
//                 EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH
//                 RESPECT TO THE ADEQUACY OF THE IMPLEMENTATION,
//                 INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
//                 REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
//                 FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES
//                 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//                 PURPOSE.
// 
//                 (c) Copyright 2013-2014 Xilinx, Inc.
//                 All rights reserved.
// 
///////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps
module trigger_acquire_tb;
reg             RESET; 
reg             DCLK;
initial 
    begin 
                DCLK = 0;
                RESET = 0;
                #100 RESET = 1;
                #50 RESET = 0;
    end
always #(10) DCLK= ~DCLK; // 100MHz

wire [31:0] test_data_out;
wire test_data_valid;
test_pattern_generator dut
(
.clk(DCLK),
.reset(RESET),
.data_out(test_data_out),
.data_valid(test_data_valid)
);

endmodule