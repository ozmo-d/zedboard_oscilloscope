#include "vdma_api.h"

static int ReadSetup(vdma_handle *vdma_context);
static int StartTransfer(XAxiVdma *InstancePtr);
static unsigned int context_init=0;

/*****************************************************************************/
/**
*
* VDMAInit
*
* This API is the interface between application and other API. When application will call
* this API with right argument, This API will call rest of the API to configure the read
* and write path of VDMA,based on ID. After that it will start both the read and write path
* of VDMA
*
* @param	InstancePtr is the handle to XAxiVdma data structure.
* @param	DeviceId is the device ID of current VDMA
* @param	hsize is the horizontal size of the frame. It will be in Pixels.
* 		The actual size of frame will be calculated by multiplying this
* 		with tdata width.
* @param 	vsize is the Vertical size of the frame.
* @param	buf_base_addr is the buffer address where frames will be written
*		and read by VDMA.
* @param 	number_frame_count specifies after how many frames the interrupt
*		should come.
* @param 	enable_frm_cnt_intr is for enabling frame count interrupt
*		when set to 1.
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
******************************************************************************/
int VDMAInit(XAxiVdma* InstancePtr, int DeviceId, int hsize,
		int vsize, int buf_base_addr)
{
	int Status,i;
	XAxiVdma_Config *Config;
	XAxiVdma_FrameCounter FrameCfgPtr;

	/* This is one time initialization of state machine context.
	 * In first call it will be done for all VDMA instances in the system.
	 */
	if(context_init==0) {
		for(i=0; i < XPAR_XAXIVDMA_NUM_INSTANCES; i++) {
			vdma_context[i].InstancePtr = NULL;
			vdma_context[i].device_id = -1;
			vdma_context[i].hsize = 0;
			vdma_context[i].vsize = 0;
			vdma_context[i].init_done = 0;
			vdma_context[i].buffer_address = 0;
			vdma_context[i].enable_frm_cnt_intr = 1;
			vdma_context[i].number_of_frame_count = 1;

		}
		context_init = 1;
	}

	/* The below initialization will happen for each VDMA. The API argument
	 * will be stored in internal data structure
	 */

	/* The information of the XAxiVdma_Config comes from hardware build.
	 * The user IP should pass this information to the AXI DMA core.
	 */
	Config = XAxiVdma_LookupConfig(DeviceId);
	if (!Config) {
		xil_printf("No video DMA found for ID %d\r\n",DeviceId );
		return XST_FAILURE;
	}

	if(vdma_context[DeviceId].init_done ==0) {
		vdma_context[DeviceId].InstancePtr = InstancePtr;

		/* Initialize DMA engine */
		Status = XAxiVdma_CfgInitialize(vdma_context[DeviceId].InstancePtr,
						Config, Config->BaseAddress);
		if (Status != XST_SUCCESS) {
			xil_printf("Configuration Initialization failed %d\r\n",
					Status);
			return XST_FAILURE;
		}

		vdma_context[DeviceId].init_done = 1;
	}

	vdma_context[DeviceId].device_id = DeviceId;
	vdma_context[DeviceId].vsize = vsize;
	vdma_context[DeviceId].buffer_address = buf_base_addr;
	vdma_context[DeviceId].hsize = hsize * (Config->Mm2SStreamWidth>>3);

//	FrameCfgPtr.ReadDelayTimerCount = 1;
//	FrameCfgPtr.ReadFrameCount = 1;
//
//	FrameCfgPtr.WriteDelayTimerCount = 1;
//	FrameCfgPtr.WriteFrameCount = vdma_context[DeviceId].number_of_frame_count;
//
//	Status = XAxiVdma_SetFrameCounter(vdma_context[DeviceId].InstancePtr,&FrameCfgPtr);
//	if (Status != XST_SUCCESS) {
//		xil_printf("Configuration Initialization failed %d\r\n",
//				Status);
//		return XST_FAILURE;
//	}
//
//	XAxiVdma_IntrEnable(vdma_context[DeviceId].InstancePtr,	XAXIVDMA_IXR_FRMCNT_MASK, XAXIVDMA_READ);

	/* Setup the read channel */
	Status = ReadSetup(&vdma_context[DeviceId]);
	if (Status != XST_SUCCESS) {
		xil_printf("Read channel setup failed %d\r\n", Status);
		if(Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");
		return XST_FAILURE;
	}


	/* Start the DMA engine to transfer */
	Status = StartTransfer(vdma_context[DeviceId].InstancePtr);
	if (Status != XST_SUCCESS) {
		if(Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the read channel
*
* @param	vdma_context is the context pointer to the VDMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int ReadSetup(vdma_handle *vdma_context)
{
	int Index;
	u32 Addr;
	int Status;

	vdma_context->ReadCfg.VertSizeInput = vdma_context->vsize;
	vdma_context->ReadCfg.HoriSizeInput = vdma_context->hsize;

	vdma_context->ReadCfg.Stride = vdma_context->hsize;
	vdma_context->ReadCfg.FrameDelay = 0;  /* This example does not test frame delay */

	vdma_context->ReadCfg.EnableCircularBuf = 0;
	vdma_context->ReadCfg.EnableSync = 1;  /* Gen-Lock */

	vdma_context->ReadCfg.PointNum = 0;
	vdma_context->ReadCfg.EnableFrameCounter = 0; /* Endless transfers */

	vdma_context->ReadCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */
	/* Configure the VDMA is per fixed configuration, This configuration is being used by majority
	 * of customer. Expert users can play around with this if they have different configurations */

	Status = XAxiVdma_DmaConfig(vdma_context->InstancePtr, XAXIVDMA_READ, &vdma_context->ReadCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("Read channel config failed %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Initialize buffer addresses
	 *
	 * These addresses are physical addresses
	 */
	Addr = vdma_context->buffer_address;

	for(Index = 0; Index < vdma_context->InstancePtr->MaxNumFrames; Index++) {
		vdma_context->ReadCfg.FrameStoreStartAddr[Index] = Addr;
		xil_printf("vdma frame buffer %i at address 0x%08x initialized.\n", Index, Addr);
		Addr +=  vdma_context->hsize * vdma_context->vsize;
	}

	/* Set the buffer addresses for transfer in the DMA engine
	 * The buffer addresses are physical addresses
	 */
	Status = XAxiVdma_DmaSetBufferAddr(vdma_context->InstancePtr, XAXIVDMA_READ,
			vdma_context->ReadCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Read channel set buffer address failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


int VDMAUpdateParkPtr(int ptr) {
	vdma_context[0].ReadCfg.FixedFrameStoreAddr = ptr;

	int status = XAxiVdma_DmaConfig(vdma_context->InstancePtr, XAXIVDMA_READ, &(vdma_context[0].ReadCfg));
	if (status != XST_SUCCESS) {
		xil_printf("Couldn't update park pointer %d\r\n", status);
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function starts the DMA transfers. Since the DMA engine is operating
* in circular buffer mode, video frames will be transferred continuously.
*
* @param	InstancePtr points to the DMA engine instance
*
* @return
*		- XST_SUCCESS if both read and write start successfully
*		- XST_FAILURE if one or both directions cannot be started
*
* @note		None.
*
******************************************************************************/
static int StartTransfer(XAxiVdma *InstancePtr)
{
	int Status;

	/* Start the Read channel of VDMA */
	Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_READ);
	if (Status != XST_SUCCESS) {
		xil_printf("Start read transfer failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


int VDMAReset(XAxiVdma *InstancePtr) {
	XAxiVdma_Reset(InstancePtr, XAXIVDMA_READ);

	int Polls = XAXIVDMA_RESET_TIMEOUT;
	while(Polls && XAxiVdma_ResetNotDone(InstancePtr, XAXIVDMA_READ)) {
		Polls -= 1;
	}
	if (!Polls) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

