//------------------------------------------------------------------------------
/**
 * @file server.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L Server app header.
 * @version 0.1
 * @date 2022-10-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#ifndef __SERVER_H__
#define __SERVER_H__

//------------------------------------------------------------------------------
// Default global value
//------------------------------------------------------------------------------
#define	SERVER_FB_DEVICE		"/dev/fb0"
#define	SERVER_UI_CONFIG		"ui.cfg"

#define	FIND_UART_NODE_BASE		"/sys/devices"

#define	SERVER_UART_L_DEVICE	"/dev/ttyUSB0"
#define	SERVER_UART_L_USB_PORT	"usb/usb1/1-1/1-1.4/1-1.4:1.0"
#define	SERVER_UART_R_DEVICE	"/dev/ttyUSB1"
#define	SERVER_UART_R_USB_PORT	"usb1/1-1/1-1.3/1-1.3:1.0"

#define	SERVER_I2C_L_PORT		"/dev/i2c-1"
#define	SERVER_I2C_R_PORT		"/dev/i2c-0"

#define	WATCHDOG_CHECK_INTERVAL	1000	/* 1 sec */
#define	WATCHDOG_RESET_COUNT	60	    // 60 sec

#define	CMD_CHAR_MAX	        128
#define	CMD_SEND_INTERVAL	    20      // 20 ms
#define	CMD_COUNT_MAX	        256
#define	POWER_PINS_MAX	        16

#define	ALIVE_DISPLAY_R_ITEM	0
#define	ALIVE_DISPLAY_IMTERVAL	1000	/* 1000 ms */

#define	FINISH_DISPLAY_R_ITEM_L	162
#define	FINISH_DISPLAY_R_ITEM_R	166
#define	APP_LOOP_DELAY			500		/* 500 us */

#define	STATUS_L_UART_R_ITEM	42
#define	STATUS_R_UART_R_ITEM	46

#define	POWER_CHECK_INTERVAL	500		/* 500ms */
#define	STATUS_CHECK_INTERVAL	500

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
enum eCHANNEL {
	CH_L = 0,
	CH_R,
	CH_END
};

//------------------------------------------------------------------------------
enum SYSTEM_STATE {
	SYSTEM_START = 0,
	/* Server system boot */
	SYSTEM_INIT,
	/* target power off */
	SYSTEM_WAIT,
	/* target power on */
	SYSTEM_BOOT,
	/* boot cmd received from target */
	SYSTEM_RUNNING,
	/* cmd count == target cmd pos */
	SYSTEM_FINISH,
	/* after 1 min poower on not received boot cmd from traget */
	SYSTEM_ERROR
};

//------------------------------------------------------------------------------
typedef struct channel__t {
	/* UART Control struct */
	ptc_grp_t	*puart;

	/* command를 보내고 데이터를 기다리는 상태 표시 */
	bool	is_busy;
	/* ttyUSB node가 있어 UART open이 가능한 상태표시 */
	bool	is_available;
	/* POWER_PIN으로 정의 되어진 모든 PIN이 정상인지 표시 */
	bool	power_status;
	
	/* Client 보드와 통신하기 위한 USB uart node 이름 */
	char	dev_uart_name[128];
	char	usb_port[128];

	/* ADC Board를 control하기 위한 device node 및 fd */
	int		fd_i2c;
	char	dev_i2c_name[128];

	/* Client와 Server간의 BOOT cmd의 명령어 처리 상태 표시 */
	bool	is_connect;

	/* 진행중인 테스트 command위치 */
	int		cmd_pos;

	/* command send control time */
	struct timeval t;

	/* watchdog count */
	int				watchdog_cnt;

	/* Busy status의 경우 일정시간 cmd delay */
	int				cmd_wait_delay;	

	/* Test result display */
	int				finish_r_item;

    /* channel run state */
    char            state;
}	channel_t;

//------------------------------------------------------------------------------
typedef struct power_pins__t {
	char	adc_name[16];	/* ADC Port name */
	int		v_max, v_min;
}	power_pins_t;

//------------------------------------------------------------------------------
typedef struct cmd__t {
	bool		is_info, is_str, is_adc;
	int			uid[2];
	char		group[10];
	char		action[10];
	char		adc_name[16];
	int			max, min;
	/* command result */
	bool		result[2];
}	cmd_t;

//------------------------------------------------------------------------------
struct server_t {

	int			alive_r_item;

	/* FB name */
	char		fb_dev[128];
	char		ui_config[128];

	fb_info_t	*pfb;
	ui_grp_t	*pui;
	channel_t	channel[2];
	
	int				power_pin_count;
	power_pins_t	power_pins[POWER_PINS_MAX];

