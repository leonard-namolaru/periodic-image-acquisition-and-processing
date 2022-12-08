/*
 * buffer_circulaire_partagee.c
 */
#include <rtems.h>
#include <assert.h> // assert
#include "buffer_circulaire_partagee.h"
#include "flux.h"

rtems_id semaphore_id;
rtems_name semaphore_name;

void init_buffer_circulaire_partagee(buffer_circulaire_partagee* b, flux* buffer, uint16_t taille_donnee, uint32_t nombre_donnees) {
	rtems_status_code status;
	semaphore_name = rtems_build_name('S','E','M','A');

	init_buffer_circulaire(&(b->buffer), buffer, taille_donnee, nombre_donnees);

	 // rtems_status_code rtems_semaphore_create(rtems_name name, uint32_t count, rtems_attribute attribute_set, rtems_task_priority priority_ceiling, rtems_id* id);
	 status = rtems_semaphore_create(semaphore_name, 1, RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY | RTEMS_BINARY_SEMAPHORE, 0, &semaphore_id);
	 assert(status == RTEMS_SUCCESSFUL);

}

int push_partagee(buffer_circulaire_partagee* fifo, flux* source, uint16_t taille) {
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


int pop_partagee(buffer_circulaire_partagee* fifo, flux* destination, uint16_t taille_max) {
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

unsigned int test_unitaire_buffer_circulaire_partagee() {
	unsigned int res = 0;
	int tmp;
	unsigned int i = 0;
	static const char TAILLE = 2;
	static const char NB_DONNEES = 3;
	flux buffer_fifo[NB_DONNEES][TAILLE];
	flux buffer[TAILLE * 2];

	buffer_circulaire_partagee bc;
	init_buffer_circulaire_partagee(&bc, (flux*) &buffer_fifo, TAILLE, NB_DONNEES);

	for (i = 0; i < TAILLE; i++) {
		for (tmp = 0; tmp < NB_DONNEES; tmp++) {
			buffer_fifo[i][tmp].id_window = 0;
		}
	}
	for (i = 0; i < TAILLE * 2; i++) {
		buffer[i].id_window = 0;
	}

	i = 0;

	// pop sur fifo vide
	if (res == 0) {
		i++;
		if (-2 != pop_partagee(&bc, buffer, TAILLE) || bc.buffer.occupation != 0 || bc.buffer.occupation_max != 0) {
			res = i;
		}
	}

	// donnee trop grande
	if (res == 0) {
		i++;
		buffer[0].id_window = 0x11;
		buffer[1].id_window = 0x22;
		buffer[2].id_window = 0x33;
		tmp = push_partagee(&bc, buffer, TAILLE + 1);
		if (-1 != tmp || buffer_fifo[0][0].id_window != 0 || bc.buffer.occupation != 0 || bc.buffer.occupation_max != 0) {
			res = i;
		}
	}

	// push OK
	if (res == 0) {
		i++;
		buffer[0].id_window = 0x11;
		buffer[1].id_window = 0x22;
		tmp = push_partagee(&bc, buffer, TAILLE);
		if (0 != tmp || buffer_fifo[0][0].id_window != 0x11 || buffer_fifo[0][1].id_window != 0x22 || bc.buffer.occupation != 1 || bc.buffer.occupation_max != 1) {
			res = i;
		}
	}

	// pop avec buffer trop petit
	if (res == 0) {
		i++;
		buffer[0].id_window = 0;
		buffer[1].id_window = 0;
		buffer[2].id_window = 0;
		if (-1 != pop_partagee(&bc, buffer, TAILLE - 1) || buffer[0].id_window != 0 || buffer[0].id_window != 0 || bc.buffer.occupation != 1 || bc.buffer.occupation_max != 1) {
			res = i;
		}
	}

	// pop OK
	if (res == 0) {
		i++;
		buffer[0].id_window = 0;
		buffer[1].id_window = 0;
		buffer[2].id_window = 0;
		tmp = pop_partagee(&bc, buffer, TAILLE);
		if (2 != tmp || buffer[0].id_window != 0x11 || buffer[1].id_window != 0x22 || bc.buffer.occupation != 0 || bc.buffer.occupation_max != 1) {
			res = i;
		}
	}

	// pop une fifo vide
	if (res == 0) {
		i++;
		buffer[0].id_window = 0;
		buffer[1].id_window = 0;
		buffer[2].id_window = 0;
		tmp = pop_partagee(&bc, buffer, TAILLE);
		if (-2 != tmp || buffer[0].id_window != 0 || buffer[1].id_window != 0 || bc.buffer.occupation != 0 || bc.buffer.occupation_max != 1) {
			res = i;
		}
	}

	//push 3 fois avec succès
	if (res == 0) {
		i++;
		buffer[0].id_window = 0x11;
		buffer[1].id_window = 0x12;
		tmp = push_partagee(&bc, buffer, TAILLE);
		res = buffer_fifo[1][0].id_window;
		res = buffer_fifo[1][1].id_window;
		res = 0;
		if (0 != tmp || buffer_fifo[1][0].id_window != 0x11 || buffer_fifo[1][1].id_window != 0x12 || bc.buffer.occupation != 1 || bc.buffer.occupation_max != 1) {
			res = i;
		}
	}
	if (res == 0) {
		i++;
		buffer[0].id_window = 0x13;
		buffer[1].id_window = 0x14;
		if (0 != push_partagee(&bc, buffer, TAILLE) || buffer_fifo[2][0].id_window != 0x13 || buffer_fifo[2][1].id_window != 0x14|| bc.buffer.occupation != 2 || bc.buffer.occupation_max != 2) {
			res = i;
		}
	}
	if (res == 0) {
		i++;
		buffer[0].id_window = 0x17;
		buffer[1].id_window = 0x18;
		if (0 != push_partagee(&bc, buffer, TAILLE) || buffer_fifo[0][0].id_window != 0x17 || buffer_fifo[0][1].id_window != 0x18|| bc.buffer.occupation != 3 || bc.buffer.occupation_max != 3) {
			res = i;
		}
	}

	// push sur une fifo pleine
	if (res == 0) {
		i++;
		buffer[0].id_window = 0x19;
		buffer[1].id_window = 0x1a;
		if (-2 != push_partagee(&bc, buffer, TAILLE)|| bc.buffer.occupation != 3 || bc.buffer.occupation_max != 3) {
			res = i;
		}
	}

	return res;
}
