#include <stdio.h>
#include "xil_types.h"

//#include "xtmrctr.h"
#include "xparameters.h"

#include "xil_io.h"
#include "xil_exception.h"
#include "xscugic.h"

XScuGic InterruptController; /* Instance of the Interrupt Controller */
static XScuGic_Config *GicConfig;/* The configuration parameters of the
controller */

#define IRQ_ID_BUTTON_UP     65
#define IRQ_ID_BUTTON_LEFT   64
#define IRQ_ID_BUTTON_CENTER 63
#define IRQ_ID_BUTTON_RIGHT  62
#define IRQ_ID_BUTTON_DOWN   61

void Button_InterruptHandler(void* data) {
	xil_printf("Button Pressed id:");
	xil_printf("%d", *(u32*)data);
}

void Timer_InterruptHandler(void *data, u8 TmrCtrNumber)
{
print(" Interrupt acknowledged \n \r ");
print("\r\n");
print("\r\n");
}

int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr)
{
/*
* Connect the interrupt controller interrupt handler to the hardware
* interrupt handling logic in the ARM processor.
*/
Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
(Xil_ExceptionHandler) XScuGic_InterruptHandler,
XScuGicInstancePtr);
/*
* Enable interrupts in the ARM
*/
Xil_ExceptionEnable();
return XST_SUCCESS;
}

int ScuGicInterrupt_Init(u16 DeviceId)
{
	int Status;
	/*
	* Initialize the interrupt controller driver so that it is ready to
	* use.
	* */
	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}
	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig,
	GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	* Setup the Interrupt System
	* */
	Status = SetUpInterruptSystem(&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	* Connect a device driver handler that will be called when an
	* interrupt for the device occurs, the device driver handler performs
	* the specific interrupt processing for the device
	*/
	Status = XScuGic_Connect(&InterruptController, IRQ_ID_BUTTON_DOWN, (Xil_ExceptionHandler)Button_InterruptHandler, (void *)NULL);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	* Enable the interrupt for the device and then cause (simulate) an
	* interrupt so the handlers will be called
	*/
	XScuGic_Enable(&InterruptController, IRQ_ID_BUTTON_DOWN);
	return XST_SUCCESS;
}

int main() {

	int xStatus;
	print("##### Application Starts #####\n\r");
	print("\r\n");

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//SCUGIC interrupt controller Intialization
	//Registration of the Timer ISR
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	xStatus = ScuGicInterrupt_Init(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	if(XST_SUCCESS != xStatus)
	print(" :( SCUGIC INIT FAILED \n\r");

	//Wait For interrupt;

	print("Wait for the Timer interrupt to tigger \r\n");
	print("########################################\r\n");
	print(" \r\n");

	while(1)
	{
	}
	return 0;
}




