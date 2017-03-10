	
// Single Channel Lighting Features ---
struct ledFeature {
	uint8_t 	state;
	uint16_t 	pwmRaw;
	uint16_t 	pwmTarget;
	uint64_t 	timerExp;
	//uint8_t 	rollover;
	uint32_t	rate;
	uint8_t	dim;
	uint8_t	inc_dec;
	uint16_t 	address;
	uint8_t 	device;
	};

struct ledFeature readingLight1;
struct ledFeature readingLight2;
struct ledFeature capTTLLight;
struct ledFeature capLAYLight;
struct ledFeature capRLLight;
struct ledFeature capDNDWhiteLight;
struct ledFeature capDNDRedLight;
struct ledFeature capATTDWhiteLight;
struct ledFeature capATTDBlueLight;
//-------------------------------------

// 3 Channel Lighting Features --------
struct ledRGBFeature {
	uint8_t 	state;
	uint16_t 	pwmRaw_R;
	uint16_t 	pwmTarget_R;
	uint16_t 	pwmRaw_G;
	uint16_t 	pwmTarget_G;
	uint16_t 	pwmRaw_B;
	uint16_t 	pwmTarget_B;
	int8_t		step_r;
	int8_t		step_g;
	int8_t		step_b;
	uint64_t	timerExp;
	uint32_t	rate;
	uint8_t	dim;
	uint8_t	inc_dec;
	uint16_t 	address;
	uint8_t 	device;
	};

struct ledRGBFeature backshellLight;	// Contains "pinlight" as well
struct ledRGBFeature footwellLight;	
struct ledRGBFeature underIFELight;
struct ledRGBFeature stowageLight;
//--------------------------------------

// RGB Structures
struct rgbRaw {
	uint16_t r;
	uint16_t g;
	uint16_t b;
};

struct rgbRaw neuRGB;
struct rgbRaw layRGB;
struct rgbRaw ttlRGB;
struct rgbRaw offRGB;

// Reading Light State -----------------
struct rlFeature {
	uint8_t prev_state;	// ON / OFF
	uint8_t next_state;	// ON / OFF
	uint8_t mode;			// RL1 / RL2
	uint16_t intensity;	// Use for transition dimming
	};

struct rlFeature readingLight; 
//--------------------------------------


// Switch States -----------------------
struct SW {
	uint8_t prev_state;
	uint8_t next_state;
	};

struct SW neu_usw;
struct SW lay_usw;
struct SW cap_rl_btn;
struct SW cap_attd;
struct SW cap_dnd;
//--------------------------------------


uint8_t init_bcm(void);
void init_periph(void);
void set_initial_conditions(void);
void write_lighting_feature(struct ledFeature);
void write_RGBlighting_feature(struct ledRGBFeature);
void i2c_end(void);
void close_bcm(void);
void delay_ms(unsigned int);
uint64_t getTimer(void);
void svc_ATTD_btn(void);
void svc_DND_btn(void);
void svc_RL_btn(void);
void svc_RL_btn2(void);
void svc_LAY_btn(void);
void svc_TTL_btn(void);
void set_led(uint8_t);
void clear_led(uint8_t);
void toggle_led(void);//uint8_t);
void svc_NEU_usw(void);
void svc_LAY_usw(void);
void mute_audio(void);
void unmute_audio(void);
void svc_Light_Features(void);
void svc_readingLight(void);
struct ledFeature svcLightFeature(struct ledFeature);
struct ledRGBFeature svcRGBLightFeature(struct ledRGBFeature);
struct ledFeature dimStart(struct ledFeature, struct rlFeature, uint16_t);
struct ledRGBFeature dimRGBStart(struct ledRGBFeature, struct rgbRaw);
void illuminateCapTouch(uint16_t);
struct rgbPCA translateRGBvalues(struct rgbRaw);
void init_lighting(void);
uint16_t setTarget(uint8_t);
