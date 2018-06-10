`timescale 1ns / 1ps
module counter # (parameter WIDTH = 16)(
    input clk,
	input reset,
	input en,
	output [WIDTH-1:0] count
);
	reg [WIDTH-1:0] counter = 0;
	
	always @(posedge clk) begin
		if (reset)
			counter <= 0;
		else
			if (en) counter <= counter + 1;
	end
	assign count = counter;

endmodule