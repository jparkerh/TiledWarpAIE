#ifndef WARPCOEFF_H
#define WARPCOEFF_H

#define WC_SIZE 6

struct WarpCoefficients {
	int coefficients[6];

	WarpCoefficients (int coefficientInput[6]) {
		for (int c=0; c<6; c++)
			coefficients[c] = coefficientInput[c];
	}

	int getCoefficient (int row, int col) {
		return coefficients[(row * 3) + col];
	}
};

#endif
