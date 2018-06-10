#include <stdio.h>
#include "xparameters.h"
#include <xuartps_hw.h>

#include "xadc/xadc_handler.h"
#include "xadc/xadc_api.h"
#include "vga/plotting_api.h"

#include "oscilloscope_api.h"

#define DISPLAY_MEMORY_BASE	 0x01000000
#define SCOPE_DATA_BASE_ADDR 0x02000000

int main()
{
	xil_printf("hello\n");
	TriggerConfig trigger_config;
	trigger_config.post_trigger_count = 0x00000000;
	trigger_config.trigger_value = 0x00002000;
	trigger_config.en  = 1;
	trigger_config.pol = 0;
	trigger_config.imm = 1;
	trigger_config.ch  = 1;

	init_oscilloscope(DISPLAY_MEMORY_BASE, SCOPE_DATA_BASE_ADDR, &trigger_config);
	int mode = 3;

	while(1) {
		if (XUartPs_IsReceiveData(STDIN_BASEADDRESS)) {
			char input = inbyte();
			switch(input) {
				case '1':
					xil_printf("resetting trigger\n");
					reset_trigger();
					break;
				case '2':
					xil_printf("decrementing clock divider\n");
					update_clock_divider(CLK_DIV_DEC);
					break;
				case '3':
					xil_printf("incrementing clock divider\n");
					update_clock_divider(CLK_DIV_INC);
					break;
				case '4':
					xil_printf("setting continuous mode\n");
					trigger_config.imm = 1;
					trigger_config.post_trigger_count = 0;
					break;
				case '5':
					xil_printf("setting to trigger mode\n");
					trigger_config.imm = 0;
					trigger_config.post_trigger_count = 0x00000080;
					break;
				case '6':
					xil_printf("changing trigger channel\n");
					trigger_config.ch = !(trigger_config.ch);
					break;
				case '7':
					xil_printf("changing trigger polarity\n");
					trigger_config.pol = !(trigger_config.pol);
					break;
				case 'q':
					return 0;
					break;
				default:
					break;
					//xil_printf("Unknown input %s\n", input);
			}
		}

		if (oscilloscope_context.new_data && !oscilloscope_context.rendered) {
			oscilloscope_render_data_opt();
		}
	}
}
