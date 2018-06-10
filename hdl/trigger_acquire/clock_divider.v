`timescale 1ns / 1ps
module clock_divider # (parameter WIDTH = 16)(
    input clk_in,
	input reset,
	output clk_out
);
	reg [WIDTH-1:0] counter;
	
	always @(posedge clk_in) begin
		if (reset)
			counter <= 0;
		else
			counter <= counter + 1;
	end

	assign clk_out = ~(counter[WIDTH-1]);

endmodule