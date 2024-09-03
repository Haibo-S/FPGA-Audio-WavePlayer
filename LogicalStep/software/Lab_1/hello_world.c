#include "alt_types.h"  // define types used by Altera code, e.g. alt_u8
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "system.h"  // constants such as the base addresses of any PIOs
                     // defined in your hardware
#include "sys/alt_irq.h"  // required when using interrupts
#include <io.h>
#include "altera_avalon_pio_regs.h"


int background()
{
	// LED 0
	IOWR(LED_PIO_BASE, 0, IORD(LED_PIO_BASE, 0) | 1);
	int j;
	int x = 0;
	int grainsize = 4;
	int g_taskProcessed = 0;

	for(j = 0; j < grainsize; j++){
		g_taskProcessed++;
	}
	IOWR(LED_PIO_BASE, 0, IORD(LED_PIO_BASE, 0) ^ 1);
	return x;
}

// interrupt code
static void STIMULUS_ISR(void* context, alt_u32 id){
	// LED 2
	IOWR(LED_PIO_BASE, 0, IORD(LED_PIO_BASE, 0) | 4);
	// flip on and off the response
	IOWR(RESPONSE_OUT_BASE, 0, 0x1);
	IOWR(RESPONSE_OUT_BASE, 0, 0x0);

	// turning interrupt back on
	IOWR(STIMULUS_IN_BASE, 3, 0x0);

	IOWR(LED_PIO_BASE, 0, IORD(LED_PIO_BASE, 0) ^ 4);
}

int main(void)
{
	alt_u8 switch_0;
	alt_u8 push_button_0;
	alt_8 mode;
	alt_u16 egm_busy;

	alt_u8 prev_stimulus_state = 0;
	alt_u8 current_stimulus_state = 0;

	int period = 2;
	int pulse_width = period/2;
	int bg_tasks_run = 0;
	int latency = 0;
	int missed = 0;
	int multiple = 0;

	switch_0 = IORD(SWITCH_PIO_BASE, 0) & 0x1;

	if(switch_0 == 1){
		mode = 1;
		printf("Tight polling method selected\n");
	}
	else{
		mode = 0;
		printf("Interrupt method selected\n");
	}

	printf("Please, press PB0 to continue\n");

	// poll for PB0 being pressed
	do{
		// note that push buttons are active low
		push_button_0 = ~IORD(BUTTON_PIO_BASE, 0) & 0x1;
	}while(push_button_0 == 0);

	printf("PB0 pressed\n");


	// tight polling method
	if(mode == 1){
		int characteristic_count;


		// setting initial period
		IOWR(EGM_0_BASE, 2, period);
		// setting initial pulse width
		IOWR(EGM_0_BASE, 3, pulse_width);
		// turning on EGM
		IOWR(EGM_0_BASE, 0, 0);
		IOWR(EGM_0_BASE, 0, 1);

		while (period <= 1500) {

			bg_tasks_run = 0;
			characteristic_count = 0;

			// LED[1] Start of test run
			IOWR(LED_PIO_BASE, 0, IORD(LED_PIO_BASE, 0) | 2);
			IOWR(LED_PIO_BASE, 0, IORD(LED_PIO_BASE, 0) ^ 2);

			while(IORD(EGM_0_BASE, 1)){
				if(characteristic_count == 0){
					// first rising edge
					while(!IORD(STIMULUS_IN_BASE, 0)){
						// do nothing
					}
					IOWR(RESPONSE_OUT_BASE, 0, 0x1);
					IOWR(RESPONSE_OUT_BASE, 0, 0x0);

					// detect for rising edge
					prev_stimulus_state = 1;
					current_stimulus_state = IORD(STIMULUS_IN_BASE, 0);
					while(((current_stimulus_state-prev_stimulus_state) != 1)  && IORD(EGM_0_BASE, 1)){
						prev_stimulus_state = current_stimulus_state;
						current_stimulus_state = IORD(STIMULUS_IN_BASE, 0);
						background();
						bg_tasks_run += 1;
					}
					IOWR(RESPONSE_OUT_BASE, 0, 0x1);
					IOWR(RESPONSE_OUT_BASE, 0, 0x0);
					characteristic_count = bg_tasks_run-1;
				}
				else{
					for(int i = 0; i < characteristic_count; i++){
						if(IORD(EGM_0_BASE, 1)){
							bg_tasks_run += 1;
							background();
						}
						else{
							break;
						}
					}

					// wait for stimulus high
					while(!IORD(STIMULUS_IN_BASE, 0) && IORD(EGM_0_BASE, 1)){}
					if(IORD(EGM_0_BASE, 1)){
						IOWR(RESPONSE_OUT_BASE, 0, 0x1);
						IOWR(RESPONSE_OUT_BASE, 0, 0x0);
					}
				}
			}


			IOWR(EGM_0_BASE, 0, 0);
			latency = IORD(EGM_0_BASE, 4);
			missed = IORD(EGM_0_BASE, 5);
			multiple = IORD(EGM_0_BASE, 6);
			printf("%d, %d, %d, %d, %d, %d\n", period, pulse_width, bg_tasks_run, latency, missed, multiple);

			period += 2;
			pulse_width = period / 2;

			IOWR(EGM_0_BASE, 0, 1);
			IOWR(EGM_0_BASE, 2, period);
			IOWR(EGM_0_BASE, 3, pulse_width);
		}

	}
	// interrupt method
	else{

		// initialize an interrupt vector
		alt_irq_register(STIMULUS_IN_IRQ, (void*)0, STIMULUS_ISR);
		IOWR(STIMULUS_IN_BASE, 2, 0x1);

		// setting initial period
		IOWR(EGM_0_BASE, 2, period);
		// setting initial pulse width
		IOWR(EGM_0_BASE, 3, pulse_width);
		// turning on EGM
		IOWR(EGM_0_BASE, 0, 0);
		IOWR(EGM_0_BASE, 0, 1);

		// while loop for EGM

		while(period <= 5000){

			// LED[1] Start of test run
			IOWR(LED_PIO_BASE, 0, IORD(LED_PIO_BASE, 0) | 2);
			IOWR(LED_PIO_BASE, 0, IORD(LED_PIO_BASE, 0) ^ 2);

			// initialize background loop
			bg_tasks_run = 0;

			do{
				egm_busy = IORD(EGM_0_BASE, 1);
				bg_tasks_run += 1;
				background();
			}while(egm_busy);

			IOWR(EGM_0_BASE, 0, 0);
			latency = IORD(EGM_0_BASE, 4);
			missed = IORD(EGM_0_BASE, 5);
			multiple = IORD(EGM_0_BASE, 6);

			printf("%d, %d, %d, %d, %d, %d\n", period, pulse_width, bg_tasks_run, latency, missed, multiple);
			period += 2;
			pulse_width = period/2;
			IOWR(EGM_0_BASE, 0, 1);
			IOWR(EGM_0_BASE, 2, period);
			IOWR(EGM_0_BASE, 3, pulse_width);
		}
		// run background task
	}

	// disable and EGM
	IOWR(EGM_0_BASE, 0, 0);

	return(0);
}
