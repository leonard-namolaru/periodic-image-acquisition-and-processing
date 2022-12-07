/*  Test_task
 *
 *  This routine serves as a test task.  It verifies the basic task
 *  switching capabilities of the executive.
 *
 *  Input parameters:  NONE
 *
 *  Output parameters:  NONE
 *
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id: tasks.c,v 1.7.2.1 2000/05/05 12:58:06 joel Exp $
 */

#include <rtems.h>
#include <assert.h>

/* ********** RTEMS CONFIGURATION MACROS ********** */
// The RTEMS configuration macros must be located before #include <rtems/confdefs.h>
#define CONFIGURE_MAXIMUM_TIMERS 5
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 10

/* ********** FUNCTIONS ********** */
rtems_task Init(rtems_task_argument argument);
rtems_task acquisition_task(rtems_task_argument argument);

/* ********** GLOBAL VARIABLES ********** */
// Keep the names and IDs in global variables so another task can use them.
extern rtems_id task_id;
extern rtems_name task_name;

#include "configuration.h" // Incluant la ligne : #include <rtems/confdefs.h>

/* ********** HANDY MACROS AND STATIC INLINE FUNCTIONS ********** */

// Macro to hide the ugliness of printing the time.
#define print_time(_s1, _tb, _s2) \
  do { \
    iprintf( "%s%02"PRIu32":%02"PRIu32":%02"PRIu32"   %02"PRIu32"/%02"PRIu32"/%04"PRIu32"%s", \
       _s1, (_tb)->hour, (_tb)->minute, (_tb)->second, \
       (_tb)->month, (_tb)->day, (_tb)->year, _s2 ); \
    fflush(stdout); \
  } while ( 0 )

//  Macro to print an task name that is composed of ASCII characters.
#define put_name( _name, _crlf ) \
  do { \
    uint32_t c0, c1, c2, c3; \
    \
    c0 = ((_name) >> 24) & 0xff; \
    c1 = ((_name) >> 16) & 0xff; \
    c2 = ((_name) >> 8) & 0xff; \
    c3 = (_name) & 0xff; \
    putchar( (char)c0 ); \
    if ( c1 ) putchar( (char)c1 ); \
    if ( c2 ) putchar( (char)c2 ); \
    if ( c3 ) putchar( (char)c3 ); \
    if ( (_crlf) ) \
      putchar( '\n' ); \
  } while (0)


// This allows us to view the "Test_task" instantiations as a set
// of numbered tasks by eliminating the number of application
// tasks created.

// In reality, this is too complex for the purposes of this
// example.  It would have been easier to pass a task argument. :)
// But it shows how rtems_id's can sometimes be used.
#define task_number( tid ) \
  ( rtems_object_id_get_index( tid ) - \
     rtems_configuration_get_rtems_api_configuration()->number_of_initialization_tasks )


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "windows-producer.h" // init(), produce_images(), get_mask()

#define TASK_STACK_SIZE 10240
#define NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES 100
#define MESSAGE_QUEUE_COUNT 5
#define MESSAGE_QUEUE_SIZE 4

rtems_id task_id, timer_id, message_queue_id;
rtems_name task_name, timer_name;

float img_buffer[NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES][36];

float get_elapsed_time();
void break_point(float elapsed_time);

void reload_timer(unsigned int arg1, void* arg2) {
	  rtems_status_code status;
	  float elapsed_time;
	  uint32_t buffer;

	  status = rtems_timer_fire_after(timer_id, rtems_clock_get_ticks_per_second() / 2, reload_timer ,NULL);
	  assert(status == RTEMS_SUCCESSFUL);

	  do{
		  buffer = 1;
		  // rtems_status_code rtems_message_queue_send(rtems_id id, const void *buffer, size_t  size);
		  status = rtems_message_queue_send(message_queue_id, buffer, sizeof(uint32_t));
	  }while(status == RTEMS_TOO_MANY);

	  elapsed_time = get_elapsed_time();
	  break_point(elapsed_time);
}

