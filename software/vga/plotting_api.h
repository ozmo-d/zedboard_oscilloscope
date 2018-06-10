#ifndef PLOTTING_API_H_
#define PLOTTING_API_H_
/* API for drawing plots using the display api
 * Wesley Kendall 2018
 *
 */

#include "display_api.h"
#include <stdlib.h>
#include "letter.h"

// define the axis of the plot.
#define N_GRID_STEPS_X 10
#define N_GRID_STEPS_Y 10
static unsigned int _canvas_margin, _canvas_x_size, _canvas_y_size, _canvas_y_mid, _canvas_y_bottom;
static unsigned int _grid_step_x, _grid_step_y;

static Rect _canvas_rect;
static u32 _grid_color =       0x00000000;
static u32 _background_color = 0x00000000;
static u32 _canvas_color =     0x00020000;

//#define COLOR_RED   0xFF000000
//#define COLOR_GREEN 0x00FF0000
//#define COLOR_BLUE  0x0000FF00
#define CH1_COLOR COLOR_RED
#define CH2_COLOR COLOR_BLUE


typedef struct {
	int x, y;
} Point;

typedef struct {
	int length;
	Point* points;
} PointList;


#define N_FRACTIONAL_BITS 16
static u32 _x_pix_scaling = (1 << N_FRACTIONAL_BITS);
static u32 _x_n_samples;

#define Y_MAX_VALUE 0x0000FFFF
static u32 _y_pix_scaling;

void draw_grid();

void plot_init() {
	_canvas_margin = 50;
	_canvas_x_size = DISPLAY_X_SIZE - (2*_canvas_margin);
	_canvas_y_size = DISPLAY_Y_SIZE - (2*_canvas_margin);
	_canvas_y_mid = (_canvas_y_size/2) + _canvas_margin;
	_canvas_y_bottom = DISPLAY_Y_SIZE - (_canvas_margin) - 1;

	_grid_step_x = ((_canvas_x_size << N_FRACTIONAL_BITS)/N_GRID_STEPS_X) >> N_FRACTIONAL_BITS;
	_grid_step_y = ((_canvas_y_size << N_FRACTIONAL_BITS)/N_GRID_STEPS_Y) >> N_FRACTIONAL_BITS;

	_canvas_rect.x0  = _canvas_margin;
	_canvas_rect.y0  = _canvas_margin;
	_canvas_rect.xsz = _canvas_x_size;
	_canvas_rect.ysz = _canvas_y_size;

	display_fill_screen(_background_color);
	display_draw_rect(_canvas_color, _canvas_rect);
	draw_grid();
	display_swap_buffers();

	display_fill_screen(_background_color);
	display_draw_rect(_canvas_color, _canvas_rect);
	draw_grid();
	display_swap_buffers();

}


// set up a plot with a fixed array length
void plot_init_fast(int array_length, char* units) {
	plot_init();

	_x_pix_scaling = (_canvas_x_size << N_FRACTIONAL_BITS)/array_length;
	_x_n_samples = array_length;
	_y_pix_scaling = ((_canvas_y_size-1) << N_FRACTIONAL_BITS)/Y_MAX_VALUE;
}


void plot_draw_array(u32* array, u32 color) {
	u32 x_val = 0;
	u32 y_prev = (array[0] & Y_MAX_VALUE)*(_y_pix_scaling);
	Line line;
	for (int i = 0; i < (_x_n_samples-1); i++) {
		line.x0 = (x_val >> N_FRACTIONAL_BITS) + _canvas_margin;
		x_val += _x_pix_scaling;
		line.x1 = (x_val >> N_FRACTIONAL_BITS) + _canvas_margin;
		line.y0 = _canvas_y_bottom - (y_prev>>N_FRACTIONAL_BITS);
		y_prev = (array[i+1] & Y_MAX_VALUE);
		y_prev = y_prev* _y_pix_scaling;
		line.y1 = _canvas_y_bottom - (y_prev>>N_FRACTIONAL_BITS);
		display_draw_line(line, color);
	}
}


void draw_grid() {
	Rect line;
	line.x0 = _canvas_margin;
	line.xsz = _canvas_x_size;
	line.y0 = _canvas_margin;
	line.ysz = 1;
	for (int i = 0; i < N_GRID_STEPS_Y; i++) {
		display_draw_rect(_grid_color, line);
		line.y0 += _grid_step_y;
	}
	line.x0 = _canvas_margin;
	line.xsz = 1;
	line.y0 = _canvas_margin;
	line.ysz = _canvas_y_size;
	for (int i = 0; i < N_GRID_STEPS_X; i++) {
		display_draw_rect(_grid_color, line);
		line.x0 += _grid_step_x;
	}
}


/* plot_draw:
 *     "Unsafe" draw function which performs only offsetting.
 *     y=0 goes to the vertical middle of the canvas, x=0 goes to the left edge of the canvas.
 *	   Positive y is towards the top of the canvas.
 */

