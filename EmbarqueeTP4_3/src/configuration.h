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

/* ********** CONFIGURATION INFORMATION ********** */
#include <bsp.h> // For device driver prototypes

#define CONFIGURE_INIT
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_TASKS              4

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)

#include <rtems/confdefs.h>

/* If --drvmgr was enabled during the configuration of the RTEMS kernel */
#ifdef RTEMS_DRVMGR_STARTUP
 #ifdef LEON3
  /* Add Timer and UART Driver for this example */
  #ifdef CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
   #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
  #endif
  #ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
   #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
  #endif

  #include <grlib/ambapp_bus.h>
  /* OPTIONAL FOR GRLIB SYSTEMS WITH GPTIMER AS SYSTEM CLOCK TIMER */
  struct drvmgr_key grlib_drv_res_gptimer0[] =
  {
  	/* If all timers should not be used (typically on an AMP system, or when timers
  	 * are used customly in a project) one can limit to a range of timers.
  	 * timerStart: start of range (0..6)
  	 * timerCnt: Number of timers
  	 */
	#if 0
  	{"timerStart", DRVMGR_KT_INT, {(unsigned int)SET_START}},
  	{"timerCnt", DRVMGR_KT_INT, {(unsigned int)SET_NUMBER_FO_TIMERS}},
	  /* Select Prescaler (Base frequency of all timers on a timer core) */
  	{"prescaler", DRVMGR_KT_INT, {(unsigned int)SET_PRESCALER_HERE}},
  	/* Select which timer should be used as the system clock (default is 0) */
  	{"clockTimer", DRVMGR_KT_INT, {(unsigned int)TIMER_INDEX_USED_AS_CLOCK}},
  	#endif
  	DRVMGR_KEY_EMPTY
  };

  /* Use GPTIMER core 4 (not present in most systems) as a high
   * resoulution timer */
  struct drvmgr_key grlib_drv_res_gptimer4[] =
  {
  	{"prescaler", DRVMGR_KT_INT, {(unsigned int)4}},
  	DRVMGR_KEY_EMPTY
  };

  struct drvmgr_bus_res grlib_drv_resources =
  {
    .next = NULL,
    .resource = {
  #if 0
  	{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 0, &grlib_drv_res_gptimer0[0]},
  	{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 1, NULL}, /* Do not use timers on this GPTIMER core */
  	{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 2, NULL}, /* Do not use timers on this GPTIMER core */
  	{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 3, NULL}, /* Do not use timers on this GPTIMER core */
  	{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 4, &grlib_drv_res_gptimer4[0]},
  #endif
  	DRVMGR_RES_EMPTY
    }
  };
 #endif

 #include <drvmgr/drvmgr_confdefs.h>
#endif


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
