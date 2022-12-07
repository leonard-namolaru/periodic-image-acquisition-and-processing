/*
 ============================================================================
 Name        : EmbarqueeTP3_3.c
 ============================================================================
 */

#include <stdio.h>
#include <stdint.h> // uint32_t
#include <stdlib.h> // exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <bcc/bcc.h> // bcc_isr_register()
#include "windows-producer.h" // init(), produce_images(), get_mask()
#include "modifier_registre.h" // modifier_registre_1()
#include "algo.h" // computeFluxPondere()

#define INTERRUPT_MASK_REGISTER 0x80000240
volatile uint32_t* interrupt_mask_register = (uint32_t*) INTERRUPT_MASK_REGISTER;

#define NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES 5
#define NB_CALLS_TO_PRODUCE_IMAGES 10

#define TIMER_2_COUNTER_VALUE_REGISTER 0x80000320
volatile uint32_t* timer_2_counter_value_register = (uint32_t*) TIMER_2_COUNTER_VALUE_REGISTER;

// Timer control register (Source : https://www.gaisler.com/doc/gr712rc-usermanual.pdf , p. 92)
// 31: 7 Reserved. Always reads as ‘000...0’.
#define DH 6 // Debug Halt : Value of GPTI.DHALT signal which is used to freeze counters (e.g. when a system is in debug mode). Read-only.
#define CH 5 // Chain : Chain with preceding timer. If set for timer n, timer n will be decremented each time when timer (n-1) underflows.
#define IP 4 // Interrupt Pending (IP): The core sets this bit to ‘1’ when an interrupt is signalled. This bit remains ‘1’ until cleared by writing ‘0’ to this bit.
#define IE 3 // Interrupt Enable : If set the timer signals interrupt when it underflows.
#define LD 2 // Load : Load value from the timer reload register to the timer counter value register.
#define RS 1 // Restart : If set, the timer counter value register is reloaded with the value of the reload register when the timer underflows
#define EN 0 //Enable : Enable the timer.

volatile float flux_pondere[NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES][NB_CALLS_TO_PRODUCE_IMAGES];

// Flags
uint8_t interruption = 0;
uint32_t new_images_are_ready = 0;

uint32_t count_calls_to_produce_images = 0;

void activate_interrupt(uint32_t irq, void* handler){
	modifier_registre_1(interrupt_mask_register, irq);

	// void *bcc_isr_register(int source, void (*handler)(void *arg, int source), void *arg);
	bcc_isr_register(irq,handler,0);
}


void handler(void* arg, int irq){
	interruption = 1;
}

void produce_images_finished(void* arg, int irq){
	new_images_are_ready = 1;
}


/**
* Configure les registres d'un timer pour qu'il déclenche une interruption
* après reload_value ticks et se recharge automatiquement avec la même période.
* @param timer_counter_register pointeur sur le registre de compteur du timer
* @param period nombre tick entre deux déclenchement du timer
*/
void start_timer(volatile uint32_t* timer_counter_register, uint32_t period){
	// Les registres counter, reload et control d'un timer (se suivent en mémoire)

	// 31: 0 Timer Counter value. Decremented by 1 for each prescaler tick.
	// 31: 0 Timer Reload value. This value is loaded into the timer counter value register when ‘1’ is written to load bit in the timers control register.
	// (Source : https://www.gaisler.com/doc/gr712rc-usermanual.pdf , p. 92)

	volatile uint32_t* timer_reload_value_register = timer_counter_register + 1;
	volatile uint32_t* timer_control_register = timer_counter_register + 2;

	*timer_counter_register = period;
	*timer_reload_value_register =  period;

	// #define EN 0 // Enable : Enable the timer.
	modifier_registre_1(timer_control_register, EN);

	// #define LD 2 // Load : Load value from the timer reload register to the timer counter value register.
	modifier_registre_1(timer_control_register, LD);

	// #define RS 1 // Restart : If set, the timer counter value register is reloaded with the value of the reload register when the timer underflows
	modifier_registre_1(timer_control_register, RS);

	//#define IE 3 //Interrupt Enable : If set the timer signals interrupt when it underflows.
	modifier_registre_1(timer_control_register, IE);
}

void break_point() {

}

int main(void) {
	Windows_producer wp;
	float img_buffer[NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES][36];

	// extern char init(Windows_producer* wp, float* img_buffer, unsigned int windows_max_number);
	// Initialize the windows producer. Must be call before produce_images()
	char init_result = init(&wp, img_buffer, NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES);
	if (init_result == -1)
		exit(EXIT_FAILURE);

	// extern void enable_irq(Windows_producer* wp, int irq);
	// Enable the trigger of an IRQ when produce_images() finish
	enable_irq(&wp, 10);

	activate_interrupt(10, produce_images_finished);
	activate_interrupt(9, handler); // timer 2 => irq 9

	// void start_timer(volatile uint32_t* timer_counter_register, uint32_t period)
	// period (µs), 1 µs = 0.001 ms , 1 ms = 1,000 µs
    // 100 ms = 100,000 µs => period = 100000
	start_timer(timer_2_counter_value_register, 100000);

	while (count_calls_to_produce_images < NB_CALLS_TO_PRODUCE_IMAGES) {
		if(new_images_are_ready == 1) {
			 uint32_t i;
			 for (i = 0; i < NB_WINDOWS_TO_PRODUCE_FOR_EACH_CALL_TO_PRODUCE_IMAGES ; i++) {
				 float* mask = get_mask(&wp, i);
				 float* window = img_buffer[i];

				 flux_pondere[i][count_calls_to_produce_images] = computeFluxPondere(window, mask);
			 }

			 count_calls_to_produce_images++;
			 new_images_are_ready = 0;
		}

		if(interruption == 1) {
			// extern void produce_images(Windows_producer* wp );
			// Produce new windows for the same star but with an evolved brightness
			produce_images(&wp);
			interruption = 0;
		}
	}

	break_point();

	return EXIT_SUCCESS;
}
