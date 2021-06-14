/**
 * Copyright 2021 P Holloway
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
 * permit persons to whom the Software is furnished to do so, subject to the following 
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE 
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <adf.h>
#include "../kernels.h"

void tiledWarpAndForward (input_stream_uint32 *in,
		output_stream_uint32 *out,
		output_stream_uint32 *processedOutStream,
		const int (&descriptor)[RD_SIZE],
		const int (&warpparams)[WC_SIZE]
)
{
	RangeDescriptor *filter = (RangeDescriptor*)&descriptor;
	WarpCoefficients *warpcoeffs = (WarpCoefficients*)&warpparams;

	/**
	 * 		prepare coefficient vectors for srcLoc fmacs
	 * 		a: v16float [ c00, c10, 0, 0 ];
	 * 		b: v16float [ c01, c11, 0, 0 ];
	 * 		c: v16float [ c02, c12, 0, 0 ];
	 * 		initialize d, e, f with zeros first (fix d/e later)
	 */

	v16float a = null_v16float();
	v16float b = null_v16float();
	v16float c = null_v16float();
	v8float d = null_v8float();
	v8float e = null_v8float();
	v8float f = null_v8float();
	float c00 = warpcoeffs->getCoefficient(0, 0);
	float c01 = warpcoeffs->getCoefficient(0, 1);
	float c02 = warpcoeffs->getCoefficient(0, 2);
	float c10 = warpcoeffs->getCoefficient(1, 0);
	float c11 = warpcoeffs->getCoefficient(1, 1);
	float c12 = warpcoeffs->getCoefficient(1, 2);
	a = upd_elem(a, 0, c00, c10);
	b = upd_elem(b, 0, c01, c11);
	c = upd_elem(c, 0, c02, c12);

	int filterWidth = (filter->x_max-filter->x_min);
	int filterHeight = (filter->y_max-filter->y_min);

	uint32_t data[7068];

	// Use the range descriptor to store the data for this tile
	for (int row = 0; row < filter->height; row++) {
		for (int col = 0; col < filter->width; col++)
		chess_prepare_for_pipelining
		{
			uint32 pixel = readincr(in);

			// Descriptor filters to memory
			if (row < filter->y_max &&
					row >= filter->y_min &&
					col < filter->x_max &&
					col >= filter->x_min) {
				data[((row - filter->y_min) * filterWidth) + (col - filter->x_min)] = pixel;
			}

			// Pass on all pixels to the next tile in the chain
			writeincr(out, pixel);
		}
	}

	// Warp transform in order of resultant image
	// If source pixels exist in the data buffer, apply warp
	for (int row = 0; row < filter->height; row++) {
		for (int col = 0; col < filter->width; col++)
		chess_prepare_for_pipelining
		{

			v8int32	topHalf;
			v8int32 bottomHalf;

			/**
			 * x' = c00* x + c01* y + c02
			 * y' = c10* x + c11* y + c12
			 *
			 * Find srcPixel (srcRow, srcCol)
			 *
			 * setup d and e lanes 0&1 now
			 *
			 * 		d: v8float [ col, col, 0, 0 ];
			 * 		e: v8float [ row, row, 0, 0 ];
			 */

			v8float coords;
			d = upd_elem(d, 0, (float)col, (float)col);
			e = upd_elem(e, 0, (float)row, (float)row);
			coords = fpmac(coords, a, 0, 0, d, 0, 0);
			coords = fpmac(coords, b, 0, 0, e, 0, 0);
			coords = fpmac(coords, c, 0, 0, f, 0, 0);

			/**
			 * TODO: this code needs to be moved onto the
			 * vector processor. Currently sub-optimal on the
			 * scalar engine
			 */
			float srcRowFloat = ext_elem(coords, 0);
			float srcColFloat = ext_elem(coords, 1);
			int srcRow = (int)srcRowFloat;
			int srcCol = (int)srcColFloat;

			float x = srcRowFloat - (float)srcRow;
			float y = srcColFloat - (float)srcCol;

			float p00 = (1-x) * (1-y);
			float p01 = x * (1-y);
			float p10 = (1-x) * y;
			float p11 = x * y;

			v8float percentages = null_v8float();

			if (srcRow < filter->y_max-1 &&
					srcRow >= filter->y_min &&
					srcCol < filter->x_max-1 &&
					srcCol >= filter->x_min) {

				topHalf = 		*(v8int32*)data[((srcRow - filter->y_min) * filterWidth)
												+ (srcCol - filter->x_min)];
				bottomHalf = 	*(v8int32*)data[((srcRow - filter->y_min) * filterWidth)
												+ (srcCol - filter->x_min)];

				/**
				 * bilinear interpolation of the desired pixel
				 *
				 * t1: v16uint8 [s(0,0), s(0,1), .... ]
				 * t2: v16uint8 [s(1,0), s(1,1), .... ]
				 * percentages: v8float  [p(0,0), p(0,1), p(1,0), p(1,1)]
				 */

				// accum1 = [s(0,0)*p(0,0), s(0,1)*p(0,1), ....]
				v8float t1 = fix2float(topHalf);
				v8float accum1 = null_v8float();
				v16float t1_v16 = xset_w(0, t1);
				accum1 = fpmul (t1_v16, 0, 0, percentages, 0, 0);

				// accum2 = [s(1,0)*p(1,0), s(1,1)*p(1,1), ....]
				v8float t2 = fix2float(bottomHalf);
				v8float accum2 = null_v8float();
				v16float t2_v16 = xset_w(0, t2);
				accum2 = fpmul (t2_v16, 0, 0, percentages, 2, 0);

				// pixel = [(s(0,0)*p(0,0) + s(1,0)*p(1,0)), (s(0,1)*p(0,1) + s(1,1)*p(1,1))]
				v8float pixel = null_v8float();
				v16float accum2_v16 = xset_w(0, accum2);
				pixel = fpadd (accum1, accum2_v16, 0, 0);

				// pixel = [(s(0,0)*p(0,0) + s(1,0)*p(1,0) + s(0,1)*p(0,1) + s(1,1)*p(1,1))]
				v16float pixel_v16 = xset_w(0, pixel);
				pixel = fpadd (pixel, pixel_v16, 1, 0);

//				uint32_t output = (uint32_t)ext_elem(pixel, 0);
				writeincr(processedOutStream, (uint32_t)ext_elem(pixel, 0), 0);

			}
		}
	}
}