rtems_task Init(rtems_task_argument argument) {
  rtems_status_code status;

  // rtems_timer_create()
  // rtems_status_code rtems_timer_create( rtems_name name, rtems_id *id );
  // This directive creates a timer which resides on the local node. The timer has the user-defined
  // object name specified in name. The assigned object identifier is returned in id. This identifier is
  // used to access the timer with other timer related directives.
  // Source : RTEMS Classic API Guide
  timer_name = rtems_build_name('T','I','M','E');
  status = rtems_timer_create(timer_name, &timer_id);
  assert(status == RTEMS_SUCCESSFUL);

  // rtems_status_code rtems_timer_fire_after(rtems_id id, rtems_interval ticks, rtems_timer_service_routine_entry routine, void *user_data);
  // Fires (starts) the timer after the interval.
  // This directive initiates the timer specified by id. If the timer is running, it is automatically
  // canceled before being initiated. The timer is scheduled to fire after an interval of clock ticks
  // has passed specified by ticks. When the timer fires, the timer service routine routine will be
  // invoked with the argument user_data in the context of the clock tick ISR.
  status = rtems_timer_fire_after(timer_id, rtems_clock_get_ticks_per_second() / 2, reload_timer ,NULL);
  assert(status == RTEMS_SUCCESSFUL);

  // rtems_status_code rtems_message_queue_create(rtems_name name, uint32_t count, size_t max_message_size, rtems_attribute attribute_set, rtems_id *id);
  status = rtems_message_queue_create(rtems_build_name('M','S','Q', '1'),
  MESSAGE_QUEUE_COUNT, MESSAGE_QUEUE_SIZE,
  RTEMS_LOCAL | RTEMS_PRIORITY, &message_queue_id);
  assert(status == RTEMS_SUCCESSFUL);


  task_name = rtems_build_name('T','A','S','K');

  // rtems_status_code rtems_task_create(
  // rtems_name name,
  // rtems_task_priority initial_priority,
  // size_t stack_size, rtems_mode initial_modes,
  // rtems_attribute attribute_set,
  // rtems_id *id
  // );
  status = rtems_task_create(task_name, 1, TASK_STACK_SIZE,
  RTEMS_PREEMPT | RTEMS_NO_TIMESLICE | RTEMS_INTERRUPT_LEVEL(0),
  RTEMS_LOCAL | RTEMS_FLOATING_POINT, &task_id);

  assert(status == RTEMS_SUCCESSFUL);

  // rtems_status_code rtems_task_start(rtems_id id, rtems_task_entry entry_point, rtems_task_argument argument)
  status = rtems_task_start(task_id, acquisition_task, 1 );
  assert(status == RTEMS_SUCCESSFUL);

  status = rtems_task_delete( RTEMS_SELF );
  assert(status == RTEMS_SUCCESSFUL);
}
#include <stdio.h>

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

void break_point(float elapsed_time) {}

rtems_task acquisition_task(rtems_task_argument argument) {
	Windows_producer wp;
	rtems_status_code status;
	uint32_t buffer;
	size_t size;

	// extern char init(Windows_producer* wp, float* img_buffer, unsigned int windows_max_number);
	// Initialize the windows producer. Must be call before produce_images()
	char init_result = init(&wp, img_buffer, NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES);
	if (init_result == -1)
		exit(EXIT_FAILURE);

	while (1) {
		// rtems_status_code rtems_message_queue_receive(rtems_id id, void *buffer, size_t *size, rtems_option option_set, rtems_interval timeout);
		status = rtems_message_queue_receive(message_queue_id,
		&buffer, &size, RTEMS_WAIT, RTEMS_NO_TIMEOUT);


		if(status == RTEMS_SUCCESSFUL) {
			// extern void produce_images(Windows_producer* wp );
			// Produce new windows for the same star but with an evolved brightness
			produce_images(&wp);
		}

	}
}
