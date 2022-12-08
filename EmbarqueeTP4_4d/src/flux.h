/*
 * flux.h
 */

#ifndef FLUX_H_
#define FLUX_H_

#define FLUX_LENGTH 10

typedef struct flux{
	uint32_t id_window ;
	uint32_t id_first_acquisition ; ///< measurement acquisition counter value[0]
	float measures[FLUX_LENGTH];
} flux ;

#endif /* FLUX_H_ */
