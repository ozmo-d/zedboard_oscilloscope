#ifndef XADC_API_H   /* prevent circular inclusions */
#define XADC_API_H   /* by using protection macros */
/*
Wrapper for XADC module

*/

#include "xparameters.h"

// XPAR_TRIGGER_ACQUIRE_V1_0_0_BASEADDR
// XPAR_TRIGGER_ACQUIRE_V1_0_0_HIGHADDR

// Register Offset
#define TRIGGER_ACQUIRE_REG0_OFFSET 0
#define TRIGGER_ACQUIRE_REG1_OFFSET 4
#define TRIGGER_ACQUIRE_REG2_OFFSET 8

// Register values
#define TRIGGER_ACQUIRE_REG1_EN  0x00000001
#define TRIGGER_ACQUIRE_REG1_POL 0x00000002
#define TRIGGER_ACQUIRE_REG1_IMM 0x00000004
#define TRIGGER_ACQUIRE_REG1_CH  0x00000008


typedef struct {
	u32 post_trigger_count;
	u32 trigger_value;
	u32 en;
	u32 pol;
	u32 imm;
	u32 ch;
} TriggerConfig;


// init the trigger with settings
void set_trigger(TriggerConfig* config) {
	u32 reg0 = (config->post_trigger_count << 16) | config->trigger_value;

	u32 reg1 = 0x0;
	if (config->pol)
		reg1 = reg1 | TRIGGER_ACQUIRE_REG1_POL;
	if (config->en)
		reg1 = reg1 | TRIGGER_ACQUIRE_REG1_EN;
	if (config->imm)
		reg1 = reg1 | TRIGGER_ACQUIRE_REG1_IMM;
	if (config->ch)
		reg1 = reg1 | TRIGGER_ACQUIRE_REG1_CH;

	u32 reg0_addr = XPAR_TRIGGER_ACQUIRE_V1_0_0_BASEADDR + TRIGGER_ACQUIRE_REG0_OFFSET;
	u32 reg1_addr = XPAR_TRIGGER_ACQUIRE_V1_0_0_BASEADDR + TRIGGER_ACQUIRE_REG1_OFFSET;

	Xil_Out32(reg0_addr, reg0);
	Xil_Out32(reg1_addr, reg1);
	//Xil_Out32(reg0_addr, 0xFFFFFFFF);
	//Xil_Out32(reg1_addr, 0xFFFFFFFF);
}

void set_clock_division(u32 value) {
	u32 reg2_addr = XPAR_TRIGGER_ACQUIRE_V1_0_0_BASEADDR + TRIGGER_ACQUIRE_REG2_OFFSET;
	Xil_Out32(reg2_addr, value);
}

// After the system is triggered, it will go to the 'active' state.
// To move to the 'set' state we need to call this function.
void reset_trigger() {
	u32 reg1_addr  = XPAR_TRIGGER_ACQUIRE_V1_0_0_BASEADDR + TRIGGER_ACQUIRE_REG1_OFFSET;
	u32 reg1_value = Xil_In32(reg1_addr);

	reg1_value = reg1_value | TRIGGER_ACQUIRE_REG1_EN;
	Xil_Out32(reg1_addr, reg1_value);
}

void set_all_f() {
	u32 reg0 = 0xFFFFFFFF;
	u32 reg1 = 0xFFFFFFFF;

	u32 reg0_addr = XPAR_TRIGGER_ACQUIRE_V1_0_0_BASEADDR + TRIGGER_ACQUIRE_REG0_OFFSET;
	u32 reg1_addr = XPAR_TRIGGER_ACQUIRE_V1_0_0_BASEADDR + TRIGGER_ACQUIRE_REG1_OFFSET;

	Xil_Out32(reg0_addr, reg0);
	Xil_Out32(reg1_addr, reg1);
}


#endif
