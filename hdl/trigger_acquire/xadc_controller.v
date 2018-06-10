///////////////////////////////////////////////////////////////////////////////
//    
//    Engineer:         Wesley Kendall
//    Date:             3/05/2018
//    Design Name:      trigger_acquire
//    Module Name:      xadc_controller.v
//    Description:      XADC controller for two channel simultaneous sampling,
//                      based on example by Jim Tatsukawa from Xilinx.
//                      Design uses only AUX pairs 0 and 8; handles all
//                      handshaking of the XADC module.
// 
// 
///////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps
module xadc_controller (
    input DCLK, // Clock input for DRP
    input RESET,
    input VAUXP_0, VAUXN_0, VAUXP_8, VAUXN_8, // Auxiliary analog channel inputs
    input VP, VN,
    output reg [15:0] MEASURED_AUX0, MEASURED_AUX8,
	output FIFO_EN // this will be enabled for one cycle, telling the fifo to sample both channels (this lets me use a single fifo).
    );

    wire busy;
    wire [5:0] channel;
    wire drdy;
    wire eoc;
    wire eos;
       
    reg [6:0] daddr;
    reg [15:0] di_drp;
    wire [15:0] do_drp;
    wire [15:0] vauxp_active;
    wire [15:0] vauxn_active;
    wire dclk_bufg;

    reg [1:0]  den_reg;
    reg [1:0]  dwe_reg;
    
    parameter       INIT_READ             = 8'h00, //initialize
                    READ_VAUX00           = 8'h01,
                    READ_VAUX00_WAITDRDY  = 8'h02,
                    READ_VAUX08           = 8'h03,
					READ_VAUX08_WAITDRDY  = 8'h04,
                    WRITE_FIFO            = 8'h05; // send new data to fifo0

    reg [7:0]   state = INIT_READ;
   
	//BUFG i_bufg (.I(DCLK), .O(dclk_bufg));
	assign dclk_bufg = DCLK;
   always @(posedge dclk_bufg)
      if (RESET) begin
         state   <= INIT_READ;
         den_reg <= 2'h0;
         dwe_reg <= 2'h0;
         di_drp  <= 16'h0000;
      end
      else
         case (state)
         INIT_READ : begin
            daddr <= 7'h40;
            den_reg <= 2'h2; // performing read
            if (busy == 0 ) state <= READ_VAUX00;
            end
         READ_VAUX00 : begin
            daddr   <= 7'h10;
            den_reg <= 2'h2; // performing read
            if (eos == 1) state   <=READ_VAUX00_WAITDRDY;
            end
         READ_VAUX00_WAITDRDY: 
            if (drdy ==1)  	begin
               MEASURED_AUX0 <= do_drp; 
               state <=READ_VAUX08;
               end
            else begin
               den_reg <= { 1'b0, den_reg[1] };
               dwe_reg <= { 1'b0, dwe_reg[1] };
               state <= state;
            end

         READ_VAUX08 : begin
            daddr   <= 7'h18;
            den_reg <= 2'h2; // performing read
            state   <= READ_VAUX08_WAITDRDY;
            end

         READ_VAUX08_WAITDRDY :
            if (drdy ==1)  	begin
               MEASURED_AUX8 <= do_drp; 
               state <=WRITE_FIFO;
               daddr   <= 7'h00;
            end
            else begin
               den_reg <= { 1'b0, den_reg[1] } ;
               dwe_reg <= { 1'b0, dwe_reg[1] } ;      
               state <= state;
            end

		WRITE_FIFO: begin
            state <= READ_VAUX00;
			end
			
         default : begin
            daddr <= 7'h40;
            den_reg <= 2'h2; // performing read
            state <= INIT_READ;
            end
         endcase

assign FIFO_EN = (state == WRITE_FIFO);

assign vauxp_active[0] = VAUXP_0;
assign vauxn_active[0] = VAUXN_0;

assign vauxp_active[8] = VAUXP_8;
assign vauxn_active[8] = VAUXN_8;
		 
XADC #(// Initializing the XADC Control Registers
    .INIT_40(16'h0000),// bits 0x3000 control averaging (0=none, 1=16, 2=64, 3=256)
    .INIT_41(16'h4100),// simultaneous sampling mode, all alarms disabled, calibration enabled
    .INIT_42(16'h0400),// Set DCLK to divide by 4 (100Mhz / 4) = 25MHz
    .INIT_48(16'h0800),// Sequencer register 48h is not used in simultaneous sampling
    .INIT_49(16'h0001),// We are only interested in sequence number 1 (bit 0), which is VAUX0 and VAUX8
    .SIM_MONITOR_FILE("design.txt")// Analog Stimulus file for simulation
)
XADC_INST (// Connect up instance IO. See UG480 for port descriptions
    .VN(VN),
    .VP(VP),
	.VAUXN(vauxn_active),
	.VAUXP(vauxp_active),
    .CONVST (1'b0),// not used
    .CONVSTCLK  (1'b0), // not used
    .DADDR  (daddr),
    .DCLK   (dclk_bufg),
    .DEN    (den_reg[0]),
    .DI     (di_drp),
    .DWE    (dwe_reg[0]),
    .RESET  (RESET),
    .ALM    (ALM),
    .BUSY   (busy),
    .CHANNEL(CHANNEL),
    .DO     (do_drp),
    .DRDY   (drdy),
    .EOC    (eoc),
    .EOS    (eos),
    .JTAGBUSY   (),// not used
    .JTAGLOCKED (),// not used
    .JTAGMODIFIED   (),// not used
    .OT     (OT),
    .MUXADDR    ()// not used
);

endmodule