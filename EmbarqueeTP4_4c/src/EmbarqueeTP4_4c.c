#include <rtems.h>
#include <assert.h> // assert
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "windows-producer.h" // init(), produce_images(), get_mask()
#include "algo.h" // computeFluxPondere()
#include "buffer_circulaire_partagee.h" // init_buffer_circulaire_partagee()
#include "flux.h" // flux, FLUX_LENGTH

/* ********** MACROS ********** */
#define TASK_STACK_SIZE 6000

#define MESSAGE_QUEUE_COUNT 1
#define MESSAGE_QUEUE_SIZE 4

#define ACQUISITION_TASK_PRIORITY 1
#define PROCESSING_TASK_PRIORITY 2
#define TELEMETRY_MANAGER_TASK_PRIORITY 3

#define NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES 5

/* ********** RTEMS CONFIGURATION MACROS ********** */
// The RTEMS configuration macros must be located before #include <rtems/confdefs.h>
#define CONFIGURE_MAXIMUM_TIMERS 5
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 10
#define CONFIGURE_MAXIMUM_SEMAPHORES 3

/* ********** FUNCTIONS ********** */
rtems_task Init(rtems_task_argument argument);
rtems_task acquisition_task(rtems_task_argument argument);
rtems_task processing_task(rtems_task_argument argument);
rtems_task telemetry_manager_task(rtems_task_argument argument);

float get_elapsed_time();
void reload_timer(unsigned int arg1, void* arg2);
void break_point(float elapsed_time);
void send_flux(uint32_t time, flux f);


/* ********** GLOBAL VARIABLES ********** */
// Keep the names and IDs in global variables so another task can use them.
rtems_id acquisition_task_id, processing_task_id, telemetry_manager_task_id;
rtems_id timer_id, message_queue_synchronize_timer_acquisition_id, message_queue_synchronize_acquisition_processing_id;

rtems_name acquisition_task_name, processing_task_name, telemetry_manager_task_name;
rtems_name timer_name, message_queue_synchronize_timer_acquisition_name, message_queue_synchronize_acquisition_processing_name;

float img_buffer[NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES][36];
flux flux_pondere[NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES];
buffer_circulaire_partagee circular_buffer;

Windows_producer wp;

/* ******************************** */

#include "configuration.h" // Including the line : #include <rtems/confdefs.h>

/* ********** AUXILIARY FUNCTIONS ********** */

void break_point(float elapsed_time) {}

float get_elapsed_time() {
	static uint32_t last_time = 0;
	uint32_t elapsed_time = 0, current_time;

	// rtems_interval rtems_clock_get_ticks_since_boot( void );
	// Gets the number of clock ticks since some time point during the system initialization or the last overflow of the clock tick counter
	// Source : RTEMS Classic API Guide
	current_time = rtems_clock_get_ticks_since_boot();

	elapsed_time = current_time - last_time;
	last_time = current_time;

	return  (float)(elapsed_time) / rtems_clock_get_ticks_per_second();
}

void reload_timer(unsigned int arg1, void* arg2) {
	  rtems_status_code status;
	  float elapsed_time;
	  static uint32_t timer_count = 0;

	  status = rtems_timer_fire_after(timer_id, rtems_clock_get_ticks_per_second() / 5, reload_timer ,NULL);
	  assert(status == RTEMS_SUCCESSFUL);

	  timer_count++;

	  // rtems_status_code rtems_message_queue_send(rtems_id id, const void *buffer, size_t  size);
	  status = rtems_message_queue_send(message_queue_synchronize_timer_acquisition_id, &timer_count, sizeof(uint32_t));
	  if (status == RTEMS_SUCCESSFUL)
	  	  timer_count = 0;

	  elapsed_time = get_elapsed_time();
	  break_point(elapsed_time);
}

/**
* Simule l'émission d'une séquence de mesures de photométrie.
* @param f structure enregistrant la séquence
* @param time moment de l'émission, exprimé en ticks depuis le démarrage du programme .
*/
void send_flux(uint32_t time, flux f){
	static int cpt = 0 ;
	cpt++ ;
}

/* ********** TASKS ********** */

rtems_task acquisition_task(rtems_task_argument argument) {
	rtems_status_code status;
	uint32_t acquisition_counter = -1, timer_count;
	size_t size;
	// extern char init(Windows_producer* wp, float* img_buffer, unsigned int windows_max_number);
	// Initialize the windows producer. Must be call before produce_images()
	char init_result = init(&wp, (float*) &img_buffer, NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES);
	if (init_result == -1)
		exit(EXIT_FAILURE);

	while (1) {
		// rtems_status_code rtems_message_queue_receive(rtems_id id, void *buffer, size_t *size, rtems_option option_set, rtems_interval timeout);
		status = rtems_message_queue_receive(message_queue_synchronize_timer_acquisition_id, &timer_count, &size, RTEMS_WAIT, RTEMS_NO_TIMEOUT);

		if(status == RTEMS_SUCCESSFUL) {
			for(uint32_t i = 0 ; i < timer_count ; i++){
				// extern void produce_images(Windows_producer* wp );
				// Produce new windows for the same star but with an evolved brightness
				produce_images(&wp);
				acquisition_counter++;

			  	// rtems_status_code rtems_message_queue_send(rtems_id id, const void *buffer, size_t  size);
			  	status = rtems_message_queue_send(message_queue_synchronize_acquisition_processing_id, &acquisition_counter, sizeof(uint32_t));
			}
		}
	}
}

