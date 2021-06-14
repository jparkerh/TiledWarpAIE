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

#include "graph.h"
adf::PLIO *attr_i_0 = new adf::PLIO("TestLogicalNameOut1",
		adf::plio_128_bits,
		"data/input.txt");

adf::PLIO *attr_o_0 = new adf::PLIO("TestLogicalNameOut2",
		adf::plio_128_bits,
		"data/output1.txt");

adf::PLIO *attr_o_1 = new adf::PLIO("TestLogicalNameOut3",
		adf::plio_128_bits,
		"data/output2.txt");

adf::PLIO *attr_o_2 = new adf::PLIO("TestLogicalNameOut4",
		adf::plio_128_bits,
		"data/output3.txt");

adf::PLIO *attr_o_3 = new adf::PLIO("TestLogicalNameOut5",
		adf::plio_128_bits,
		"data/output4.txt");

adf::PLIO *attr_o_chain = new adf::PLIO("TestLogicalNameOut6",
		adf::plio_128_bits,
		"data/output5.txt");

adf::simulation::platform<1, 5> platform(attr_i_0, attr_o_0, attr_o_1, attr_o_2, attr_o_3, attr_o_chain);

TiledWarp tiledwarp;

adf::connect<> netIn(platform.src[0], tiledwarp.in);
adf::connect<> net0(tiledwarp.processedOut[0], platform.sink[0]);
adf::connect<> net1(tiledwarp.processedOut[1], platform.sink[1]);
adf::connect<> net2(tiledwarp.processedOut[2], platform.sink[2]);
adf::connect<> net3(tiledwarp.processedOut[3], platform.sink[3]);
adf::connect<> netChain(tiledwarp.out, platform.sink[4]);

int main(int argc, char ** argv) 
{ 
	tiledwarp.init();
	tiledwarp.run(1);
	tiledwarp.end();
	return 0;
}