/*
void plot_blank() {
	display_fill_screen(_background_color);
	display_draw_rect(_canvas_color, _canvas_rect);
}
*/





void plot_char(int x, int y, int letterIndex, int magnification, int color){
	for (int i=0;i<16;i++) //row --O(1) in this line because i is constant
		for (int j=0;j<8;j++)//col --O(1) in this line because j is constant
			if (letter[letterIndex][i][j]==1)
				for (int k = 0; k < magnification; k++)
					for (int l = 0; l < magnification; l++)
						draw_point(x+j*magnification+k, y+i*magnification+l, color);
}

void parse_text(char *text, int x, int y, int magnification, int color){
	int i = 0;
	char ch = text[i];
	while(ch != '\0'){ //for text in 16*8 pixel
		plot_char(x + i * 10 * magnification,y,ch,magnification, color);
		i++;
		ch = text[i];
	}
}

void draw_channel_label(int x, int y){
	char ch1_text[] = "channel 1";
	char ch2_text[] = "channel 2";
	parse_text("*", x, y, 1, CH1_COLOR);
	parse_text(ch1_text, (x+8), y, 1, COLOR_WHITE);
	parse_text("*", x, (y + 20), 1, CH2_COLOR);
	parse_text(ch2_text, (x+8), (y+20), 1, COLOR_WHITE);

	//Rect rect = {x0, y0, xsz, ysz}
}

void draw_unit_label(int value, char* units, int color, int x, int y ) {


}



void plot_blank() {
	display_draw_rect(_canvas_color, _canvas_rect);
	draw_grid();
	//draw_channel_label();
}

void plot_draw(PointList* point_list, u32 color) {
	int length = point_list->length;

	for (int i = 0; i < (length-1); i++) {
		// here we are making a line from adjacent points.
		Point p0 = point_list->points[i];
		Point p1 = point_list->points[i+1];
		Line line;
		line.x0 = _canvas_margin + p0.x;
		line.y0 = _canvas_y_bottom  - p0.y; // y needs to be inverted
		line.x1 = _canvas_margin + p1.x;
		line.y1 = _canvas_y_bottom  - p1.y;
		display_draw_line(line, color);
	}
}

void plot_update() {
	display_swap_buffers();
}

static PointList square_wave;
static unsigned int square_phase, square_amplitude, square_period;
#define SQUARE_MAX_PERIOD 50
#define SQUARE_MAX_AMPLITUDE 200
#define SQUARE_NUM_POINTS 300


PointList* init_wave(int length) {
	PointList* wave;
	wave = malloc(sizeof(wave));
	wave->length = length;
	wave->points = (Point*) malloc(wave->length*sizeof(Point));
	for (int i=0; i < wave->length; i++) {
		wave->points[i].x = i;
		wave->points[i].y = 0;
	}
	return wave;
}


PointList* init_square() {
	int init_length = SQUARE_NUM_POINTS;
	square_wave.length = init_length;
	square_wave.points = (Point*) malloc(init_length*sizeof(Point));
	for (int i=0; i < init_length; i++) {
		square_wave.points[i].x = i;
		square_wave.points[i].y = 0;
	}

	square_phase = 0;
	square_amplitude = 20;
	square_period = 10;

	return &square_wave;
}

PointList* animate_square_freq() {
	int x_inc = _canvas_x_size/square_wave.length;
	int x = 0;
	for (int i = 0; i < square_wave.length; i++) {
		int positive_cycle = ((i - square_phase) % square_period) > (square_period >> 1);
		if (positive_cycle) {
			square_wave.points[i].y = square_amplitude;
		} else {
			square_wave.points[i].y = -square_amplitude;
		}
		x += x_inc;
		square_wave.points[i].x = x;
	}
	square_period = (square_period + 1) % SQUARE_MAX_PERIOD;
	return &square_wave;
}

PointList* animate_square_amp() {
	int x_inc = _canvas_x_size/square_wave.length;
	int x = 0;
	for (int i = 0; i < square_wave.length; i++) {
		int positive_cycle = ((i - square_phase) % square_period) > (square_period >> 1);
		if (positive_cycle) {
			square_wave.points[i].y = square_amplitude;
		} else {
			square_wave.points[i].y = -square_amplitude;
		}
		x += x_inc;
		square_wave.points[i].x = x;
	}
	square_amplitude = (square_amplitude + 1) % SQUARE_MAX_AMPLITUDE;
	return &square_wave;

}

PointList* animate_square_phase() {
	int x_inc = _canvas_x_size/square_wave.length;
	int x = 0;
	for (int i = 0; i < square_wave.length; i++) {
		int positive_cycle = ((i - square_phase) % square_period) > (square_period >> 1);
		if (positive_cycle) {
			square_wave.points[i].y = square_amplitude;
		} else {
			square_wave.points[i].y = -square_amplitude;
		}
		x += x_inc;
		square_wave.points[i].x = x;
	}
	square_phase = (square_phase + 1);
	return &square_wave;
}

#endif
