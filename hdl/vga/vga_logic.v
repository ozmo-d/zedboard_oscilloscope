`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Engineer: Wesley Kendall
// 
// Create Date: 02/01/2018 05:20:39 PM
// Design Name: vdma_test
// Module Name: vga_logic
//
// Description: Paths the input 24 bit data to 4 bit RGB lines, when active_video
// is high.
//////////////////////////////////////////////////////////////////////////////////


module vga_logic(
    input [23:0]din,
    input active_video,
    output [3:0]r,
    output [3:0]g,
    output [3:0]b
    );
    wire [23:0] rgb_streaming;
    assign rgb_streaming = active_video ? din : 0;
    assign r = rgb_streaming[23:20];
    assign g = rgb_streaming[15:12]; // am i so bad I can't do this properly? yes, i am that bad.
    assign b = rgb_streaming[7:4];
    
endmodule