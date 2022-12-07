
#include "algo.h"
#include <stdint.h>

Point barycentre(float window[], float mask[]) {
	double total = 0;
	double tx = 0;
	double ty = 0;
	for (uint8_t xi = 0; xi < WINDOW_SIZE; xi++) {
		for (uint8_t  yi = 0; yi < WINDOW_SIZE; yi++) {
			if (mask[xi * WINDOW_SIZE + yi] > 0) {
				total += window[xi * WINDOW_SIZE + yi];
				tx += window[xi * WINDOW_SIZE + yi] * (xi + 1);
				ty += window[xi * WINDOW_SIZE + yi] * (yi + 1);
			}
		}
	}
	Point res;
	res.x = (1.0 * tx / total) - 1;
	res.y = (1.0 * ty / total) - 1;

	return res;
}

double computeFluxBinaire(float window[], float mask[]) {
	double res = 0;
	for (uint8_t xi = 0; xi < WINDOW_SIZE; xi++) {
		for (uint8_t yi = 0; yi < WINDOW_SIZE; yi++) {
			if (mask[xi * WINDOW_SIZE + yi] > 0) {
				res += window[xi * WINDOW_SIZE + yi];
			}
		}
	}
	return res;
}

double computeFluxPondere(float window[], float mask[]) {
	double res = 0;
	for (uint8_t xi = 0; xi < WINDOW_SIZE; xi++) {
		for (uint8_t yi = 0; yi < WINDOW_SIZE; yi++) {
			res += window[xi * WINDOW_SIZE + yi] * mask[xi * WINDOW_SIZE + yi];
		}
	}
	return res;
}
