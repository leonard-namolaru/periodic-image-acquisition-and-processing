/*
 * buffer_circulaire_partagee.c
 */
#include <rtems.h>
#include <assert.h> // assert
#include "buffer_circulaire_partagee.h"
#include "flux.h"

rtems_id semaphore_id;
rtems_name semaphore_name;

void init_buffer_circulaire_partagee(buffer_circulaire_partagee* b, uint8_t* buffer, uint16_t taille_donnee, uint32_t nombre_donnees) {
	rtems_status_code status;
	semaphore_name = rtems_build_name('S','E','M','A');

	init_buffer_circulaire(&(b->buffer), buffer, taille_donnee, nombre_donnees);

	 // rtems_status_code rtems_semaphore_create(rtems_name name, uint32_t count, rtems_attribute attribute_set, rtems_task_priority priority_ceiling, rtems_id* id);
	 status = rtems_semaphore_create(semaphore_name, 1, RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY | RTEMS_BINARY_SEMAPHORE, 0, &semaphore_id);
	 assert(status == RTEMS_SUCCESSFUL);

}

int push_partagee(buffer_circulaire_partagee* fifo, uint8_t* source, uint16_t taille) {
	rtems_status_code status;
	int result;
	// rtems_status_code rtems_semaphore_obtain(rtems_id id, rtems_option option_set, rtems_interval timeout);
	status = rtems_semaphore_obtain(semaphore_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
	assert(status == RTEMS_SUCCESSFUL);

	result =  push(&(fifo->buffer), source, taille);

	// rtems_status_code rtems_semaphore_release( rtems_id id );
	status = rtems_semaphore_release(semaphore_id);
	assert(status == RTEMS_SUCCESSFUL);

	return result;
}


int pop_partagee(buffer_circulaire_partagee* fifo, uint8_t* destination, uint16_t taille_max) {
	rtems_status_code status;
	int result;

	// rtems_status_code rtems_semaphore_obtain(rtems_id id, rtems_option option_set, rtems_interval timeout);
	status = rtems_semaphore_obtain(semaphore_id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
	assert(status == RTEMS_SUCCESSFUL);

	result =  pop(&(fifo->buffer), destination, taille_max);

	// rtems_status_code rtems_semaphore_release( rtems_id id );
	status = rtems_semaphore_release(semaphore_id);
	assert(status == RTEMS_SUCCESSFUL);

	return result;
}
