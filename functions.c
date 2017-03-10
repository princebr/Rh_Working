

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


void init_lighting(void)
{
	// Set initial lighting conditions
	write_RGBlighting_feature(backshellLight);
	write_RGBlighting_feature(stowageLight);
	write_RGBlighting_feature(underIFELight);
	write_RGBlighting_feature(footwellLight);
	
	// Initiate dim to presesntation state
	backshellLight = dimRGBStart(backshellLight, ttlRGB);
	stowageLight = dimRGBStart(stowageLight, ttlRGB);
	underIFELight = dimRGBStart(underIFELight, ttlRGB);
	footwellLight = dimRGBStart(footwellLight, ttlRGB);
}


void svc_Light_Features(void)
{
	readingLight1 = svcLightFeature(readingLight1);
	//printf("RL1.pwmRaw: %d\n", readingLight1.pwmRaw);
	//printf("RL1.dim: %d\n", readingLight1.dim);
	
	readingLight2 = svcLightFeature(readingLight2);
	//printf("RL2.pwmRaw: %d\n", readingLight2.pwmRaw);
	//printf("RL2.dim: %d\n", readingLight2.dim);

	
	backshellLight = svcRGBLightFeature(backshellLight);
	stowageLight = svcRGBLightFeature(stowageLight);
	underIFELight = svcRGBLightFeature(underIFELight);
	footwellLight = svcRGBLightFeature(footwellLight);
	
	return;
}

// Coordinator between both cap touch interfaces
void svc_readingLight(void)
{
	// Detect state change
	if (readingLight.next_state != readingLight.prev_state)
	{
		// If next_state is ON
		if (readingLight.next_state)
		{
			printf("RL ON \n");
			printf("RL.mode: %d\n", readingLight.mode);
			if (readingLight.mode == RL1)
			{
				readingLight1.pwmTarget = readingLight.intensity;
				readingLight1.inc_dec = INC;
				readingLight2.pwmTarget = 0;
				readingLight2.inc_dec = DEC;
			}
			else
			{
				readingLight1.pwmTarget = 0;
				readingLight1.inc_dec = DEC;
				readingLight2.pwmTarget = readingLight.intensity;
				readingLight2.inc_dec = INC;
			}
		}
		// If next_state is OFF
		else
		{
			printf("RL OFF \n");

			readingLight1.pwmTarget = 0;
			readingLight1.inc_dec = DEC;
			readingLight2.pwmTarget = 0;
			readingLight2.inc_dec = DEC;
		}
		
		// Set dim = ON and assign target PWM
		readingLight1 = dimStart(readingLight1, readingLight, readingLight1.pwmTarget);
		readingLight2 = dimStart(readingLight2, readingLight, readingLight2.pwmTarget);
		
		// Update state transition
		readingLight.prev_state = readingLight.next_state;
		
	}
}



struct ledRGBFeature dimRGBStart(struct ledRGBFeature rl, struct rgbRaw target)
{
	
	rl.dim = ON;
	rl.timerExp = getTimer() + rl.rate;
	if (rl.pwmRaw_R > target.r)
		rl.inc_dec = DEC;
	else
		rl.inc_dec = INC;

	// add rollover mechanism here ...
	
	//------------------------------------------------------------------
	// set pwmTarget from target ...
	
	rl.pwmTarget_R = target.r;
	rl.pwmTarget_G = target.g;
	rl.pwmTarget_B = target.b;
	
	//------------------------------------------------------------------
	// Calculate numSteps
	//
	uint16_t numSteps = 50;
	
	//------------------------------------------------------------------
	// Calculate step for each
	// step = (target - raw) / numSteps
	
	//rl.step_r = (uint16_t)((float)(rl.pwmTarget_R - rl.pwmRaw_R) / (float)numSteps);
	rl.step_r = (uint16_t)((rl.pwmTarget_R - rl.pwmRaw_R) / numSteps);
	rl.step_g = (uint16_t)((rl.pwmTarget_G - rl.pwmRaw_G) / numSteps);
	rl.step_b = (uint16_t)((rl.pwmTarget_B - rl.pwmRaw_B) / numSteps);
	
	printf("rl.step_r: %d\n", rl.step_r);
	printf("rl.step_g: %d\n", rl.step_g);
	printf("rl.step_b: %d\n", rl.step_b);
	
	return rl;
}

uint16_t setTarget(uint8_t rgb)
{
	float f_target = ((float)rgb/255.0)*4095.0;
	uint16_t target = (uint16_t)f_target;
	
	return target;
}


