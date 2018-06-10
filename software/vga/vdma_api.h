/******************************************************************************
*
* Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
 *
 * Modified vdma_api, removed write path and simplified
 ****************************************************************************/

/******************** Include files **********************************/
#include "xaxivdma.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"


/******************** Data structure Declarations *****************************/


// Status registers
#define VDMA_STATUS_REGISTER_HALTED                     0x00000001  // Read-only
#define VDMA_STATUS_REGISTER_VDMAInternalError          0x00000010  // Read or write-clear
#define VDMA_STATUS_REGISTER_VDMASlaveError             0x00000020  // Read-only
#define VDMA_STATUS_REGISTER_VDMADecodeError            0x00000040  // Read-only
#define VDMA_STATUS_REGISTER_StartOfFrameEarlyError     0x00000080  // Read-only
#define VDMA_STATUS_REGISTER_EndOfLineEarlyError        0x00000100  // Read-only
#define VDMA_STATUS_REGISTER_StartOfFrameLateError      0x00000800  // Read-only
#define VDMA_STATUS_REGISTER_FrameCountInterrupt        0x00001000  // Read-only
#define VDMA_STATUS_REGISTER_DelayCountInterrupt        0x00002000  // Read-only
#define VDMA_STATUS_REGISTER_ErrorInterrupt             0x00004000  // Read-only
#define VDMA_STATUS_REGISTER_EndOfLineLateError         0x00008000  // Read-only
#define VDMA_STATUS_REGISTER_FrameCount                 0x00ff0000  // Read-only
#define VDMA_STATUS_REGISTER_DelayCount                 0xff000000  // Read-only

#define DEBUG_MODE		0
#define XAXIVDMA_RESET_TIMEOUT   500


typedef struct vdma_handle
{
	/* The device ID of the VDMA */
	unsigned int device_id;
	/* The state variable to keep track if the initialization is done*/
	unsigned int init_done;
	/** The XAxiVdma driver instance data. */
	XAxiVdma* InstancePtr;
	/* The XAxiVdma_DmaSetup structure contains all the necessary information to
	 * start a frame write or read. */
	XAxiVdma_DmaSetup ReadCfg;
	/* Horizontal size of frame */
	unsigned int hsize;
	/* Vertical size of frame */
	unsigned int vsize;
	/* Buffer address from where read and write will be done by VDMA */
	unsigned int buffer_address;
	/* Flag to tell VDMA to interrupt on frame completion*/
	unsigned int enable_frm_cnt_intr;
	/* The counter to tell VDMA on how many frames the interrupt should happen*/
	unsigned int number_of_frame_count;
}vdma_handle;

vdma_handle vdma_context[XPAR_XAXIVDMA_NUM_INSTANCES];

int VDMAReset(XAxiVdma *InstancePtr);
int VDMAInit(XAxiVdma* InstancePtr, int DeviceId, int hsize, int vsize, int buf_base_addr); // now enables interrupts
int VDMAUpdateParkPtr(int ptr);
