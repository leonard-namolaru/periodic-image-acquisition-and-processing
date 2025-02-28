
#ifndef ALGO_H_
#define ALGO_H_

#define WINDOW_SIZE 6

typedef struct{
	float x;
	float y;
}Point;

Point barycentre(float window[], float mask[]);

double computeFluxBinaire(float window[], float mask[]);

double computeFluxPondere(float window[], float mask[]);


#endif /* ALGO_H_ */
