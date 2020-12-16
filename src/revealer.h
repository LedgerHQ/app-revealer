#include "os.h"
#include "bolos_ux_common.h"
#include "bolos_ux_nanos.h"

// Mersene twister algo defines
#define N  		   	624
#define M 		   	397
#define UPPER_MASK 	0x80000000U
#define LOWER_MASK 	0x7fffffffU
#define MATRIX_A   	0x9908b0dfU
#define KEY_LEN		5

// image defines
#define CHUNK_SIZE 	250
#define IMG_WIDTH  	159
#define IMG_HEIGHT 	97
#define IMG_YX 	    1+(IMG_WIDTH*IMG_HEIGHT)/8

// Nvm struct to store revealer image
typedef struct internalStorage_t {
	// Every byte of revealer image encodes 8 pixels
	// BYTE 0	BYTE 1 	...
	// bit7		bit7	...		
	// bit6		bit6	...
	// bit5		bit5	...
	// bit4		bit4	...
	// bit3		bit3	...
	// bit2		bit2	...
	// bit1		bit1	...
	// bit0		bit0	...
	// ...      ...		...
	// BYTE N   BYTE N+1
	uint8_t revealer_image[IMG_YX];
} internalStorage_t;

extern WIDE internalStorage_t N_storage_real;
#define N_storage (*(WIDE internalStorage_t *)PIC(&N_storage_real))

#define BAGL_FONT_FONT_LIGHT_16PX 0x00
#define BAGL_FONT_FONT_LIGHT_11PX 0x01
#define BAGL_FONT_FONT_SEMIBOLD_18PX 0x02
#define BAGL_FONT_FONT_BOLD_13PX 0x03
#define BAGL_FONT_FONT_EXTRABOLD_11PX 0x04

// Checks wether user entered noise seed is valid, last 3 chars of noise seed are last 3 char of hash(noise_seed)
uint8_t isNoise(char * string, uint8_t hashPos);
// Converts a slength hexadecimal values string to a slength/2 byte array returns 1 on success, 0 on failure
uint8_t hexStringToByteArray(char* string, uint8_t slength, uint8_t* byteArray);
// Initialises hmac_drbg key/val with default values and with user entered noise seed
void drbg_hmac_init(void);
// Inject new seed in hmac_drbg
void drbg_reseed(uint8_t * data, uint8_t dataPresent);
// Generates 512 (64*8) pixels from a sha 512 hmac
void drbg_generate(uint8_t *sha512_hmac, uint8_t *pixels);

void printTest(void);

// Writes pseudo random noise in revealer_image
