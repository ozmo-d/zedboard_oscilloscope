/* Simplified API for doing basic operations with the VDMA
 * Wesley Kendall 2018
 *
 */

/*** Include file ***/
#ifndef DISPLAY_API_H_
#define DISPLAY_API_H_

#include "xparameters.h"
#include "xstatus.h"
#include "xil_exception.h"
#include "xil_assert.h"
#include "xaxivdma.h"
#include "xaxivdma_i.h"
#include "xil_cache.h"
#include "vdma_api.h"
#include "stdlib.h"
#include "math.h"
#include "xscugic.h"


#define COLOR_RED   0xFF000000
#define COLOR_GREEN 0x00FF0000
#define COLOR_BLUE  0x0000FF00
#define COLOR_BLUE2 0x000000FF
#define COLOR_WHITE  0xFFFFFFFF
#define COLOR_BLACK  0x00000000

#define DISPLAY_X_SIZE 800
#define DISPLAY_Y_SIZE 600
#define DISPLAY_TOTAL_MEM_SIZE DISPLAY_Y_SIZE*DISPLAY_X_SIZE

#define N_FRAME_BUFFERS 2

XAxiVdma VdmaInstancePtr;

typedef void (*DisplayCallback)(void* data);

typedef struct {
	int x0, y0, x1, y1;
} Line;


typedef struct {
	int x0, y0, xsz, ysz;
} Rect;

typedef struct {
	int active_buffer_i;
	int writable_buffer_i;
	volatile u32** buffer;
	XAxiVdma* VdmaInstancePtr;
	DisplayCallback display_callback;
} DisplayStruct;

DisplayStruct display_context;

int display_init(u32 srcBuffer, XScuGic* p_intc_inst, DisplayCallback display_callback, void* callback_ref) {
	display_context.VdmaInstancePtr = &VdmaInstancePtr;
 	int status;
 	status = VDMAReset(display_context.VdmaInstancePtr);
	if (status != XST_SUCCESS) {
		xil_printf("Reset failed\n");
		return XST_FAILURE;
	};

	// RUN VDMA CONTINUOUSLY
	status = VDMAInit(display_context.VdmaInstancePtr, 0, DISPLAY_X_SIZE, DISPLAY_Y_SIZE,
						srcBuffer);
	if (status != XST_SUCCESS) {
		xil_printf("Transfer of frames failed with error = %d\r\n", status);
		return XST_FAILURE;
	} else {
		xil_printf("Transfer of frames started \r\n");
	}

	// connect interrupt to GIC interrupts
	/*
	int fsync_intr_id = XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR;
	XScuGic_SetPriorityTriggerType(p_intc_inst, fsync_intr_id, 0xA0, 0x3);

	status = XScuGic_Connect(p_intc_inst, fsync_intr_id,	(Xil_InterruptHandler)callback_ref,	&display_context);
	if (status != XST_SUCCESS) {
		return status;
	}
	XScuGic_Enable(p_intc_inst, fsync_intr_id);
	*/

	for (int i=0; i < N_FRAME_BUFFERS; i++) {
		display_context.buffer[i] = (u32*) vdma_context->ReadCfg.FrameStoreStartAddr[i];
	}
	return XST_SUCCESS;
}


void display_fill_screen(u32 color) {
	for (u32 i=0; i < (DISPLAY_X_SIZE*DISPLAY_Y_SIZE); i++) {
		((volatile u32*)display_context.buffer[display_context.writable_buffer_i])[i] = color;
	}
}


void display_draw_rect(u32 color, Rect rect) {
	for (u32 i=rect.x0; i < (rect.x0 + rect.xsz); i++) {
		for (u32 j=rect.y0; j < (rect.y0 + rect.ysz); j++) {
			u32 pos = i + j*DISPLAY_X_SIZE;
			if (pos < DISPLAY_TOTAL_MEM_SIZE) {
				((volatile u32*)display_context.buffer[display_context.writable_buffer_i])[pos] = color;
			}
		}
	}
}

static inline void draw_point(int x, int y, u32 color) {
	int pos = x + y*DISPLAY_X_SIZE;
	if (pos < DISPLAY_TOTAL_MEM_SIZE) {
		((volatile u32*)display_context.buffer[display_context.writable_buffer_i])[pos] = color;
	}
}


static inline void _draw_line_low(int x0, int y0, int x1, int y1, u32 color) {
	int dx = x1 - x0;
	int dy = y1 - y0;
	int y = y0;
	int yi = 1;
	if (dy < 0) {
		yi = -1;
		dy = -dy;
	}
	if (dy == 0) {
		for (int x=x0; x< x1; x++) {
			draw_point(x, y0, color);
		}
		return;
	}
	int D = (dx << 1) - dy;


	for (int x=x0; x < x1; x++) {
		draw_point(x, y, color);
		if (D > 0) {
			y = y + yi;
			D = D - (dx << 1);
		}
		D = D + (dy << 1);
	}
	return;
}

static inline void _draw_line_high(int x0, int y0, int x1, int y1, u32 color) {
	int dx = x1 - x0;
	int dy = y1 - y0;
	int x = x0;
	int xi = 1;
	if (dx < 0) {
		xi = -1;
		dx = -dx;
	}

	if (dx == 0) {
		for (int y=y0; y < y1; y++) {
			draw_point(x0, y, color);
		}
		return;
	}

	int D = (dx << 1) - dy;


	for (int y=y0; y < y1; y++) {
		draw_point(x, y, color);
		if (D > 0) {
			x = x + xi;
			D = D - (dy << 1);
		}
		D = D + (dx << 1);
	}
	return;
}


// Draw a line segment
// Bresenham's Line Algorithm (no anti-aliasing)
// Borrowed from Wikipedia: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void display_draw_line(Line line, u32 color) {
	if (abs(line.y1- line.y0) < abs(line.x1 -line.x0)) {
		_draw_line_low(line.x0, line.y0, line.x1, line.y1, color);
	}
	else {
		if (line.y0 < line.y1) {
			_draw_line_high(line.x0, line.y0, line.x1, line.y1, color);
		}
		else {
			_draw_line_high(line.x1, line.y1, line.x0, line.y0, color);
		}
	}
}




void display_swap_buffers() {
	Xil_DCacheFlushRange((u32)display_context.buffer[display_context.writable_buffer_i], DISPLAY_TOTAL_MEM_SIZE*4);
	//Xil_DCacheFlushRange((u32)display_context.buffer[display_context.active_buffer_i], DISPLAY_TOTAL_MEM_SIZE*32);

	// access dma to change buffer pointer
	VDMAUpdateParkPtr(display_context.writable_buffer_i);
	// update variables
	display_context.active_buffer_i   = (display_context.active_buffer_i + 1) % N_FRAME_BUFFERS;
	display_context.writable_buffer_i = (display_context.writable_buffer_i+1) % N_FRAME_BUFFERS;
}


volatile u32* display_get_frame_buffer() {
	return ((volatile u32*) display_context.buffer[display_context.writable_buffer_i]);
}

#endif //DISPLAY_API_H_
