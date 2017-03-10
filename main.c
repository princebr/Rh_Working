// main.c
//
// Test program for bcm2835 library
// 


#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>	// Use to print out uint64_t data types
#include <functions.h>	// Place this after stdint.h for var types


int main(int argc, char **argv)
{
	/*--------------- Initialize Hardware -----------------*/

	init_bcm();

	/*--------------- Initialize Peripherals --------------*/

	init_periph();
	
	/*--------------- Set Initial Conditions --------------*/

	set_initial_conditions();
	
	// Write to all lighting features	
	write_RGBlighting_feature(backshellLight);
	write_RGBlighting_feature(stowageLight);
	write_RGBlighting_feature(underIFELight);
	write_RGBlighting_feature(footwellLight);
	
	// Light up Cap Touch UI
	illuminateCapTouch(500);
		
	/*------------------- Main Routine --------------------*/
	
	
	uint8_t i;
	uint64_t data;
	
	while (1)
	{
		// Monitor capacitive UI buttons and provide feedback
		svc_ATTD_btn();
		svc_DND_btn();
		svc_RL_btn();
		svc_LAY_btn();
		svc_TTL_btn();
		
		// Monitor seat microswitches
		svc_NEU_usw();
		svc_LAY_usw();
		
		// Detect state changes from cap touch
		svc_RL_btn2();
		svc_readingLight();
		
		svc_Light_Features();
		
		//uart_tx_test();
		//delay_ms(500);
		
		// Standard Loop Delay
		delay_ms(10);
		
	}

	// Close I2C session
	i2c_end();
	
	// Close bcm2835 library and deallocate memory
	close_bcm();
		
	return 0;
}