	int				cmd_count;
	cmd_t 			cmds[CMD_COUNT_MAX];
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const __u16	HeaderToGPIO[] = {
	  0,   0,	// |  1 : 3.3V      ||  2 : 5.0V      |
	493,   0,	// |  3 : I2C-2 SDA ||  4 : 5.0V      |
	494,   0,	// |  5 : I2C-2 SCL ||  6 : GND       |
	473, 488,	// |  7 : GPIOA.13  ||  8 : UART_TX_A |
	  0, 489,	// |  9 : GND       || 10 : UART_RX_A |
	479, 492,	// | 11 : GPIOX.3   || 12 : PWM_E     |
	480,   0,	// | 13 : GPIOX.4   || 14 : GND       |
	483, 476,	// | 15 : GPIOX.7   || 16 : GPIOX.0   |
	  0, 477,	// | 17 : 3.3V      || 18 : GPIOX.1   |
	484,   0,	// | 19 : SPI0_MOSI || 20 : GND       |
	485, 478,	// | 21 : SPI0_MISO || 22 : GPIOX.2   |
	487, 486,	// | 23 : SPI0_CLK  || 24 : SPI0_CS0  |
	  0, 464,	// | 25 : GND       || 26 : GPIOA.4   |
	474, 475,	// | 27 : I2C-3 SDA || 28 : I2C-3 SCL |
	490,   0,	// | 29 : GPIOX.14  || 30 : GND       |
	491, 472,	// | 31 : GPIOX.15  || 32 : GPIOA.12  |
	481,   0,	// | 33 : GPIOX.5   || 34 : GND       |
	482, 495,	// | 35 : GPIOX.6   || 36 : GPIOX.19  |
	  0,   0,	// | 37 : ADC.AIN3  || 38 : REF 1.8V  |
	  0,   0	// | 39 : GND       || 40 : ADC.AIN2  |
};

const __u16	Patterns[4][40] = {
	{
		// ALL High Pattern
		0,   0,	// |  1 : 3.3V      ||  2 : 5.0V      |
		1,   0,	// |  3 : I2C-2 SDA ||  4 : 5.0V      |
		1,   0,	// |  5 : I2C-2 SCL ||  6 : GND       |
		1,   1,	// |  7 : GPIOA.13  ||  8 : UART_TX_A |
		0,   1,	// |  9 : GND       || 10 : UART_RX_A |
		1,   1,	// | 11 : GPIOX.3   || 12 : PWM_E     |
		1,   0,	// | 13 : GPIOX.4   || 14 : GND       |
		1,   1,	// | 15 : GPIOX.7   || 16 : GPIOX.0   |
		0,   1,	// | 17 : 3.3V      || 18 : GPIOX.1   |
		1,   0,	// | 19 : SPI0_MOSI || 20 : GND       |
		1,   1,	// | 21 : SPI0_MISO || 22 : GPIOX.2   |
		1,   1,	// | 23 : SPI0_CLK  || 24 : SPI0_CS0  |
		0,   1,	// | 25 : GND       || 26 : GPIOA.4   |
		1,   1,	// | 27 : I2C-3 SDA || 28 : I2C-3 SCL |
		1,   0,	// | 29 : GPIOX.14  || 30 : GND       |
		1,   1,	// | 31 : GPIOX.15  || 32 : GPIOA.12  |
		1,   0,	// | 33 : GPIOX.5   || 34 : GND       |
		1,   1,	// | 35 : GPIOX.6   || 36 : GPIOX.19  |
		0,   0,	// | 37 : ADC.AIN3  || 38 : REF 1.8V  |
		0,   0	// | 39 : GND       || 40 : ADC.AIN2  |
	},	{
		// ALL Clear Pattern
		0,   0,	// |  1 : 3.3V      ||  2 : 5.0V      |
		0,   0,	// |  3 : I2C-2 SDA ||  4 : 5.0V      |
		0,   0,	// |  5 : I2C-2 SCL ||  6 : GND       |
		0,   0,	// |  7 : GPIOA.13  ||  8 : UART_TX_A |
		0,   0,	// |  9 : GND       || 10 : UART_RX_A |
		0,   0,	// | 11 : GPIOX.3   || 12 : PWM_E     |
		0,   0,	// | 13 : GPIOX.4   || 14 : GND       |
		0,   0,	// | 15 : GPIOX.7   || 16 : GPIOX.0   |
		0,   0,	// | 17 : 3.3V      || 18 : GPIOX.1   |
		0,   0,	// | 19 : SPI0_MOSI || 20 : GND       |
		0,   0,	// | 21 : SPI0_MISO || 22 : GPIOX.2   |
		0,   0,	// | 23 : SPI0_CLK  || 24 : SPI0_CS0  |
		0,   0,	// | 25 : GND       || 26 : GPIOA.4   |
		0,   0,	// | 27 : I2C-3 SDA || 28 : I2C-3 SCL |
		0,   0,	// | 29 : GPIOX.14  || 30 : GND       |
		0,   0,	// | 31 : GPIOX.15  || 32 : GPIOA.12  |
		0,   0,	// | 33 : GPIOX.5   || 34 : GND       |
		0,   0,	// | 35 : GPIOX.6   || 36 : GPIOX.19  |
		0,   0,	// | 37 : ADC.AIN3  || 38 : REF 1.8V  |
		0,   0	// | 39 : GND       || 40 : ADC.AIN2  |
	},	{
		// Cross Logic Pattern 1
		0,   0,	// |  1 : 3.3V      ||  2 : 5.0V      |
		1,   0,	// |  3 : I2C-2 SDA ||  4 : 5.0V      |
		0,   0,	// |  5 : I2C-2 SCL ||  6 : GND       |
		1,   0,	// |  7 : GPIOA.13  ||  8 : UART_TX_A |
		0,   1,	// |  9 : GND       || 10 : UART_RX_A |
		1,   0,	// | 11 : GPIOX.3   || 12 : PWM_E     |
		0,   0,	// | 13 : GPIOX.4   || 14 : GND       |
		1,   0,	// | 15 : GPIOX.7   || 16 : GPIOX.0   |
		0,   1,	// | 17 : 3.3V      || 18 : GPIOX.1   |
		1,   0,	// | 19 : SPI0_MOSI || 20 : GND       |
		0,   1,	// | 21 : SPI0_MISO || 22 : GPIOX.2   |
		1,   0,	// | 23 : SPI0_CLK  || 24 : SPI0_CS0  |
		0,   1,	// | 25 : GND       || 26 : GPIOA.4   |
		1,   0,	// | 27 : I2C-3 SDA || 28 : I2C-3 SCL |
		0,   0,	// | 29 : GPIOX.14  || 30 : GND       |
		1,   0,	// | 31 : GPIOX.15  || 32 : GPIOA.12  |
		0,   0,	// | 33 : GPIOX.5   || 34 : GND       |
		1,   0,	// | 35 : GPIOX.6   || 36 : GPIOX.19  |
		0,   0,	// | 37 : ADC.AIN3  || 38 : REF 1.8V  |
		0,   0	// | 39 : GND       || 40 : ADC.AIN2  |
	},	{
		// Cross Logic Pattern 2
		0,   0,	// |  1 : 3.3V      ||  2 : 5.0V      |
		0,   0,	// |  3 : I2C-2 SDA ||  4 : 5.0V      |
		1,   0,	// |  5 : I2C-2 SCL ||  6 : GND       |
		0,   1,	// |  7 : GPIOA.13  ||  8 : UART_TX_A |
		0,   0,	// |  9 : GND       || 10 : UART_RX_A |
		0,   1,	// | 11 : GPIOX.3   || 12 : PWM_E     |
		0,   0,	// | 13 : GPIOX.4   || 14 : GND       |
		0,   1,	// | 15 : GPIOX.7   || 16 : GPIOX.0   |
		0,   0,	// | 17 : 3.3V      || 18 : GPIOX.1   |
		0,   0,	// | 19 : SPI0_MOSI || 20 : GND       |
		1,   0,	// | 21 : SPI0_MISO || 22 : GPIOX.2   |
		0,   1,	// | 23 : SPI0_CLK  || 24 : SPI0_CS0  |
		0,   0,	// | 25 : GND       || 26 : GPIOA.4   |
		0,   1,	// | 27 : I2C-3 SDA || 28 : I2C-3 SCL |
		1,   0,	// | 29 : GPIOX.14  || 30 : GND       |
		0,   1,	// | 31 : GPIOX.15  || 32 : GPIOA.12  |
		1,   0,	// | 33 : GPIOX.5   || 34 : GND       |
		0,   1,	// | 35 : GPIOX.6   || 36 : GPIOX.19  |
		0,   0,	// | 37 : ADC.AIN3  || 38 : REF 1.8V  |
		0,   0	// | 39 : GND       || 40 : ADC.AIN2  |
	},
};

//------------------------------------------------------------------------------
// Function prototype
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define	UPTIME_DISPLAY_S_ITEM	7
#define	IPADDR_DISPLAY_S_ITEM	22

#if defined(UPTIME_DISPLAY_S_ITEM)
	#define	UPTIME_DISPLAY_INTERVAL	1
	void server_uptime_display (struct server_t *pserver);
#endif

#if defined(IPADDR_DISPLAY_S_ITEM)
	#define	IPADDR_DISPLAY_INTERVAL	30
	void server_ipaddr_display (struct server_t *pserver);
#endif
//------------------------------------------------------------------------------
void	find_uart_dev 			(struct server_t *pserver, int channel);
void	server_cmd_load 		(struct server_t *pserver);
void	power_pin_load 			(struct server_t *pserver);
void	app_cfg_load 			(struct server_t *pserver);
void	app_protocol_install 	(struct server_t *pserver);
int		app_init 				(struct server_t *pserver);
void	app_exit 				(struct server_t *pserver);
void	power_pins_check 		(struct server_t *pserver);
void	system_watchdog 		(struct server_t *pserver);
void	server_status_display 	(struct server_t *pserver);
void	server_alive_display 	(struct server_t *pserver);
bool	adc_pattern_check 		(int *values, int pin_cnt, char pattern_no, int max, int min);
void	client_msg_catch 		(struct server_t *pserver, char ch, char ret_ack, char *msg);
void	client_msg_parser 		(struct server_t *pserver);
void	cmd_sned_control 		(struct server_t *pserver);
int		main					(int argc, char **argv);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif  // #define __SERVER_H__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
