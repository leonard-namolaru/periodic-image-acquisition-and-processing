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
#include "configuration.h"

/* ********** FUNCTIONS ********** */
rtems_task Init(rtems_task_argument argument);
rtems_task acquisition_task(rtems_task_argument argument);

/* ********** GLOBAL VARIABLES ********** */
// Keep the names and IDs in global variables so another task can use them.
extern rtems_id task_id;
extern rtems_name task_name;

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

rtems_id task_id;
rtems_name task_name;

float img_buffer[NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES][36];

rtems_task Init(rtems_task_argument argument) {
  rtems_status_code status;

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

	return  (float)(elapsed_time) / 100.f; // rtems_clock_get_ticks_per_second() == 100
}

void break_point(float elapsed_time) {}

rtems_task acquisition_task(rtems_task_argument argument) {
	Windows_producer wp;
	rtems_status_code status;
	float elapsed_time;

	// extern char init(Windows_producer* wp, float* img_buffer, unsigned int windows_max_number);
	// Initialize the windows producer. Must be call before produce_images()
	char init_result = init(&wp, img_buffer, NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES);
	if (init_result == -1)
		exit(EXIT_FAILURE);

	while (1) {
		// extern void produce_images(Windows_producer* wp );
		// Produce new windows for the same star but with an evolved brightness
		produce_images(&wp);

		// rtems_status_code rtems_task_wake_after( rtems_interval ticks );
		// Wakes up after an interval in clock ticks or yields the processor.
		// A system tick worth here  10 ms: i.e. the hardware timer associated with the time manager of the
		// RTOS is set to issue an interrupt every 10 ms. (Sources: Lesson 6, RTEMS Classic API Guide)
	    status = rtems_task_wake_after( 50 ); // 50 ticks = 500 ms
	    assert(status == RTEMS_SUCCESSFUL);

	    elapsed_time = get_elapsed_time();
	    break_point(elapsed_time);
	}

}
