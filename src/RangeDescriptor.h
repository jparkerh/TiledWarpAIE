#ifndef RANGEDESC_H
#define RANGEDESC_H

#define RD_SIZE 6

struct RangeDescriptor {
	int x_min;
	int x_max;
	int y_min;
	int y_max;
	int width;
	int height;

	RangeDescriptor(int x_min, int x_max, int y_min, int y_max, int width, int height):
		x_min(x_min),
		x_max(x_max),
		y_min(y_min),
		y_max(y_max),
		width(width),
		height(height) {}
};

#endif
