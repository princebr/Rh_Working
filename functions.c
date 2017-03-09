

#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>	// Use to print out uint64_t data types

// Project Headers
#include <functions.h>
#include <config.h>
//#include <uart.h>


char wbuf[MAX_LEN];
char rbuf[MAX_LEN];


char led0_on[] = {LED0_ON_L, 0x00, 0x00, 0x00, 0x00};

// TEST Buffers
char led2_on[] = {LED2_ON_L, 0x00, 0x00, 0x10, 0x00};
char led2_off[] = {LED2_ON_L, 0x00, 0x00, 0x00, 0x00};
//--------------

char all_on[] = {ALL_LED_ON, 0xBB};
char all_off[] = {ALL_LED_OFF, 0x00};

//uint8_t slave_address = 0x42;
//uint8_t data;

uint8_t init_bcm(void)
{
	printf("Initialize BCM... \n");
	
	if (!bcm2835_init())
		return 1;
	
	else
	{
		// Set CapTouch I/O to inputs
		bcm2835_gpio_fsel(CAP_ATTD, BCM2835_GPIO_FSEL_INPT);
		bcm2835_gpio_fsel(CAP_DND, BCM2835_GPIO_FSEL_INPT);
		bcm2835_gpio_fsel(CAP_RL, BCM2835_GPIO_FSEL_INPT);
		bcm2835_gpio_fsel(CAP_LF, BCM2835_GPIO_FSEL_INPT);
		bcm2835_gpio_fsel(CAP_TTL, BCM2835_GPIO_FSEL_INPT);
			
		// Set RL CapTouch I/O to input
		bcm2835_gpio_fsel(CAP_RL_BTN, BCM2835_GPIO_FSEL_INPT);
		
		// Set uSW I/O to inputs
		bcm2835_gpio_fsel(NEU_USW, BCM2835_GPIO_FSEL_INPT);
		bcm2835_gpio_fsel(LAY_USW, BCM2835_GPIO_FSEL_INPT);
		
		// Clear EDS registers for uSW
		bcm2835_gpio_ren(NEU_USW);
		bcm2835_gpio_ren(LAY_USW);
		bcm2835_gpio_set_eds(NEU_USW);
		bcm2835_gpio_set_eds(LAY_USW);
		
		// Clear EDS registers for RL inputs
		bcm2835_gpio_ren(CAP_RL);		// Rising edge detect
		bcm2835_gpio_ren(CAP_RL_BTN);	// Rising edge detect
		bcm2835_gpio_set_eds(CAP_RL);
		bcm2835_gpio_set_eds(CAP_RL_BTN);
		bcm2835_gpio_len(CAP_DND);
		bcm2835_gpio_set_eds(CAP_DND);
		bcm2835_gpio_len(CAP_ATTD);
		bcm2835_gpio_set_eds(CAP_ATTD);
		
		// Set Relay I/O to outputs
		bcm2835_gpio_fsel(TTL_SW, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_fsel(NEU_SW, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_fsel(LAY_SW, BCM2835_GPIO_FSEL_OUTP);
		
		// Set Status LED output
		bcm2835_gpio_fsel(ST_LED, BCM2835_GPIO_FSEL_OUTP);

		// Set Amplifier Mute output
		bcm2835_gpio_fsel(MUTE, BCM2835_GPIO_FSEL_OUTP);
		
		// Set OLED Select output
		bcm2835_gpio_fsel(SEL, BCM2835_GPIO_FSEL_OUTP);
		
		// Explicitly set UART pins as alternate
		bcm2835_gpio_fsel(UART_RX, BCM2835_GPIO_FSEL_ALT0);
		bcm2835_gpio_fsel(UART_TX, BCM2835_GPIO_FSEL_ALT0);
		
		// Set RS-485 to READ mode
		bcm2835_gpio_fsel(RE_DE, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_write(RE_DE, LOW);
		
		//bcm2835_i2c_setSlaveAddress(PCA1);
		bcm2835_i2c_begin();
		
		printf("Init BCM done. \n");
		return 0;
	}	
}


void init_periph(void)
{
	uint8_t data = 0;
	
	printf("Initialize PCA1... \n");
	bcm2835_i2c_setSlaveAddress(PCA1);
	data = bcm2835_i2c_write(autoinc_msg, 2);
	printf("Write Result (Autoinc) = %d\n", data); 
	delay(1);
	data = bcm2835_i2c_write(cfg_msg, 2);
	printf("Write Result (Config) = %d\n", data); 
	delay(1);
	
	printf("Initialize PCA2... \n");
	bcm2835_i2c_setSlaveAddress(PCA2);
	data = bcm2835_i2c_write(autoinc_msg, 2);
	printf("Write Result (Autoinc) = %d\n", data); 
	delay(1);
	data = bcm2835_i2c_write(cfg_msg, 2);
	printf("Write Result (Config) = %d\n", data); 
	delay(1);
	
	
	return;
}


void svc_Light_Features(void)
{
	readingLight2 = svcLightFeature(readingLight2);
	
	return;
}

struct ledFeature dimStart(struct ledFeature rl, struct rlFeature state, uint16_t target)
{
	rl.dim = ON;
	rl.timerStart = bcm2835_st_read(); // read system timer
	rl.timerExp = rl.timerStart + rl.rate;
	printf("rl.rate: %d\n", rl.rate);
	printf("sys timer: %d\n", bcm2835_st_read());
	// add rollover mechanism here ...
	
	// set pwmTarget from target ...
	rl.pwmTarget = target;
	
	return rl;
}


struct ledFeature svcLightFeature(struct ledFeature rl)
{
	// Is dimming enabled?
	if (rl.dim == ON)
	{
		//printf("rl.timerExp: %d\n", rl.timerExp);
		// Check for timer expiration
		if (bcm2835_st_read() >= rl.timerExp)
		{
			// Incremement / Decrement
			if (rl.inc_dec == INC)
				rl.pwmRaw += 5;
			else
				rl.pwmRaw -= 5;
				
			// Update PCA 
			write_lighting_feature(rl);
			
			// Update new timer expiration
			rl.timerStart = getTimer();
			printf("rl.timerStart: %" PRIu64 "\n", rl.timerStart);
			rl.timerExp = rl.timerStart + rl.rate;
			
			// Check if pwmTarget reached
			if (rl.inc_dec == INC)
			{
				if (rl.pwmRaw >= rl.pwmTarget)
				{
					// Disable dimming ...
					rl.dim = OFF;
				}
			}
			else
			{
				if (rl.pwmRaw <= rl.pwmTarget)
				{
					// Disable dimming ...
					rl.dim = OFF;
				}	
			}			
		}
	}
	
	return rl;
}


void set_initial_conditions(void)
{
	// Turn on Status LED
	set_led(ST_LED);

	// Mute Audio Amplifier
	mute_audio();

	// Set Reading Light 1
	readingLight1.address = CH7_BASE;
	readingLight1.device = PCA1;

	// Set Reading Light 2
	readingLight2.address = CH8_BASE;
	readingLight2.device = PCA1;
	
	// Default Reading Light Intensity
	readingLight.intensity = RL_DEFAULT_MAX;
	
		
	/*******************************************/
	/* REVISIT THIS CONFIGURATION 
		
	// Set Reading Light state and turn on 
	if (bcm2835_gpio_lev(NEU_USW))
	{
		readingLight.mode = RL2;
		readingLight1.pwmRaw = 0;
		readingLight2.pwmRaw = readingLight.intensity;
	}
	else
	{
		readingLight.mode = RL1;
		readingLight1.pwmRaw = readingLight.intensity;
		readingLight2.pwmRaw = 0;
	}*/
	
	/*******************************************/
		
	// Trigger to turn on reading light by default
	readingLight.next_state = 1;
	readingLight.prev_state = 0;
	
	// Clear CAP_RL_BTN states
	cap_rl_btn.next_state = 0;
	cap_rl_btn.prev_state = 0;
	
	// Clear CAP ATTD states
	cap_attd.next_state = 0;
	cap_attd.prev_state = 0;
	
	// Clear CAP DND states
	cap_dnd.next_state = 0;
	cap_dnd.prev_state = 0;
	
	// Set OLED display 1
	bcm2835_gpio_write(SEL, HIGH);
	
	//***********************************/
	
	// Inititialize Reading Light
	readingLight.mode = RL1;
	//readingLight1.pwmRaw = readingLight.intensity;
	readingLight1.pwmRaw = 0;
	readingLight2.pwmRaw = 0;
	
	readingLight1.inc_dec = INC;
	readingLight2.inc_dec = INC;
	readingLight2.rate = DIMMING_RATE;
	readingLight2 = dimStart(readingLight2, readingLight, 1000);

	// Set Cap TTL Light
	capTTLLight.state = OFF;
	capTTLLight.pwmRaw = CAP_LIGHT_DEFAULT;
	capTTLLight.address = CAP_TTL_BASE;
	capTTLLight.device = PCA1;

	// Set Cap LAY Light
	capLAYLight.state = OFF;
	capLAYLight.pwmRaw = CAP_LIGHT_DEFAULT;
	capLAYLight.address = CAP_LAY_BASE;
	capLAYLight.device = PCA1;
	
	// Set Cap RL Light
	capRLLight.state = OFF;
	capRLLight.pwmRaw = CAP_LIGHT_DEFAULT;
	capRLLight.address = CAP_RL_BASE;
	capRLLight.device = PCA1;

	// Set Cap DND White
	capDNDWhiteLight.state = OFF;
	capDNDWhiteLight.pwmRaw = CAP_LIGHT_DEFAULT;
	capDNDWhiteLight.address = CAP_DND_WHITE_BASE;
	capDNDWhiteLight.device = PCA1;

	// Set Cap DND Red
	capDNDRedLight.state = OFF;
	capDNDRedLight.pwmRaw = 0;
	capDNDRedLight.address = CAP_DND_RED_BASE;
	capDNDRedLight.device = PCA1;
	
	// Set Cap ATTD White
	capATTDWhiteLight.state = OFF;
	capATTDWhiteLight.pwmRaw = CAP_LIGHT_DEFAULT;
	capATTDWhiteLight.address = CAP_ATTD_WHITE_BASE;
	capATTDWhiteLight.device = PCA1;

	// Set Cap ATTD Blue
	capATTDBlueLight.state = OFF;
	capATTDBlueLight.pwmRaw = 0;
	capATTDBlueLight.address = CAP_ATTD_BLUE_BASE;
	capATTDBlueLight.device = PCA1;
			
	// Set Backshell Light
	backshellLight.state = OFF;
	backshellLight.pwmRaw_R = RGB_R_DEFAULT;
	backshellLight.pwmRaw_G = RGB_G_DEFAULT;
	backshellLight.pwmRaw_B = RGB_B_DEFAULT;
	backshellLight.address = CH13_BASE;
	backshellLight.device = PCA1;

	// Set Footwell Light
	footwellLight.state = OFF;
	footwellLight.pwmRaw_R = RGB_R_DEFAULT;
	footwellLight.pwmRaw_G = RGB_G_DEFAULT;
	footwellLight.pwmRaw_B = RGB_B_DEFAULT;
	footwellLight.address = CH3_BASE;
	footwellLight.device = PCA2;
	
	// Set Under IFE Light
	underIFELight.state = OFF;
	underIFELight.pwmRaw_R = RGB_R_DEFAULT;
	underIFELight.pwmRaw_G = RGB_G_DEFAULT;
	underIFELight.pwmRaw_B = RGB_B_DEFAULT;
	underIFELight.address = CH0_BASE;
	underIFELight.device = PCA2;
	
	// Set Stowage Light
	stowageLight.state = OFF;
	stowageLight.pwmRaw_R = RGB_R_DEFAULT;
	stowageLight.pwmRaw_G = RGB_G_DEFAULT;
	stowageLight.pwmRaw_B = RGB_B_DEFAULT;
	stowageLight.address = CH10_BASE;
	stowageLight.device = PCA1;	
		
	return;
}

/* -------------- Write to Single Lighting Features ----------------- */
void write_lighting_feature(struct ledFeature feature)
{
	// break pwmRaw values into ~OFF_L and ~OFF_H
	uint8_t ledx_on_h = 0;
	uint8_t ledx_on_l = 0;
	uint8_t ledx_off_l = feature.pwmRaw & 0xFF;
	uint8_t ledx_off_h = feature.pwmRaw >> 8;
	
	// assemble message
	uint8_t msg[] = {feature.address,
		ledx_on_l, ledx_on_h, ledx_off_l, ledx_off_h
	};
	
	// set i2c address based on feature.device
	bcm2835_i2c_setSlaveAddress(feature.device);
	
	// Send message
	bcm2835_i2c_write(msg, sizeof(msg));
	return;
}

/* ---------------- Write to RGB Lighting Features ------------------ */
void write_RGBlighting_feature(struct ledRGBFeature feature)
{
	// break pwmRaw values into ~OFF_L and ~OFF_H
	uint8_t ledx_on_h = 0;
	uint8_t ledx_on_l = 0;
	uint8_t ledr_off_l = feature.pwmRaw_R & 0xFF;
	uint8_t ledr_off_h = feature.pwmRaw_R >> 8;
	uint8_t ledg_off_l = feature.pwmRaw_G & 0xFF;
	uint8_t ledg_off_h = feature.pwmRaw_G >> 8;
	uint8_t ledb_off_l = feature.pwmRaw_B & 0xFF;
	uint8_t ledb_off_h = feature.pwmRaw_B >> 8;
	
	// assemble message
	uint8_t msg[] = {feature.address,
		ledx_on_l, ledx_on_h, ledr_off_l, ledr_off_h,
		ledx_on_l, ledx_on_h, ledg_off_l, ledg_off_h,
		ledx_on_l, ledx_on_h, ledb_off_l, ledb_off_h
	};
	
	// set i2c address based on feature.device
	bcm2835_i2c_setSlaveAddress(feature.device);
	
	// Send message
	bcm2835_i2c_write(msg, sizeof(msg));
	return;
}


void svc_readingLight(void)
{
	// Detect state change
	if (readingLight.next_state != readingLight.prev_state)
	{
		// If next_state is ON
		if (readingLight.next_state)
		{
			printf("RL ON \n");
			if (readingLight.mode == RL1)
			{
				readingLight1.pwmRaw = readingLight.intensity;
				readingLight2.pwmRaw = 0;
			}
			else
			{
				readingLight1.pwmRaw = 0;
				readingLight2.pwmRaw = readingLight.intensity;
			}
		}
		// If nex_state is OFF
		else
		{
			printf("RL OFF \n");
			readingLight1.pwmRaw = 0;
			readingLight2.pwmRaw = 0;
		}
		
		// Update state transition
		readingLight.prev_state = readingLight.next_state;
		
		write_lighting_feature(readingLight1);
		write_lighting_feature(readingLight2);
		
	}
}



uint64_t getTimer(void)
{
	//uint64_t data = bcm2835_st_read();
	//printf("Timer 1: %d" PRIu64 "\n", bcm2835_st_read());
	return bcm2835_st_read();
}


/* --------- SWITCH ACTIONS --------- */
void svc_NEU_usw(void)
{
	// Capture uSW state
	neu_usw.next_state = bcm2835_gpio_lev(NEU_USW);
	
	// Update lighting only on state change
	if (neu_usw.next_state != neu_usw.prev_state)
	{
		// Change to use .state - don't pull lev twice...
		if (bcm2835_gpio_lev(NEU_USW) & readingLight.mode == RL1)
		{
			printf("NEU\n");
			// Adjust state values even when RL is OFF
			readingLight.mode = RL2;
			readingLight1.pwmRaw = 0;
			readingLight2.pwmRaw = readingLight.intensity;
		}
		// Change to use .state - don't pull lev twice...
		else if (!bcm2835_gpio_lev(NEU_USW) & readingLight.mode == RL2)
		{
			printf("/NEU\n");
			readingLight.mode = RL1;
			readingLight1.pwmRaw = readingLight.intensity;
			readingLight2.pwmRaw = 0;
		}

		// REPLACE WITH FADE ROUTINE ******************
		// Write only if RL is turned ON
		if (readingLight.next_state == ON)
		{
			write_lighting_feature(readingLight1);
			write_lighting_feature(readingLight2);
		}
		// ********************************************
		
		// Update prev_state
		neu_usw.prev_state = neu_usw.next_state;
	}

	
	return;
}


void svc_LAY_usw(void)
{
	// CHANGE TO LEV ***************
	if (bcm2835_gpio_lev(LAY_USW))
	{
		printf("LAY uSW \n");
		
		// dim lighting..................
		
		// unmute audio
		bcm2835_gpio_write(MUTE, LOW);
	}
	else
	{
		bcm2835_gpio_write(MUTE, HIGH);
	}
	
	return;
}


/* --------- BTN ACTIONS --------- */

void svc_ATTD_btn(void)
{
	// Capture btn state
	cap_attd.next_state = !bcm2835_gpio_lev(CAP_ATTD);

	// Update lighting only on falling edge
	if ((cap_attd.next_state == 1) & (cap_attd.prev_state == 0))
	{
		if (capATTDWhiteLight.state == WHITE) 
		{
			capATTDWhiteLight.pwmRaw = CAP_LIGHT_OFF;
			capATTDBlueLight.pwmRaw = CAP_LIGHT_DEFAULT;
			write_lighting_feature(capATTDWhiteLight);
			write_lighting_feature(capATTDBlueLight);
			
			capATTDWhiteLight.state = BLUE;
			//	printf("ATTD: %d\n", capATTDWhiteLight.state);
		}
		else 
		{
			capATTDWhiteLight.pwmRaw = CAP_LIGHT_DEFAULT;
			capATTDBlueLight.pwmRaw = CAP_LIGHT_OFF;
			write_lighting_feature(capATTDWhiteLight);
			write_lighting_feature(capATTDBlueLight);
			
			capATTDWhiteLight.state = WHITE;
		}
			
		cap_attd.prev_state = cap_attd.next_state;
	}
		
	// Rising Edge
	if ((cap_attd.next_state == 0) & (cap_attd.prev_state == 1))
	{
		cap_attd.prev_state = cap_attd.next_state;
	}	
	
	return;
}


void svc_DND_btn(void)
{
	// Capture btn state
	cap_dnd.next_state = !bcm2835_gpio_lev(CAP_DND);

	// Update lighting only on falling edge
	if ((cap_dnd.next_state == 1) & (cap_dnd.prev_state == 0))
	{	
		if (capDNDWhiteLight.state == WHITE) 
		{
			capDNDWhiteLight.pwmRaw = CAP_LIGHT_OFF;
			capDNDRedLight.pwmRaw = CAP_LIGHT_DEFAULT;
			write_lighting_feature(capDNDWhiteLight);
			write_lighting_feature(capDNDRedLight);

			capDNDWhiteLight.state = RED;
		}
		else 
		{
			capDNDWhiteLight.pwmRaw = CAP_LIGHT_DEFAULT;
			capDNDRedLight.pwmRaw = CAP_LIGHT_OFF;
			write_lighting_feature(capDNDWhiteLight);
			write_lighting_feature(capDNDRedLight);
			
			capDNDWhiteLight.state = WHITE;
		}	
		
		cap_dnd.prev_state = cap_dnd.next_state;
	}
	
	// Rising Edge
	if ((cap_dnd.next_state == 0) & (cap_dnd.prev_state == 1))
	{
		cap_dnd.prev_state = cap_dnd.next_state;
	}			
				
	return;
}

// Duplicate for Cap button at RL...
void svc_RL_btn(void)
{
	if (!bcm2835_gpio_lev(CAP_RL)) 
	{
		// Feedback illumination at button
		capRLLight.pwmRaw = readingLight.intensity;
		write_lighting_feature(capRLLight);
		
		// Detect rising edge of button
		if (bcm2835_gpio_eds(CAP_RL))
		{
			if (readingLight.prev_state)
				readingLight.next_state = 0;
			else
				readingLight.next_state = 1;
			
			// Clear EDS flag
			bcm2835_gpio_set_eds(CAP_RL);
		}			
	}
	else {
		// Feedback illumination at button
		capRLLight.pwmRaw = CAP_LIGHT_DEFAULT;
		write_lighting_feature(capRLLight);
	}
	return;
}


void svc_LAY_btn(void)
{
	if (!bcm2835_gpio_lev(CAP_LF)) {
		capLAYLight.pwmRaw = CAP_LIGHT_HIGH;
		write_lighting_feature(capLAYLight);
		bcm2835_gpio_write(LAY_SW, HIGH);
	}
	else {
		capLAYLight.pwmRaw = CAP_LIGHT_DEFAULT;
		write_lighting_feature(capLAYLight);
		bcm2835_gpio_write(LAY_SW, LOW);
	}
	return;
}

void svc_TTL_btn(void)
{
	if (!bcm2835_gpio_lev(CAP_TTL)) {
		capTTLLight.pwmRaw = CAP_LIGHT_HIGH;
		write_lighting_feature(capTTLLight);
		bcm2835_gpio_write(TTL_SW, HIGH);
	}
	else {
		capTTLLight.pwmRaw = CAP_LIGHT_DEFAULT;
		write_lighting_feature(capTTLLight);
		bcm2835_gpio_write(TTL_SW, LOW);
	}
	return;
}

// Cap touch at RL
// EDS didn't work on Pin 40, so had to make own edge detect
void svc_RL_btn2(void)
{

	// Capture uSW state
	cap_rl_btn.next_state = bcm2835_gpio_lev(CAP_RL_BTN);
	
	// Rising Edge
	if ((cap_rl_btn.next_state == 1) & (cap_rl_btn.prev_state == 0))
	{
		printf("CAP_RL_BTN Next: %d\n", cap_rl_btn.next_state);

		if (readingLight.prev_state)
			readingLight.next_state = 0;
		else
			readingLight.next_state = 1;
		
		printf("CAP_RL_BTN EDS: %d\n", bcm2835_gpio_eds(CAP_RL_BTN));
	
		cap_rl_btn.prev_state = cap_rl_btn.next_state;
	}	
	
	// Falling Edge
	if ((cap_rl_btn.next_state == 0) & (cap_rl_btn.prev_state == 1))
	{
		cap_rl_btn.prev_state = cap_rl_btn.next_state;
	}	
	
	return;
}

/* ------------------------------- */

/* --------- LED ACTIONS --------- */
void set_led(uint8_t led)
{
	bcm2835_gpio_write(led, HIGH);
	return;	
}

void clear_led(uint8_t led)
{
	bcm2835_gpio_write(led, LOW);
	return;	
}

void toggle_led(void)//uint8_t led)
{
	if (bcm2835_gpio_lev(SEL))
		bcm2835_gpio_write(SEL, LOW);
	else
		bcm2835_gpio_write(SEL, HIGH);
	return;
}

/* ------------------------------- */

/* -------- AUDIO ACTIONS -------- */

void mute_audio(void)
{
	bcm2835_gpio_write(MUTE, HIGH);
	return;
}

void unmute_audio(void)
{
	bcm2835_gpio_write(MUTE, LOW);
	return;
}

/* --------------------------------- */

/* ------------- MISC -------------- */

void i2c_end(void)
{
	bcm2835_i2c_end(); 
	return;
}


void close_bcm(void)
{
	bcm2835_close();
	return;
}


void delay_ms(unsigned int ms)
{
	bcm2835_delay(ms);
	return;
}
