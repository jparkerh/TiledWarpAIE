// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689

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
