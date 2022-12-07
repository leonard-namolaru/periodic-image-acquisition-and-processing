/*
 * buffer_circulaire_partagee.h
 */

#ifndef BUFFER_CIRCULAIRE_PARTAGEE_H_
#define BUFFER_CIRCULAIRE_PARTAGEE_H_

#include "buffer_circulaire.h"

typedef struct buffer_circulaire_partagee {
	buffer_circulaire buffer;
} buffer_circulaire_partagee;

void init_buffer_circulaire_partagee(buffer_circulaire_partagee* b, flux* buffer, uint16_t taille_donnee, uint32_t nombre_donnees);
int push_partagee(buffer_circulaire_partagee* fifo, flux* source, uint16_t taille);
int pop_partagee(buffer_circulaire_partagee* fifo, flux* destination, uint16_t taille_max);
unsigned int test_unitaire_buffer_circulaire_partagee();

#endif /* BUFFER_CIRCULAIRE_PARTAGEE_H_ */
