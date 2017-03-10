
/* ====================== CAP UI VERSION ============================ */
#define CAP_VER_2

/* ==================== WASH COLOR DEFAULT ========================== */
#define CUSTOM2_WASH

/* -------------------------- BIDIR --------------------------------- */
#define UART_TX	RPI_V2_GPIO_P1_08
#define UART_RX	RPI_V2_GPIO_P1_10

/* ------------------------ OUTPUTS --------------------------------- */
#define ST_LED 	RPI_V2_GPIO_P1_32	// Status LED
#define TTL_SW 	RPI_V2_GPIO_P1_15	// TTL Relay
#define NEU_SW 	RPI_V2_GPIO_P1_13	// NEU Relay
#define LAY_SW 	RPI_V2_GPIO_P1_11	// LAY Relay
#define MUTE 	RPI_V2_GPIO_P1_07	// Amplifier Mute
#define SEL 	RPI_V2_GPIO_P1_22	// OLED Select
#define RE_DE	RPI_V2_GPIO_P1_12	// RS-485 = /Read, Write

/* ------------------------ INPUTS ---------------------------------- */

// CAP Version 1
#ifdef CAP_VER_1
#define CAP_ATTD RPI_V2_GPIO_P1_29		// ATTD
#define CAP_DND	RPI_V2_GPIO_P1_31		// DND
#define CAP_RL 	RPI_V2_GPIO_P1_33		// RL
#define CAP_LF 	RPI_V2_GPIO_P1_35		// LF
#define CAP_TTL RPI_V2_GPIO_P1_37		// TTL
#endif

// CAP Version 2
#ifdef CAP_VER_2
#define CAP_ATTD RPI_V2_GPIO_P1_37	// ATTD
#define CAP_DND	RPI_V2_GPIO_P1_35	// DND
#define CAP_RL 	RPI_V2_GPIO_P1_33	// RL
#define CAP_LF 	RPI_V2_GPIO_P1_31	// LF
#define CAP_TTL RPI_V2_GPIO_P1_29	// TTL
#endif

#define NEU_USW RPI_V2_GPIO_P1_18
#define LAY_USW RPI_V2_GPIO_P1_16

#define CAP_RL_BTN RPI_V2_GPIO_P1_40 // Cap switch at RL

/* ----------------------- LIGHTING CONSTANTS ----------------------- */
#define MODE_READ 0
#define MODE_WRITE 1
#define MAX_LEN 32

#define DIMMING_INC		10
#define DIMMING_RATE	0	// Higher is slower... 	(20000 ~ 5 seconds)
							//						(    0 ~ 2 seconds)

#ifdef RED_WASH
#define RGB_R_DEFAULT	1000
#define RGB_G_DEFAULT	0
#define RGB_B_DEFAULT	0
#elif defined GREEN_WASH
#define RGB_R_DEFAULT	0
#define RGB_G_DEFAULT	1000
#define RGB_B_DEFAULT	0
#elif defined BLUE_WASH
#define RGB_R_DEFAULT	0
#define RGB_G_DEFAULT	0
#define RGB_B_DEFAULT	1000
#elif defined WHITE_WASH
#define RGB_R_DEFAULT	500
#define RGB_G_DEFAULT	500
#define RGB_B_DEFAULT	500
#elif defined CUSTOM1_WASH
#define RGB_R_DEFAULT	255
#define RGB_G_DEFAULT	197
#define RGB_B_DEFAULT	60
#elif defined CUSTOM2_WASH
#define RGB_R_DEFAULT	2000
#define RGB_G_DEFAULT	1545
#define RGB_B_DEFAULT	510
#endif

#define CAP_LIGHT_DEFAULT	500
#define CAP_LIGHT_HIGH		4000
#define CAP_LIGHT_OFF		0
#define RL_DEFAULT_MAX		2000

#define OFF 	0
#define ON 		1
#define WHITE 	2
#define BLUE	3
#define RED		4

#define RL1		1
#define RL2		2

#define INC		1
#define DEC		0

/* ------------------------ PCA9685 CONSTANTS ----------------------- */
#define MODE1 0x00
#define MODE2 0x01
#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09
#define LED2_ON_L 0x0E
#define ALL_LED_ON 0xFA
#define ALL_LED_OFF 0xFC
#define PRE_SCALE 0xFE

#define CH0_BASE 0x06
#define CH1_BASE 0x0A
#define CH2_BASE 0x0E
#define CH3_BASE 0x12
#define CH4_BASE 0x16
#define CH5_BASE 0x1A
#define CH6_BASE 0x1E
#define CH7_BASE 0x22
#define CH8_BASE 0x26
#define CH9_BASE 0x2A
#define CH10_BASE 0x2E
#define CH11_BASE 0x32
#define CH12_BASE 0x36
#define CH13_BASE 0x3A
#define CH14_BASE 0x3E
#define CH15_BASE 0x42

// CAP Version 1
#ifdef CAP_VER_1
#define CAP_TTL_BASE	CH6_BASE
#define CAP_LAY_BASE	CH5_BASE
#define CAP_RL_BASE		CH4_BASE
#define CAP_DND_RED_BASE	CH3_BASE
#define CAP_DND_WHITE_BASE	CH2_BASE
#define CAP_ATTD_BLUE_BASE	CH1_BASE
#define CAP_ATTD_WHITE_BASE	CH0_BASE
#endif

// CAP Version 2
#ifdef CAP_VER_2
#define CAP_TTL_BASE	CH0_BASE
#define CAP_LAY_BASE	CH1_BASE
#define CAP_RL_BASE		CH2_BASE
#define CAP_DND_RED_BASE	CH3_BASE
#define CAP_DND_WHITE_BASE	CH4_BASE
#define CAP_ATTD_BLUE_BASE	CH5_BASE
#define CAP_ATTD_WHITE_BASE	CH6_BASE
#endif

#define PCA1 0x42
#define PCA2 0x41

char mode_msg[] = {MODE1, 0x80};		// MODE1 => 0x01, reset chip
char autoinc_msg[] = {MODE1, 0xA1}; 	// MODE1 => 0xA1, auto-increment
char cfg_msg[] = {MODE2, 0x04}; 		// MODE2 => 0x04, hardware configuration
