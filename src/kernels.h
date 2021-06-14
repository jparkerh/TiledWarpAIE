#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H
#include "./RangeDescriptor.h"
#include "./WarpCoefficients.h"

void tiledWarpAndForward (input_stream_uint32 *in,
		output_stream_uint32 *out,
		output_stream_uint32 *processedOutStream,
		const int (&descriptor)[RD_SIZE],
		const int (&warpparams)[WC_SIZE]
		);

#endif