struct ledFeature svcLightFeature(struct ledFeature rl)
{
	// Is dimming enabled?
	if (rl.dim == ON)
	{
		printf("rl.pwmRaw: %d\n", rl.pwmRaw);
		// Check for timer expiration
		if (bcm2835_st_read() >= rl.timerExp)
		{
			// Check if pwmTarget reached
			if ((rl.inc_dec == INC) & (rl.pwmRaw >= rl.pwmTarget))
				rl.dim = OFF;
			else if ((rl.inc_dec == DEC) & (rl.pwmRaw <= rl.pwmTarget))
				rl.dim = OFF;
				
			// Incremement / Decrement
			if ((rl.inc_dec == INC) & (rl.pwmRaw < rl.pwmTarget))
				rl.pwmRaw += DIMMING_INC;
			else if ((rl.inc_dec == DEC) & (rl.pwmRaw > rl.pwmTarget))
				rl.pwmRaw -= DIMMING_INC;
				
			// Update PCA 
			write_lighting_feature(rl);
			
			// Update new timer expiration
			rl.timerExp = getTimer() + rl.rate;			
					
		}
		// Consider out-of-bounds conditions for pwmRaw ...
	}
	
	return rl;
}


struct ledFeature dimStart(struct ledFeature rl, struct rlFeature state, uint16_t target)
{
	rl.dim = ON;
	rl.timerExp = getTimer() + rl.rate;

	// add rollover mechanism here ...
	
	// set pwmTarget from target ...
	rl.pwmTarget = target;
	
	return rl;
}


struct ledRGBFeature svcRGBLightFeature(struct ledRGBFeature rl)
{
	printf("rgb rl.dim: %d\n", rl.dim);
	// Is dimming enabled?
	if (rl.dim == ON)
	{
		printf("pwmRaw_R: %d\n", rl.pwmRaw_R);
		// Check for timer expiration
		if (bcm2835_st_read() >= rl.timerExp)
		{
			// Check if pwmTarget reached
			if ((rl.inc_dec == INC) & (rl.pwmRaw_R >= rl.pwmTarget_R))
				rl.dim = OFF;
			else if ((rl.inc_dec == DEC) & (rl.pwmRaw_R <= rl.pwmTarget_R))
				rl.dim = OFF;
				
			// Incremement / Decrement
			if ((rl.inc_dec == INC) & (rl.pwmRaw_R < rl.pwmTarget_R))
			{
				rl.pwmRaw_R += rl.step_r;
				rl.pwmRaw_G += rl.step_g;
				rl.pwmRaw_B += rl.step_b;
			}
			else if ((rl.inc_dec == DEC) & (rl.pwmRaw_R > rl.pwmTarget_R))
			{
				rl.pwmRaw_R += rl.step_r;
				rl.pwmRaw_G += rl.step_g;
				rl.pwmRaw_B += rl.step_b;
			}
				
			// Update PCA 
			write_RGBlighting_feature(rl);
			
			// Update new timer expiration
			rl.timerExp = getTimer() + rl.rate;			
					
		}
		// Consider out-of-bounds conditions for pwmRaw ...
	}
	
	return rl;
}


