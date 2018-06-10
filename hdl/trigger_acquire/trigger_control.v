`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Engineer: Wesley Kendall
// 
// Create Date: 02/22/2018 06:15:39 PM
// Design Name: trigger_acquire
// Module Name: trigger_control
//
// Description: 
//   Simple combinational logic which handles the triggering of the trigger acquire system.
//   Can detect a rising or falling voltage edge. Can also fire a trigger immediately.
//   Will later be controlled by registers.
//////////////////////////////////////////////////////////////////////////////////
module trigger_control
#(
    parameter FIFO_ADDR_WIDTH = 10, 
    parameter C_M_AXIS_TDATA_WIDTH = 32,
    parameter FIFO_DEPTH = 1 << FIFO_ADDR_WIDTH
)
(
    // register inputs
    input [FIFO_ADDR_WIDTH-1:0] reg_post_trigger_count,
	input [15:0] reg_trig_value,
	input reg_trig_enable,
	input reg_trig_polarity, // 0 for rising. 1 for falling.
	input reg_trig_immediate,
    input reg_trig_channel,
    	
	// xadc / datapath inputs
	input [31:0] data_in,
	input data_in_valid,
	
	// debug signals
	output wire [1:0] state_out,
	output wire trigger_out,
	
	// ram out signals
	output wire ram_wr_en,
	output wire [FIFO_ADDR_WIDTH-1:0] ram_addr,

	
	// axi ports
    input wire  M_AXIS_ACLK,
    input wire  M_AXIS_ARESETN,
    output wire  M_AXIS_TVALID,
    output wire [C_M_AXIS_TDATA_WIDTH-1 : 0] M_AXIS_TDATA,
    output wire  M_AXIS_TLAST,
    input wire  M_AXIS_TREADY
    );
    
	
	parameter ACTIVE=3'b00, SET=3'b01, TRIGD=3'b10, AXI_STREAM=3'b11;
	reg [1:0] state = ACTIVE;
  
	reg [FIFO_ADDR_WIDTH-1:0] post_counter = 0;
	wire post_count_complete;

	wire [15:0]trig_channel;
	wire trig_compare, trig_edge;
	wire triggered; // this signal consolidates all of the possible ways we can trigger.
	wire axi_stream_done;
	reg [FIFO_ADDR_WIDTH-1:0] wr_ptr, rd_ptr;
    reg [C_M_AXIS_TDATA_WIDTH-1:0] ram [0:FIFO_DEPTH-1]/*synthesis syn_ramstyle = "block_ram" */;
    reg [C_M_AXIS_TDATA_WIDTH-1:0] tdata;

	
	reg [FIFO_ADDR_WIDTH-1:0] axi_stream_counter = 0;
    // implementation of trigger system
	// We have an edge trigger, which needs to be set up by selecting channel, trig value, polarity, and enable.
	// We also have an immediate trigger.
	// The final output "triggered" will move us to the AXI_STREAM state.
	
	reg prev_trig_compare = 1;
	assign trig_channel = reg_trig_channel ? data_in[31:16] : data_in[15:0]; // select channel
	assign trig_compare = ((trig_channel < reg_trig_value) ^ (~reg_trig_polarity)); // check if we are above the threshold
	assign trig_edge = (trig_compare && ~prev_trig_compare); // detect the edge
	assign triggered = (trig_edge | reg_trig_immediate); // (* mark_debug = "true" *)
	
	assign post_count_complete = (post_counter==reg_post_trigger_count);

	always @(posedge M_AXIS_ACLK) begin
	   if (~M_AXIS_ARESETN)
	       prev_trig_compare <= 1'b1;
	   else begin
	       if (state == SET)
            prev_trig_compare <= trig_compare;
       end
    end
	
	// state machine
    always @(posedge M_AXIS_ACLK)
    if (~M_AXIS_ARESETN) begin
        rd_ptr <= 0;
        state <= ACTIVE;
        post_counter <= 0;
        axi_stream_counter <= 0;
        end
    else 
	begin
        case (state)
			ACTIVE:
			begin
				post_counter <= 0;
				if (reg_trig_enable) begin
					state <= SET;
				end
			end
			SET: 
			begin
				if (triggered) begin 
					state <= TRIGD;
				end
			end
			TRIGD: 
			begin
			   if (post_count_complete) begin
				   state <= AXI_STREAM;
				   rd_ptr <= (wr_ptr + 1); // we are now pointing to the oldest piece of data.
			   end
			   else if (data_in_valid) begin
				   post_counter <= post_counter + 1;
			   end                
			end
			AXI_STREAM:
			begin
				tdata <= ram[rd_ptr];
				if (reg_trig_enable) begin
					state <= SET;
				end
				else if (axi_stream_done) begin
					state <= ACTIVE;
					axi_stream_counter <= 0;
				end
				else if (packet_sent) begin
					axi_stream_counter <= axi_stream_counter + 1;
					rd_ptr <= rd_ptr + 1;
				end
			end
		endcase
	end
        
    wire wr_en;
    assign wr_en = ((state == ACTIVE) || (state == SET) || (state == TRIGD)) && data_in_valid;


    always @(posedge M_AXIS_ACLK)
    if (~M_AXIS_ARESETN) begin
        wr_ptr <= 0;
    end
    else begin
    if (wr_en) begin
        ram[wr_ptr] <= data_in;
        wr_ptr <= wr_ptr + 1;
        end
    end
  
    wire tvalid, packet_sent;
	reg tvalid_delay;
    assign tvalid = (state==AXI_STREAM);
	
	always @(posedge M_AXIS_ACLK) begin
		tvalid_delay <= tvalid;
	end
	
    assign packet_sent = M_AXIS_TREADY && tvalid_delay;

    assign last = &(axi_stream_counter);
	assign axi_stream_done = last && (state == AXI_STREAM);
	
     // AXI output
    assign M_AXIS_TVALID = tvalid_delay && (state==AXI_STREAM);
    assign M_AXIS_TDATA  = tdata;
    assign M_AXIS_TLAST  = last && tvalid_delay; // TLAST indicates the boundary of a packet.

	assign trigger_out = triggered;
	assign state_out = state;
	assign ram_wr_en = wr_en;
	assign ram_addr = wr_ptr;

endmodule