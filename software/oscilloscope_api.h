#ifndef RENDER_H
#define RENDER_H

#include "xil_printf.h"
#include "xscugic.h"
#include "xadc/xadc_api.h"
#include "vga/plotting_api.h"
#include "xscutimer.h"

#define RENDER_DATA_LENGTH 256
#define LOWER_BITS 0x0000FFFF

#define CLK_DIV_DEC 0
#define CLK_DIV_INC 1
#define CLK_DIV_MAX 0x10000
#define CLK_DIV_INC_VAL 0x01

#define TIMER_LOAD_VALUE XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ
// we will set timer to one second (equal to clock frequency)
// then the frame rate will simply be the number of frames since last interrupt

typedef struct {
	TriggerConfig* trigger_config;
	u32* xadc_data_base_addr;
	u32* video_memory_base_addr;
	u32* wave_ch1;
	u32* wave_ch2;
	int new_data;
	int rendered;
	int swap_buffer_flag;
	XScuTimer frame_timer;
	u32 n_frames;
	u32 n_vdma_callback;
	u32 clock_division;
} OscilloscopeStruct;

OscilloscopeStruct oscilloscope_context;

int init_intc(XScuGic* p_intc_inst, int intc_device_id) {
	// Local variables
	int             status = 0;
	XScuGic_Config* cfg_ptr;

	// Look up hardware configuration for device
	cfg_ptr = XScuGic_LookupConfig(intc_device_id);
	if (!cfg_ptr)
	{
		xil_printf("ERROR! No hardware configuration found for Interrupt Controller with device id %d.\r\n", intc_device_id);
		return -1;
	}

	// Initialize driver
	status = XScuGic_CfgInitialize(p_intc_inst, cfg_ptr, cfg_ptr->CpuBaseAddress);
	if (status != XST_SUCCESS)
	{
		xil_printf("ERROR! Initialization of Interrupt Controller failed with %d.\r\n", status);
		return -1;
	}
	return 0;
}



void xadc_callback(void* data) {
	oscilloscope_context.new_data = 1;
	oscilloscope_context.rendered = 0;
}

// when the vdma is finished update flags if we have rendered.
void vdma_callback(void* data) {
/*
	oscilloscope_context.n_vdma_callback++;
	if (oscilloscope_context.new_data &&
		oscilloscope_context.rendered &&
		!oscilloscope_context.displayed)
	{
			oscilloscope_context.n_frames++;
			plot_update();
			oscilloscope_context.displayed = 1;
			oscilloscope_context.new_data = 0;
	}
*/
}

// The timer is configured with auto-reload, so we can just print
void timer_callback(void* data) {
//	xil_printf("frame rate %d\n", oscilloscope_context.n_frames);
//	xil_printf("callback rate %d\n", oscilloscope_context.n_vdma_callback);
//	oscilloscope_context.n_frames=0;
//	oscilloscope_context.n_vdma_callback=0;
}

void fixed_to_string(char* string, int number) {
	unsigned int frac = ((number & 0x0F)*10)>>4;
	unsigned int whole = number >> 4;

	sprintf(string, "%d.%d", whole, frac);
}

void scope_blank() {
	Rect label_area = {0,0,DISPLAY_X_SIZE,40};
	display_draw_rect(_background_color, label_area);
	display_draw_rect(_canvas_color, _canvas_rect);
	draw_grid();

	int x_pos = 0;
	int y_pos = 0;
	draw_channel_label(x_pos, y_pos);
	parse_text("0.10 volts/div", (x_pos+200), y_pos, 1, COLOR_WHITE);


	// time scale will be a fixed integer
	char time_str[50] = "";
	u32 time_val = (409 << (oscilloscope_context.clock_division));
	fixed_to_string(time_str, time_val);
	strcat(time_str, " us/div");

	parse_text(time_str, (x_pos+400), y_pos, 1, COLOR_WHITE);

}


void update_clock_divider(u32 direction) {
	u32 clk_div = oscilloscope_context.clock_division;
	switch(direction) {
	case CLK_DIV_DEC:
		if(clk_div > 0)
			clk_div = clk_div >> 1;
		break;
	case CLK_DIV_INC:
		if(clk_div < CLK_DIV_MAX) {
			if(clk_div == 0) {
				clk_div = 1;
			} else {
				clk_div = clk_div << 1;
			}
		}
		break;
	default:
		break;
	}
	oscilloscope_context.clock_division = clk_div;
	set_clock_division(oscilloscope_context.clock_division);
}


int setup_timer(XScuGic* p_intc_inst) {
	oscilloscope_context.n_frames = 0;
	oscilloscope_context.n_vdma_callback = 0;

	int status;
	XScuTimer_Config *config_ptr;
	XScuTimer* timer = &(oscilloscope_context.frame_timer);

	config_ptr = XScuTimer_LookupConfig(XPAR_XSCUTIMER_0_DEVICE_ID);
	status = XScuTimer_CfgInitialize(timer, config_ptr,	config_ptr->BaseAddr);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	int timer_interrupt_id = XPAR_SCUTIMER_INTR;
	status = XScuGic_Connect(p_intc_inst, timer_interrupt_id,
							 timer_callback, (void*) timer);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XScuGic_Enable(p_intc_inst, timer_interrupt_id);

	XScuTimer_EnableInterrupt(timer);
	XScuTimer_EnableAutoReload(timer);
	XScuTimer_LoadTimer(timer, TIMER_LOAD_VALUE);
	XScuTimer_Start(timer);

	return XST_SUCCESS;
}


