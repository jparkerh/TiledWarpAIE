#include <adf.h>
#include <vector>
#include "kernels.h"

#define NUM_TILES 4

/*
 * --- Tiled Warp ---
 * 1. Using 4 tiles, sends pixels along the chain
 * 2. Each tile stores it's pixels to memory
 * 3. Each tile performs the compute, outputting
 * 		the values that it can via its stored data
 * 4. Streaming HLS function reorders output pixels
 * 		in the programmable logic
 *
 * 	Operates on 24-bit monochrome pixels, each pixel is aligned
 * 	to 32-bits in the input stream (8-bit padding)
 */

class TiledWarp : public adf::graph
{
private:
	std::vector < adf::kernel > tilers;

public:
	adf::port<input> in;
	adf::port<input> config[NUM_TILES];
	adf::port<output> out;
	adf::port<output> processedOut[NUM_TILES];

	TiledWarp()
	{
		tilers.push_back(adf::kernel::create(tiledWarpAndForward));
		adf::connect< adf::stream > (in, tilers.back().in[0]);

		for (int tile=0; tile < NUM_TILES-1; tile++) {
			tilers.push_back(adf::kernel::create(tiledWarpAndForward));

			adf::connect< adf::stream > (tilers[tilers.size()-2].out[0], tilers.back().in[0]);
			adf::connect< adf::parameter > (config[tile+1], tilers[tile].in[1]);
		}

		adf::connect< adf::stream > (tilers.back().out[0], out);

		for (int tile=0; tile < NUM_TILES; tile++) {
			adf::connect< adf::stream > (tilers[tile].out[1], processedOut[tile]);
			adf::source(tilers[tile]) = "kernels/kernels.cc";
			adf::runtime<ratio>(tilers[tile]) = 0.1;
		}
	}
};

