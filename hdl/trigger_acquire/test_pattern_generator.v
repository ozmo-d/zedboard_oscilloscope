/*
Test pattern generator.

Use two clock dividers. The first will operate at our 'sample' frequency.


*/

`timescale 1 ns / 1 ps
module test_pattern_generator
(
input clk,
input reset,
output [31:0] data_out,
output data_valid
);

wire clk_div1, clk_div2;

clock_divider #(.WIDTH(4)) clock_divider1(
	.clk_in(clk),
	.reset(reset),
	.clk_out(clk_div1)
);

clock_divider #(.WIDTH(2)) clock_divider2 (
	.clk_in(clk_div1),
	.reset(reset),
	.clk_out(clk_div2)
);

wire [15:0] count1, count2;

counter #(.WIDTH(16)) counter1 (
    .clk(clk_div1),
	.reset(reset),
	.en(1),
	.count(count1)
);

counter #(.WIDTH(16)) counter2 (
    .clk(clk_div2),
	.reset(reset),
	.en(1),
	.count(count2)
);

reg clk_div1_delay;

always @(posedge clk) begin
	clk_div1_delay <= clk_div1;
end

assign data_valid = (clk_div1_delay == 0) && (clk_div1 == 1);
assign data_out = {count1, count2};

endmodule