//------------------------------------------------------------------------------
/**
 * @file lib_adc.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ADC LTC2309 (12bits-8CH) control library.
 * @version 0.1
 * @date 2022-10-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//
// I2C ADC LTC2309 (12bits-8CH). (Use 8 Single-Ended, Unipolar Mode)
//
//------------------------------------------------------------------------------------------------------------
#include "../typedefs.h"
#include "../common.h"
#include "i2c.h"

//------------------------------------------------------------------------------------------------------------
// LTC2309 DEVICE ADDR
//------------------------------------------------------------------------------------------------------------
// LTC2309 Chip Pin Status (AD1, AD0)
//
// 0x08 = (  LOW,   LOW), 0x09 = (  LOW, FLOAT), 0x0A = ( LOW, HIGH), 0x0B = (FLOAT,  HIGH), 
// 0x18 = (FLOAT, FLOAT), 0x19 = (FLOAT,   LOW), 0x1A = (HIGH,  LOW), 0x1B = ( HIGH, FLOAT), 
// 0x14 = ( HIGH,  HIGH)
//
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
// LTC2309 Reg Bits
//------------------------------------------------------------------------------------------------------------
//
//  BIT7  BIT6  BIT5  BIT4  BIT3  BIT2  BIT1  BIT0
//  S/D   O/S   S1    S0    UNI    SLP   X     X
//
//  S/D : Single-ended/Defferential
//  O/S : ODD/Sign
//  S1  : Channel Select 1
//  S0  : Channel Select 0
//  UNI : UNIPOLAR/BIPOLAR
//  SLP : Sleep Mode
//
//  if ) Channel 0 Read
//       Single-ended = 1 | Sign = 0 | S1 = 0 | S0 = 0 | Unipolar = 1 | sleep = 0 | x | x | = 0x88
//  if ) Channel 1 Read
//       Single-ended = 1 | ODD = 1  | S1 = 0 | S0 = 0 | Unipolar = 1 | sleep = 0 | x | x | = 0xC8
//  if ) Channel 2 Read
//       Single-ended = 1 | Sign = 0 | S1 = 0 | S0 = 1 | Unipolar = 1 | sleep = 0 | x | x | = 0xA8
//  if ) Channel 3 Read
//       Single-ended = 1 | ODD = 1  | S1 = 0 | S0 = 1 | Unipolar = 1 | sleep = 0 | x | x | = 0xD8
//
//------------------------------------------------------------------------------------------------------------
#define SWAP_WORD(x)	(((x >> 8) & 0xFF) | ((x << 8) & 0xFF00))

#define	ADC_REF_VOLTAGE	5

// Reference 5V, ADC weight value = 1200uV
#define ADC_WEIGHT_uV	((ADC_REF_VOLTAGE * 1000000) / 4096)

enum {
	CHIP_ADC0 = 0,	
	CHIP_ADC1	,
	CHIP_ADC2	,
	CHIP_ADC3	,
	CHIP_ADC4	,
	CHIP_ADC5	,
	CHIP_ADC_CNT,
	NOT_USED	,	
};

const unsigned char ADC_ADDR[] = {
	0x08, 0x09, 0x0A, 0x0B, 0x18, 0x19
};

const unsigned char ADC_CH[] = {
	0x88, 0xC8, 0x98, 0xD8, 0xA8, 0xE8, 0xB8, 0xF8
};

struct pin_info {
	const char *name;
	unsigned char pin_num;
	unsigned char adc_idx;
	unsigned char ch_idx;
};

const struct pin_info HEADER_CON1[] = {
	{ "CON1.0" ,   0, NOT_USED , 0},	// Header Pin 0

	{ "CON1.1" ,   1, CHIP_ADC0, 0},	// Header Pin 1 Info
	{ "CON1.2" ,   2, CHIP_ADC0, 1},	// Header Pin 2 Info
	{ "CON1.3" ,   3, CHIP_ADC1, 0},
	{ "CON1.4" ,   4, CHIP_ADC0, 2},
	{ "CON1.5" ,   5, CHIP_ADC1, 1},
	{ "CON1.6" ,   6, NOT_USED , 0},
	{ "CON1.7" ,   7, CHIP_ADC1, 2},
	{ "CON1.8" ,   8, CHIP_ADC1, 3},
	{ "CON1.9" ,   9, NOT_USED , 0},
	{ "CON1.10",  10, CHIP_ADC1, 4},

	{ "CON1.11",  11, CHIP_ADC1, 5},
	{ "CON1.12",  12, CHIP_ADC1, 6},
	{ "CON1.13",  13, CHIP_ADC1, 7},
	{ "CON1.14",  14, NOT_USED , 0},
	{ "CON1.15",  15, CHIP_ADC2, 0},
	{ "CON1.16",  16, CHIP_ADC2, 1},
	{ "CON1.17",  17, CHIP_ADC0, 3},
	{ "CON1.18",  18, CHIP_ADC2, 2},
	{ "CON1.19",  19, CHIP_ADC2, 3},
	{ "CON1.20",  20, NOT_USED , 0},

	{ "CON1.21",  21, CHIP_ADC2, 4},
	{ "CON1.22",  22, CHIP_ADC2, 5},
	{ "CON1.23",  23, CHIP_ADC2, 6},
	{ "CON1.24",  24, CHIP_ADC2, 7},
	{ "CON1.25",  25, NOT_USED , 0},
	{ "CON1.26",  26, CHIP_ADC3, 0},
	{ "CON1.27",  27, CHIP_ADC3, 1},
	{ "CON1.28",  28, CHIP_ADC3, 2},
	{ "CON1.29",  29, CHIP_ADC3, 3},
	{ "CON1.30",  30, NOT_USED , 0},

	{ "CON1.31",  31, CHIP_ADC3, 4},
	{ "CON1.32",  32, CHIP_ADC3, 5},
	{ "CON1.33",  33, CHIP_ADC3, 6},
	{ "CON1.34",  34, NOT_USED , 0},
	{ "CON1.35",  35, CHIP_ADC3, 7},
	{ "CON1.36",  36, CHIP_ADC4, 0},
	{ "CON1.37",  37, NOT_USED , 0},
	{ "CON1.38",  38, CHIP_ADC0, 4},
	{ "CON1.39",  39, NOT_USED , 0},
	{ "CON1.40",  40, NOT_USED , 0},
};

const struct pin_info HEADER_P3[] = {
	{ "P3.0" ,  0, NOT_USED, 0},	// Header Pin 0
	{ "P3.1" ,  1, NOT_USED, 0},	// Header Pin 1 Info
	{ "P3.2" ,  2, CHIP_ADC5, 0},	// Header Pin 2 Info
	{ "P3.3" ,  3, CHIP_ADC5, 1},
	{ "P3.4" ,  4, NOT_USED, 0},
	{ "P3.5" ,  5, CHIP_ADC5, 2},
	{ "P3.6" ,  6, CHIP_ADC5, 3},
	{ "P3.7" ,  7, NOT_USED, 0},
	{ "P3.8" ,  8, CHIP_ADC5, 4},
	{ "P3.9" ,  9, CHIP_ADC5, 5},
	{ "P3.10", 10, NOT_USED, 0},
};

const struct pin_info HEADER_P13[] = {
	{ "P13.0", 0, NOT_USED , 0},	// Header Pin 0
	{ "P13.1", 1, NOT_USED , 0},	// Header Pin 1 Info
	{ "P13.2", 2, CHIP_ADC4, 1},	// Header Pin 2 Info
	{ "P13.3", 3, CHIP_ADC0, 5},
	{ "P13.4", 4, CHIP_ADC4, 2},
	{ "P13.5", 5, CHIP_ADC4, 3},
	{ "P13.6", 6, CHIP_ADC4, 4},
	{ "P13.7", 7, CHIP_ADC4, 5},
};

const struct pin_info HEADER_P1_1[] = {
	{ "P1_1.0", 0, NOT_USED , 0},	// Header Pin 0
	{ "P1_1.1", 1, CHIP_ADC0, 0},	// Header Pin 1 Info
	{ "P1_1.2", 2, CHIP_ADC0, 1},	// Header Pin 2 Info
	{ "P1_1.3", 3, CHIP_ADC0, 2},
	{ "P1_1.4", 4, CHIP_ADC0, 3},
	{ "P1_1.5", 5, CHIP_ADC0, 4},
	{ "P1_1.6", 6, CHIP_ADC0, 5},
	{ "P1_1.7", 7, CHIP_ADC0, 6},
	{ "P1_1.8", 8, CHIP_ADC0, 7},
};

const struct pin_info HEADER_P1_2[] = {
	{ "P1_2.0", 0, NOT_USED , 0},	// Header Pin 0
	{ "P1_2.1", 1, CHIP_ADC1, 0},	// Header Pin 1 Info
	{ "P1_2.2", 2, CHIP_ADC1, 1},	// Header Pin 2 Info
	{ "P1_2.3", 3, CHIP_ADC1, 2},
	{ "P1_2.4", 4, CHIP_ADC1, 3},
	{ "P1_2.5", 5, CHIP_ADC1, 4},
	{ "P1_2.6", 6, CHIP_ADC1, 5},
	{ "P1_2.7", 7, CHIP_ADC1, 6},
	{ "P1_2.8", 8, CHIP_ADC1, 7},
};

const struct pin_info HEADER_P1_3[] = {
	{ "P1_3.0", 0, NOT_USED , 0},	// Header Pin 0
	{ "P1_3.1", 1, CHIP_ADC2, 0},	// Header Pin 1 Info
	{ "P1_3.2", 2, CHIP_ADC2, 1},	// Header Pin 2 Info
	{ "P1_3.3", 3, CHIP_ADC2, 2},
	{ "P1_3.4", 4, CHIP_ADC2, 3},
	{ "P1_3.5", 5, CHIP_ADC2, 4},
	{ "P1_3.6", 6, CHIP_ADC2, 5},
	{ "P1_3.7", 7, CHIP_ADC2, 6},
	{ "P1_3.8", 8, CHIP_ADC2, 7},
};

const struct pin_info HEADER_P1_4[] = {
	{ "P1_4.0", 0, NOT_USED , 0},	// Header Pin 0
	{ "P1_4.1", 1, CHIP_ADC3, 0},	// Header Pin 1 Info
	{ "P1_4.2", 2, CHIP_ADC3, 1},	// Header Pin 2 Info
	{ "P1_4.3", 3, CHIP_ADC3, 2},
	{ "P1_4.4", 4, CHIP_ADC3, 3},
	{ "P1_4.5", 5, CHIP_ADC3, 4},
	{ "P1_4.6", 6, CHIP_ADC3, 5},
	{ "P1_4.7", 7, CHIP_ADC3, 6},
	{ "P1_4.8", 8, CHIP_ADC3, 7},
}; 

const struct pin_info HEADER_P1_5[] = {
	{ "P1_5.0", 0, NOT_USED , 0},	// Header Pin 0
	{ "P1_5.1", 1, CHIP_ADC4, 0},	// Header Pin 1 Info
	{ "P1_5.2", 2, CHIP_ADC4, 1},	// Header Pin 2 Info
	{ "P1_5.3", 3, CHIP_ADC4, 2},
	{ "P1_5.4", 4, CHIP_ADC4, 3},
	{ "P1_5.5", 5, CHIP_ADC4, 4},
	{ "P1_5.6", 6, CHIP_ADC4, 5},
	{ "P1_5.7", 7, CHIP_ADC4, 6},
	{ "P1_5.8", 8, CHIP_ADC4, 7},
};

const struct pin_info HEADER_P1_6[] = {
	{ "P1_6.0", 0, NOT_USED , 0},	// Header Pin 0
	{ "P1_6.1", 1, CHIP_ADC5, 0},	// Header Pin 1 Info
	{ "P1_6.2", 2, CHIP_ADC5, 1},	// Header Pin 2 Info
	{ "P1_6.3", 3, CHIP_ADC5, 2},
	{ "P1_6.4", 4, CHIP_ADC5, 3},
	{ "P1_6.5", 5, CHIP_ADC5, 4},
	{ "P1_6.6", 6, CHIP_ADC5, 5},
	{ "P1_6.7", 7, CHIP_ADC5, 6},
	{ "P1_6.8", 8, CHIP_ADC5, 7},
};

#define	ARRARY_SIZE(x)	(sizeof(x) / sizeof(x[0]))

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static	bool 			check_adc_device(int fd);
static	struct pin_info *header_info	(const char *h_name, char pin_no, char *p_cnt);
static	unsigned int	convert_to_mv 	(unsigned short adc_value);
static	int		 		read_pin_value 	(int fd, struct pin_info *info);
		int 			adc_board_init 	(const char *i2c_fname);
		bool 			adc_read_pin 	(int fd, const char *name, unsigned int *read_value, unsigned int *cnt);

//------------------------------------------------------------------------------
static int read_pin_value (int fd, struct pin_info *info)
{
	int read_val = 0, retry = 3;

	while (i2c_set_addr(fd, ADC_ADDR[info->adc_idx]) && retry --)
		usleep(100);

	if (retry) {
		// Dummy read for chip wake up & conversion
		i2c_read_word(fd, ADC_CH[info->ch_idx]);
		read_val  = i2c_read_word(fd, ADC_CH[info->ch_idx]);
		read_val  = (read_val < 0) ? 0 : read_val;
		read_val  = (SWAP_WORD(read_val) >> 4) & 0xFFF;
	}
	return read_val;
}

//------------------------------------------------------------------------------
static unsigned int convert_to_mv (unsigned short adc_value)
{
	unsigned int volt = adc_value * ADC_WEIGHT_uV;
	return	(volt / 1000);
}

//------------------------------------------------------------------------------
static struct pin_info *header_info(const char *h_name, char pin_no, char *p_cnt)
{
	const struct pin_info *p;

	if 			(!strncmp("CON1", h_name, sizeof(h_name))) {
		pin_no 	= pin_no < ARRARY_SIZE(HEADER_CON1) ? pin_no : 0;
		p 		= pin_no ? &HEADER_CON1[pin_no] : &HEADER_CON1[1];
		*p_cnt	= pin_no ? 1 : ARRARY_SIZE(HEADER_CON1) -1;
	} else if 	(!strncmp("P3"  , h_name, sizeof(h_name))) {
		pin_no 	= pin_no < ARRARY_SIZE(HEADER_P3) ? pin_no : 0;
		p 		= pin_no ? &HEADER_P3[pin_no] : &HEADER_P3[1];
		*p_cnt	= pin_no ? 1 : ARRARY_SIZE(HEADER_P3) -1;
	} else if	(!strncmp("P13" , h_name, sizeof(h_name))) {
		pin_no 	= pin_no < ARRARY_SIZE(HEADER_P13) ? pin_no : 0;
		p 		= pin_no ? &HEADER_P13[pin_no] : &HEADER_P13[1];
		*p_cnt	= pin_no ? 1 : ARRARY_SIZE(HEADER_P13) -1;
	} else if	(!strncmp("P1_1", h_name, sizeof(h_name))) {
		pin_no 	= pin_no < ARRARY_SIZE(HEADER_P1_1) ? pin_no : 0;
		p 		= pin_no ? &HEADER_P1_1[pin_no] : &HEADER_P1_1[1];
		*p_cnt	= pin_no ? 1 : ARRARY_SIZE(HEADER_P1_1) -1;
	} else if	(!strncmp("P1_2", h_name, sizeof(h_name))) {
		pin_no 	= pin_no < ARRARY_SIZE(HEADER_P1_2) ? pin_no : 0;
		p 		= pin_no ? &HEADER_P1_2[pin_no] : &HEADER_P1_2[1];
		*p_cnt	= pin_no ? 1 : ARRARY_SIZE(HEADER_P1_2) -1;
	} else if	(!strncmp("P1_3", h_name, sizeof(h_name))) {
		pin_no 	= pin_no < ARRARY_SIZE(HEADER_P1_3) ? pin_no : 0;
		p 		= pin_no ? &HEADER_P1_3[pin_no] : &HEADER_P1_3[1];
		*p_cnt	= pin_no ? 1 : ARRARY_SIZE(HEADER_P1_3) -1;
	} else if	(!strncmp("P1_4", h_name, sizeof(h_name))) {
		pin_no 	= pin_no < ARRARY_SIZE(HEADER_P1_4) ? pin_no : 0;
		p 		= pin_no ? &HEADER_P1_4[pin_no] : &HEADER_P1_4[1];
		*p_cnt	= pin_no ? 1 : ARRARY_SIZE(HEADER_P1_4) -1;
	} else if	(!strncmp("P1_5", h_name, sizeof(h_name))) {
		pin_no 	= pin_no < ARRARY_SIZE(HEADER_P1_5) ? pin_no : 0;
		p 		= pin_no ? &HEADER_P1_5[pin_no] : &HEADER_P1_5[1];
		*p_cnt	= pin_no ? 1 : ARRARY_SIZE(HEADER_P1_5) -1;
	} else if	(!strncmp("P1_6", h_name, sizeof(h_name))) {
		pin_no 	= pin_no < ARRARY_SIZE(HEADER_P1_6) ? pin_no : 0;
		p 		= pin_no ? &HEADER_P1_6[pin_no] : &HEADER_P1_6[1];
		*p_cnt	= pin_no ? 1 : ARRARY_SIZE(HEADER_P1_6) -1;
	} else {
		p = &HEADER_CON1[0];
		*p_cnt	= 0;
	}
	return (struct pin_info *)p;
}

//------------------------------------------------------------------------------
bool adc_read_pin (int fd, const char *name, unsigned int *read_value, unsigned int *cnt)
{
	char *h_name, *pin_str;
	char str[10], pin_no = 0, pin_cnt = 0, i;
	struct pin_info *p;

	if ((name == NULL) || !fd)
		return -1;

	memset(str, 0x00, sizeof(str));	
	memcpy(str, name, sizeof(name));

	if (h_name  = strtok(str, "."))
		toupperstr(h_name);

	if (pin_str = strtok(NULL, " "))
		pin_no = atoi(pin_str);

	p = header_info(h_name, pin_no, &pin_cnt);

	// DEBUG
	info ("%s : header = %s, pin = %d, pin_cnt = %d\n", name, h_name, pin_no, pin_cnt);

	if (pin_cnt) {
		for (i = 0; i < pin_cnt; i++, p++) {
			read_value[i] = convert_to_mv (read_pin_value(fd, p));
			info ("%s.%d, value = %d mV\n",
				h_name, (pin_cnt == 1) ? pin_no : i+1, read_value[i]);
		}
		*cnt = pin_cnt;
		return true;
	}
	else
		info ("can't found %s pin or header\n", name);

	return false;
}

//------------------------------------------------------------------------------
static bool check_adc_device (int fd)
{
	int i;

	for (i = 0; i < ARRARY_SIZE(ADC_ADDR); i++) {
		i2c_set_addr(fd, ADC_ADDR[i]);
		if(i2c_read_word(fd, ADC_ADDR[i]) < 0) {
			return	false;
		}
	}
	info ("%s : fd = %d\n", __func__, fd);
	return	true;
}

//------------------------------------------------------------------------------
int adc_board_init (const char *i2c_fname)
{
	int fd; 

	if ((fd = i2c_open(i2c_fname)) < 0)
		return   0;

	return	check_adc_device (fd) ? fd : 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