rtems_task processing_task(rtems_task_argument argument) {
	rtems_status_code status;
	uint32_t acquisition_counter;
	size_t size;
	while (1) {
		// rtems_status_code rtems_message_queue_receive(rtems_id id, void *buffer, size_t *size, rtems_option option_set, rtems_interval timeout);
		status = rtems_message_queue_receive(message_queue_synchronize_acquisition_processing_id, &acquisition_counter, &size, RTEMS_WAIT, RTEMS_NO_TIMEOUT);

		if(status == RTEMS_SUCCESSFUL) {
			 uint32_t i;
			 for (i = 0; i < NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES ; i++) {
				 float* mask = get_mask(&wp, i);
				 float* window = img_buffer[ flux_pondere[i].id_window ];

				 flux_pondere[i].measures[flux_pondere[i].id_first_acquisition + acquisition_counter] = computeFluxPondere(window, mask);
			 }
			 acquisition_counter++;
			 push_partagee(&circular_buffer, &flux_pondere[i], 1);
		}
	}
}

rtems_task telemetry_manager_task(rtems_task_argument argument) {
	flux buffer;
	int32_t result;

	while (1) {
		result = pop_partagee(&circular_buffer, &buffer, 1);
		if(result == 1) {
			send_flux(0, buffer);
		} else {
			rtems_task_wake_after(30);
		}
	}
}


/* ********** INIT ********** */

rtems_task Init(rtems_task_argument argument) {
  rtems_status_code status;

  for (uint32_t i = 0; i < NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES ; i++) {
	    flux_pondere[i].id_window = i;
	    flux_pondere[i].id_first_acquisition = 0;
  }

  init_buffer_circulaire_partagee(&circular_buffer, (flux *) &flux_pondere, 1, NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES);

  acquisition_task_name = rtems_build_name('A','C','U','I');
  processing_task_name  = rtems_build_name('P','R','O','C');
  telemetry_manager_task_name = rtems_build_name('T','E','L','E');
  timer_name = rtems_build_name('T','I','M','E');
  message_queue_synchronize_timer_acquisition_name = rtems_build_name('T','M','A', 'C');
  message_queue_synchronize_acquisition_processing_name = rtems_build_name('A','C','T', 'M');

  // rtems_status_code rtems_timer_create( rtems_name name, rtems_id *id );
  status = rtems_timer_create(timer_name, &timer_id);
  assert(status == RTEMS_SUCCESSFUL);

  // rtems_status_code rtems_timer_fire_after(rtems_id id, rtems_interval ticks, rtems_timer_service_routine_entry routine, void *user_data);
  status = rtems_timer_fire_after(timer_id, rtems_clock_get_ticks_per_second() / 5, reload_timer ,NULL);
  assert(status == RTEMS_SUCCESSFUL);

  // rtems_status_code rtems_message_queue_create(rtems_name name, uint32_t count, size_t max_message_size, rtems_attribute attribute_set, rtems_id *id);
  status = rtems_message_queue_create(message_queue_synchronize_timer_acquisition_name, MESSAGE_QUEUE_COUNT, MESSAGE_QUEUE_SIZE, RTEMS_LOCAL | RTEMS_PRIORITY, &message_queue_synchronize_timer_acquisition_id);
  assert(status == RTEMS_SUCCESSFUL);
  status = rtems_message_queue_create(message_queue_synchronize_acquisition_processing_name, MESSAGE_QUEUE_COUNT, MESSAGE_QUEUE_SIZE, RTEMS_LOCAL | RTEMS_PRIORITY, &message_queue_synchronize_acquisition_processing_id);
  assert(status == RTEMS_SUCCESSFUL);

  // rtems_status_code rtems_task_create(rtems_name name, rtems_task_priority initial_priority, size_t stack_size, rtems_mode initial_modes, rtems_attribute attribute_set, rtems_id *id);
  status = rtems_task_create(acquisition_task_name, ACQUISITION_TASK_PRIORITY, TASK_STACK_SIZE, RTEMS_PREEMPT | RTEMS_NO_TIMESLICE  | RTEMS_ASR | RTEMS_INTERRUPT_LEVEL(0), RTEMS_LOCAL | RTEMS_FLOATING_POINT, &acquisition_task_id);
  assert(status == RTEMS_SUCCESSFUL);
  status = rtems_task_create(processing_task_name, PROCESSING_TASK_PRIORITY, TASK_STACK_SIZE  * 2, RTEMS_PREEMPT | RTEMS_NO_TIMESLICE  | RTEMS_ASR | RTEMS_INTERRUPT_LEVEL(0), RTEMS_LOCAL | RTEMS_FLOATING_POINT, &processing_task_id);
  assert(status == RTEMS_SUCCESSFUL);
  status = rtems_task_create(telemetry_manager_task_name, TELEMETRY_MANAGER_TASK_PRIORITY, TASK_STACK_SIZE, RTEMS_PREEMPT | RTEMS_NO_TIMESLICE  | RTEMS_ASR | RTEMS_INTERRUPT_LEVEL(0), RTEMS_LOCAL | RTEMS_FLOATING_POINT, &telemetry_manager_task_id);
  assert(status == RTEMS_SUCCESSFUL);

  // rtems_status_code rtems_task_start(rtems_id id, rtems_task_entry entry_point, rtems_task_argument argument)
  status = rtems_task_start(acquisition_task_id, acquisition_task, 0);
  assert(status == RTEMS_SUCCESSFUL);
  status = rtems_task_start(processing_task_id, processing_task, 0);
  assert(status == RTEMS_SUCCESSFUL);
  status = rtems_task_start(telemetry_manager_task_id, telemetry_manager_task, 0);
  assert(status == RTEMS_SUCCESSFUL);

  status = rtems_task_delete( RTEMS_SELF );
  assert(status == RTEMS_SUCCESSFUL);
}
