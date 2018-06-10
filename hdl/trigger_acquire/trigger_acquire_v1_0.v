
`timescale 1 ns / 1 ps

	module trigger_acquire_v1_0 #
	(
		// Users to add parameters here
        
        parameter integer FIFO_ADDR_WIDTH = 8, // This determines the depth of the FIFO (not dynamic)
        
		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface S00_AXI
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 4,
		// Parameters of Axi Master Bus Interface M00_AXIS
		parameter integer C_M00_AXIS_TDATA_WIDTH	= 32
	)
	(
		// Users to add ports here
		// XADC inputs
        input VAUXP_0, VAUXN_0, VAUXP_8, VAUXN_8, // Auxiliary analog channel inputs
        input VP, VN,
		
		input test_select, // wire to select between a test pattern and real xadc data
        
		// User ports ends
		// Do not modify the ports beyond this line

		// Ports of Axi Slave Bus Interface S00_AXI
		input wire  s00_axi_aclk,
		input wire  s00_axi_aresetn,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
		input wire [2 : 0] s00_axi_awprot,
		input wire  s00_axi_awvalid,
		output wire  s00_axi_awready,
		input wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
		input wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
		input wire  s00_axi_wvalid,
		output wire  s00_axi_wready,
		output wire [1 : 0] s00_axi_bresp,
		output wire  s00_axi_bvalid,
		input wire  s00_axi_bready,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
		input wire [2 : 0] s00_axi_arprot,
		input wire  s00_axi_arvalid,
		output wire  s00_axi_arready,
		output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
		output wire [1 : 0] s00_axi_rresp,
		output wire  s00_axi_rvalid,
		input wire  s00_axi_rready,

		// Ports of Axi Master Bus Interface M00_AXIS
		input wire  m00_axis_aclk, // req
		input wire  m00_axis_aresetn, // req
		output wire  m00_axis_tvalid, // req
		output wire [C_M00_AXIS_TDATA_WIDTH-1 : 0] m00_axis_tdata, // req
		// output wire [(C_M00_AXIS_TDATA_WIDTH/8)-1 : 0] m00_axis_tstrb,
		output wire  m00_axis_tlast, // req
		input wire  m00_axis_tready  //req
	);
// Instantiation of Axi Bus Interface S00_AXI
    wire [FIFO_ADDR_WIDTH-1:0] reg_post_trigger_count;
    wire [15:0] reg_trig_value;
	wire [31:0] reg_clk_division;
    wire reg_trig_channel, reg_trig_immediate, reg_trig_polarity, reg_trig_enable;

	trigger_acquire_v1_0_S00_AXI # ( 
		.C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH),
		.FIFO_ADDR_WIDTH(FIFO_ADDR_WIDTH)
	) trigger_acquire_v1_0_S00_AXI_inst (
		.S_AXI_ACLK(s00_axi_aclk),
		.S_AXI_ARESETN(s00_axi_aresetn),
		.S_AXI_AWADDR(s00_axi_awaddr),
		.S_AXI_AWPROT(s00_axi_awprot),
		.S_AXI_AWVALID(s00_axi_awvalid),
		.S_AXI_AWREADY(s00_axi_awready),
		.S_AXI_WDATA(s00_axi_wdata),
		.S_AXI_WSTRB(s00_axi_wstrb),
		.S_AXI_WVALID(s00_axi_wvalid),
		.S_AXI_WREADY(s00_axi_wready),
		.S_AXI_BRESP(s00_axi_bresp),
		.S_AXI_BVALID(s00_axi_bvalid),
		.S_AXI_BREADY(s00_axi_bready),
		.S_AXI_ARADDR(s00_axi_araddr),
		.S_AXI_ARPROT(s00_axi_arprot),
		.S_AXI_ARVALID(s00_axi_arvalid),
		.S_AXI_ARREADY(s00_axi_arready),
		.S_AXI_RDATA(s00_axi_rdata),
		.S_AXI_RRESP(s00_axi_rresp),
		.S_AXI_RVALID(s00_axi_rvalid),
		.S_AXI_RREADY(s00_axi_rready),
		
		.reg_post_trigger_count(reg_post_trigger_count),
		.reg_trig_value(reg_trig_value),
		.reg_trig_channel(reg_trig_channel), 
		.reg_trig_immediate(reg_trig_immediate), 
		.reg_trig_polarity(reg_trig_polarity), 
		.reg_trig_enable(reg_trig_enable),
		.reg_clk_division(reg_clk_division)
	);
   
	// instantiate XADC
	
	wire [15:0] xadc_data_ch0, xadc_data_ch1;
	wire[31:0] xadc_data;
	wire xadc_data_valid;
	xadc_controller xadc_controller(
        .DCLK(s00_axi_aclk), // Clock input for DRP
        .RESET(1'b0),
        .VP(VP),
        .VN(VN),
        .VAUXP_0(VAUXP_0),
        .VAUXN_0(VAUXN_0),
        .VAUXP_8(VAUXP_8),
        .VAUXN_8(VAUXN_8), // Auxiliary analog channel inputs
        .MEASURED_AUX0(xadc_data_ch0),
        .MEASURED_AUX8(xadc_data_ch1),
        .FIFO_EN(xadc_data_valid)
        );

    assign xadc_data = {xadc_data_ch0, xadc_data_ch1};
	
	
	wire [31:0] test_data;
	wire test_data_valid;
	test_pattern_generator test_pattern_generator(
		.clk(s00_axi_aclk),
		.reset(1'b0),
		.data_out(test_data),
		.data_valid(test_data_valid)
	);
	
	// selecting the test pattern
	wire [31:0] data_in;
	wire data_valid;
	assign data_in = test_select ? test_data : xadc_data;
	assign data_valid = test_select ? test_data_valid : xadc_data_valid;


	// dividing samples
	reg [31:0] div_count = 0;
	wire data_valid_div;
	always @(posedge s00_axi_aclk) begin
		if (data_valid) begin
			div_count <= div_count + 1;
			if (div_count >= reg_clk_division) begin
				div_count <= 0;
			end
		end
	end
	assign data_valid_div = data_valid && (div_count >= reg_clk_division);
	

    trigger_control #(.FIFO_ADDR_WIDTH(FIFO_ADDR_WIDTH), .C_M_AXIS_TDATA_WIDTH(C_M00_AXIS_TDATA_WIDTH)) trigger_control
    (
        // register inputs
        .reg_post_trigger_count(reg_post_trigger_count),
        .reg_trig_value(reg_trig_value),
        .reg_trig_enable(reg_trig_enable),
        .reg_trig_polarity(reg_trig_polarity),
        .reg_trig_immediate(reg_trig_immediate),
        .reg_trig_channel(reg_trig_channel),
        
        // xadc / datapath inputs
        .data_in_valid(data_valid_div),
        .data_in(data_in),

		.state_out(state_out),
		.trigger_out(trigger_out),
		.ram_wr_en(ram_wr_en),
		.ram_addr(ram_addr),
    
        // axi ports
        .M_AXIS_ACLK(m00_axis_aclk),
        .M_AXIS_ARESETN(m00_axis_aresetn),
        .M_AXIS_TVALID(m00_axis_tvalid),
        .M_AXIS_TDATA(m00_axis_tdata),
        .M_AXIS_TLAST(m00_axis_tlast),
        .M_AXIS_TREADY(m00_axis_tready)
    );
	// User logic ends

	endmodule