void set_initial_conditions(void)
{
	// Turn on Status LED
	set_led(ST_LED);

	// Mute Audio Amplifier
	mute_audio();


	/*******************************************/
	/************** READING LIGHTS *************/
	// Set Reading Light 1 address
	readingLight1.address = CH7_BASE;
	readingLight1.device = PCA1;

	// Set Reading Light 2 address
	readingLight2.address = CH8_BASE;
	readingLight2.device = PCA1;
	
	// Default Reading Light Intensity
	readingLight.intensity = RL_DEFAULT_MAX;
	
	// Inititialize Reading Light(s) to start from 0
	readingLight1.pwmRaw = 0;
	readingLight2.pwmRaw = 0;
	
	// Set to fade on
	readingLight1.inc_dec = INC;
	readingLight1.rate = DIMMING_RATE;
	readingLight1.dim = OFF;
	readingLight2.inc_dec = DEC;
	readingLight2.rate = DIMMING_RATE;
	readingLight2.dim = OFF;
		
	// Read uSW and set reading light state and target
	if (bcm2835_gpio_lev(NEU_USW))
	{
		readingLight.mode = RL2;
		readingLight1.pwmTarget = 0;
		readingLight2.pwmTarget = readingLight.intensity;
	}
	else
	{
		readingLight.mode = RL1;
		readingLight1.pwmTarget = readingLight.intensity;
		readingLight2.pwmTarget = 0;
	}
	
	printf("RL.mode: %d\n", readingLight.mode);
	
	// Trigger to turn on reading light by default
	readingLight.next_state = 1;
	readingLight.prev_state = 0;
	
	/*******************************************/
	
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
			
	/********************** WASH SETTINGS *****************************/		
	// Set target RGB values
	neuRGB.r = NEU_RGB_R;
	neuRGB.g = NEU_RGB_G;
	neuRGB.b = NEU_RGB_B;
	layRGB.r = LAY_RGB_R;
	layRGB.g = LAY_RGB_G;
	layRGB.b = LAY_RGB_B;
	ttlRGB.r = TTL_RGB_R;
	ttlRGB.g = TTL_RGB_G;
	ttlRGB.b = TTL_RGB_B;
		
	// Set Backshell Light
	backshellLight.state = OFF;
	backshellLight.rate = RGB_DIMMING_RATE;
	backshellLight.pwmRaw_R = offRGB.r;//RGB_R_DEFAULT;
	backshellLight.pwmRaw_G = offRGB.g;//RGB_G_DEFAULT;
	backshellLight.pwmRaw_B = offRGB.b;//GB_B_DEFAULT;
	backshellLight.address = CH13_BASE;
	backshellLight.device = PCA1;

	// Set Footwell Light
	footwellLight.state = OFF;
	footwellLight.rate = RGB_DIMMING_RATE;
	footwellLight.pwmRaw_R = offRGB.r;
	footwellLight.pwmRaw_G = offRGB.g;
	footwellLight.pwmRaw_B = offRGB.b;
	footwellLight.address = CH3_BASE;
	footwellLight.device = PCA2;
	
	// Set Under IFE Light
	underIFELight.state = OFF;
	underIFELight.rate = RGB_DIMMING_RATE;
	underIFELight.pwmRaw_R = offRGB.r;
	underIFELight.pwmRaw_G = offRGB.g;
	underIFELight.pwmRaw_B = offRGB.b;
	underIFELight.address = CH0_BASE;
	underIFELight.device = PCA2;
	
	// Set Stowage Light
	stowageLight.state = OFF;
	stowageLight.rate = RGB_DIMMING_RATE;
	stowageLight.pwmRaw_R = offRGB.r;
	stowageLight.pwmRaw_G = offRGB.g;
	stowageLight.pwmRaw_B = offRGB.b;
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

// Pull system timer for use with dimming
uint64_t getTimer(void)
{
	return bcm2835_st_read();
}


/* ----------------------- SWITCH ACTIONS --------------------------- */
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
			readingLight1.pwmTarget = 0;
			readingLight1.inc_dec = DEC;
			readingLight2.pwmTarget = readingLight.intensity;
			readingLight2.inc_dec = INC;
			
			// Initiate RGB Dimming Features
			backshellLight = dimRGBStart(backshellLight, neuRGB);
			stowageLight = dimRGBStart(stowageLight, neuRGB);
			footwellLight = dimRGBStart(footwellLight, neuRGB);
			underIFELight = dimRGBStart(underIFELight, neuRGB);
			
			
		}
		// Change to use .state - don't pull lev twice...
		else if (!bcm2835_gpio_lev(NEU_USW) & readingLight.mode == RL2)
		{
			printf("/NEU\n");
			readingLight.mode = RL1;
			readingLight1.pwmTarget = readingLight.intensity;
			readingLight1.inc_dec = INC;
			readingLight2.pwmTarget = 0;
			readingLight2.inc_dec = DEC;
			
			// Initiate RGB Dimming Features
			backshellLight = dimRGBStart(backshellLight, ttlRGB);
			stowageLight = dimRGBStart(stowageLight, ttlRGB);
			footwellLight = dimRGBStart(footwellLight, ttlRGB);
			underIFELight = dimRGBStart(underIFELight, ttlRGB);
		}

		// Initiate reading light dimming
		readingLight1 = dimStart(readingLight1, readingLight, readingLight1.pwmTarget);
		readingLight2 = dimStart(readingLight2, readingLight, readingLight2.pwmTarget);
		
		// Update prev_state
		neu_usw.prev_state = neu_usw.next_state;
	}

	
	return;
}


void svc_LAY_usw(void)
{
	// Detect level on LAY uSW
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


/* -------------------------- BTN ACTIONS --------------------------- */

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

// Detect RL BTN on Cap Touch UI
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

// Engage relay for Seat Control Unit - Lay Flat Position
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

// Engage relay for Seat Control Unit - TTL Position
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

// Detect RL BTN on Reading Light Assy
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


void illuminateCapTouch(uint16_t delay)
{
	write_lighting_feature(capTTLLight);
	delay_ms(delay);
	write_lighting_feature(capLAYLight);
	delay_ms(delay);
	write_lighting_feature(capRLLight);
	delay_ms(delay);
	write_lighting_feature(capDNDWhiteLight);
	delay_ms(delay);
	write_lighting_feature(capATTDWhiteLight);
	delay_ms(delay);
	
	return;
}