// here we do the following:
// 1. Instantiate the GIC
// 2. Initialize the VDMA and plotting api, with a local callback
// 3. Initialize the XADC & DMA with a local callback
void init_oscilloscope(u32 video_memory_base_addr, u32 xadc_data_base_addr, TriggerConfig* config) {
	oscilloscope_context.xadc_data_base_addr = (u32*)xadc_data_base_addr;
	oscilloscope_context.video_memory_base_addr = (u32*)video_memory_base_addr;
	oscilloscope_context.trigger_config = config;
	oscilloscope_context.new_data = 0;
	oscilloscope_context.swap_buffer_flag = 0;
	oscilloscope_context.rendered = 0;
	oscilloscope_context.clock_division = 0;
	set_clock_division(0x00000000);

	// instantiate GIC
	XScuGic* p_intc_inst = malloc(sizeof(XScuGic));
	init_intc(p_intc_inst, XPAR_PS7_SCUGIC_0_DEVICE_ID);

	// setup the frame counter
	int status = setup_timer(p_intc_inst);
	if (status == XST_FAILURE) {
		xil_printf("Timer initialization failed\n");
	}

	// setup the display api
	display_init(video_memory_base_addr, p_intc_inst, vdma_callback, NULL);
	// initialize the plotting api
	plot_init_fast(RENDER_DATA_LENGTH, "us");

	// initialize the waves
	oscilloscope_context.wave_ch1 = malloc(RENDER_DATA_LENGTH*sizeof(u32));
	oscilloscope_context.wave_ch2 = malloc(RENDER_DATA_LENGTH*sizeof(u32));
	scope_blank();
	plot_draw_array(oscilloscope_context.wave_ch1, COLOR_RED);
	plot_draw_array(oscilloscope_context.wave_ch2, COLOR_BLUE);
	plot_update();


	// connect the XADC callback
	init_xadc_handler(p_intc_inst, (u32*)xadc_data_base_addr, RENDER_DATA_LENGTH, xadc_callback);

	// Initialize exception table and register the interrupt controller handler with exception table

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, p_intc_inst);

	// Enable non-critical exceptions
	Xil_ExceptionEnable();

	set_trigger(oscilloscope_context.trigger_config);
}

/*
void oscilloscope_render_data() {
	volatile u32* data = oscilloscope_context.xadc_data_base_addr;

	plot_blank();

	for (int i = 0; i < RENDER_DATA_LENGTH; i++) {
		oscilloscope_context.wave_ch1[i] = data[i] & LOWER_BITS;
		oscilloscope_context.wave_ch2[i] = data[i] >> 16;
	}
	plot_draw_array(oscilloscope_context.wave_ch1, COLOR_RED);
	plot_draw_array(oscilloscope_context.wave_ch2, COLOR_BLUE);

	oscilloscope_context.rendered = 1;
}
*/

// (hopefully) faster version of the render 
void oscilloscope_render_data_opt(){
	volatile u32* data = oscilloscope_context.xadc_data_base_addr;
	
	scope_blank();

	u32 x_val = 0;
	u32 y_prev1 = (data[0] & Y_MAX_VALUE)*(_y_pix_scaling);
	u32 y_prev2 = (data[0] >> 16)*(_y_pix_scaling);
	u32 y_new;
	Line line;
	for (int i = 0; i < (_x_n_samples-1); i++) {
		y_new = data[i+1];
		
		// calculate x positions
		line.x0 = (x_val >> N_FRACTIONAL_BITS) + _canvas_margin;
		x_val += _x_pix_scaling;
		line.x1 = (x_val >> N_FRACTIONAL_BITS) + _canvas_margin;
		
		// draw ch1
		line.y0 = _canvas_y_bottom - (y_prev1>>N_FRACTIONAL_BITS);
		y_prev1 = (y_new & Y_MAX_VALUE) * _y_pix_scaling;
		line.y1 = _canvas_y_bottom - (y_prev1>>N_FRACTIONAL_BITS);
		display_draw_line(line, CH1_COLOR);
		
		// draw ch2
		line.y0 = _canvas_y_bottom - (y_prev2 >> N_FRACTIONAL_BITS);
		y_prev2 = (y_new >> 16) * _y_pix_scaling;
		line.y1 = _canvas_y_bottom - (y_prev2 >> N_FRACTIONAL_BITS);
		display_draw_line(line, CH2_COLOR);
	}
	oscilloscope_context.rendered = 1;
	oscilloscope_context.new_data = 0;
	plot_update();
	set_trigger(oscilloscope_context.trigger_config);
}
#endif
