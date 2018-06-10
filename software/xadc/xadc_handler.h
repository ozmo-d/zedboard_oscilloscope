#ifndef XADC_HANDLER_H
#define XADC_HANDLER_H

#include <stdlib.h>

#include "xparameters.h"
#include "xil_io.h"
#include "xaxidma.h"
#include "xscugic.h"

#include "xadc_api.h"


int _data_length;
volatile u32* _buffer_addr;

typedef void (*XADCCallback)(void* data);

typedef struct {
	XAxiDma* p_dma_inst;
	XADCCallback xadc_callback;
} XadcHandler;


// callback when DMA receives data
static void dma_callback(void* CallbackRef) {
	// when the dma gets data

	u32 irq_status;
	XadcHandler* xadc = (XadcHandler*) CallbackRef;

	XAxiDma* p_dma_inst = xadc->p_dma_inst;


	XAxiDma_IntrDisable(p_dma_inst, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

	irq_status = XAxiDma_IntrGetIrq(p_dma_inst, XAXIDMA_DEVICE_TO_DMA);

	XAxiDma_IntrAckIrq(p_dma_inst, irq_status, XAXIDMA_DEVICE_TO_DMA);

	if(!(irq_status & XAXIDMA_IRQ_ALL_MASK))
		return;

	if (irq_status & XAXIDMA_IRQ_IOC_MASK)
		xadc->xadc_callback(_buffer_addr);

	XAxiDma_IntrEnable(p_dma_inst, (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK), XAXIDMA_DEVICE_TO_DMA);

	XAxiDma_SimpleTransfer(p_dma_inst, (UINTPTR)_buffer_addr, _data_length, XAXIDMA_DEVICE_TO_DMA);
}

int init_dma(XAxiDma* p_dma_inst, int dma_device_id) {

	// Local variables
	int             status = 0;
	XAxiDma_Config* cfg_ptr;

	// Look up hardware configuration for device
	cfg_ptr = XAxiDma_LookupConfig(dma_device_id);
	if (!cfg_ptr)
	{
		xil_printf("ERROR! No hardware configuration found for AXI DMA with device id %d.\r\n", dma_device_id);
		return -1;
	}

	// Initialize driver
	status = XAxiDma_CfgInitialize(p_dma_inst, cfg_ptr);
	if (status != XST_SUCCESS)
	{
		xil_printf("ERROR! Initialization of AXI DMA failed with %d\r\n", status);
		return -1;
	}

	// Test for Scatter Gather
	if (XAxiDma_HasSg(p_dma_inst))
	{
		xil_printf("ERROR! Device configured as SG mode.\r\n");
		return -1;
	}

	// Reset DMA
	XAxiDma_Reset(p_dma_inst);
	while (!XAxiDma_ResetIsDone(p_dma_inst)) {}

	// Enable DMA interrupts
	XAxiDma_IntrEnable(p_dma_inst, (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK), XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_IntrEnable(p_dma_inst, (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK), XAXIDMA_DEVICE_TO_DMA);

	return 0;
}


// only need the base address, and the callback that we want to use...
int init_xadc_handler(XScuGic* p_intc_inst, u32* base_addr, int data_length, XADCCallback xadc_callback) {
	_data_length = data_length*sizeof(u32);
	_buffer_addr = base_addr;

	// instantiate DMA
	XAxiDma* p_dma_inst = malloc(sizeof(XAxiDma));
	init_dma(p_dma_inst, XPAR_AXIDMA_0_DEVICE_ID);

	// instantiate handler
	XadcHandler* xadc_handler = malloc(sizeof(XadcHandler));
	xadc_handler->p_dma_inst = p_dma_inst;
	xadc_handler->xadc_callback = xadc_callback;

	// connect interrupt
	int s2mm_intr_id = XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR;
	u32 status;
	XScuGic_SetPriorityTriggerType(p_intc_inst, s2mm_intr_id, 0x20, 0x3); // initialize a rising edge interrupt
	
	status = XScuGic_Connect(p_intc_inst, s2mm_intr_id, (Xil_InterruptHandler) dma_callback, xadc_handler); // add callback
	if (status != XST_SUCCESS)
	{
		xil_printf("ERROR! Failed to connect mm2s_isr to the interrupt controller.\r\n", status);
		return -1;
	}

	XScuGic_Enable(p_intc_inst, s2mm_intr_id); // enable the interrupt

	// start the transfer
	status = XAxiDma_SimpleTransfer(p_dma_inst, (UINTPTR)_buffer_addr, _data_length, XAXIDMA_DEVICE_TO_DMA);
	if (status != XST_SUCCESS)
	{
		xil_printf("ERROR! Couldn't start simple transfer. \r\n", status);
		return -1;
	}

	return 0;
}

#endif
