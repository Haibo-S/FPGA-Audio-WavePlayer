/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include "alt_types.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"

//#include "altera_avalon_timer_sc.h"
//#include "altera_avalon_timer_ts.h"
//#include "altera_avalon_timer_vars.h"


// static variables for timer

static void handle_timer_interrupt(void *context, alt_32 id);
static void btn_interrupt(void *context, alt_32 id);
static int button_value = 0xF;
static int counter = 0;
static int button_pressed = 0;

int main()
{
	//IOWR(BUTTON_PIO_BASE, 3, 0);
	//IOWR(TIMER_0_BASE, 0, 0);


	//IOWR(TIMER_0_BASE, 1, 0x0);
	alt_irq_register(TIMER_0_IRQ, (void*)0, handle_timer_interrupt);
	alt_irq_register(BUTTON_PIO_IRQ, (void*)0, btn_interrupt);
	IOWR(BUTTON_PIO_BASE, 3, 0);
	IOWR(BUTTON_PIO_BASE, 2, 0xF);
	IOWR(TIMER_0_BASE, 0, 0); // status
	IOWR(TIMER_0_BASE, 1, 0x8); // control (stop is on, ITO is on)
	IOWR(TIMER_0_BASE, 2, 0x8F9C); // periodl
	IOWR(TIMER_0_BASE, 3, 1); // periodh

	int x = 0;
	while(1){
		if (button_value != 0xF){
			printf("button value: %d\n", button_value);
			button_value = 0xF;
		}
	}

	return 0;
}

static void handle_timer_interrupt(void* context, alt_32 id){
	int current_value = IORD(BUTTON_PIO_BASE, 0);
	IOWR(LED_PIO_BASE, 0, 0x1);

	if (button_pressed == 0 && current_value != 0xF){
		button_value = current_value;
		button_pressed = 1;
	}

	if(current_value == 0xF){
		counter += 1;
		if (counter < 20){
			// restart timer
			IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, 0x5);
        }
	}

	if (counter > 20){
        IOWR(LED_PIO_BASE, 0, 0x0);
        IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, 0x8);
        IOWR(BUTTON_PIO_BASE, 2, 0xF);
        // reset timer variables
        counter = 0;
        button_pressed = 0;
	}
}

static void btn_interrupt(void *context, alt_32 id){
	IOWR(BUTTON_PIO_BASE, 3, 0);
	IOWR(BUTTON_PIO_BASE, 2, 0);
	IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, 0x5);
}
