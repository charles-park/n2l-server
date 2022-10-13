//------------------------------------------------------------------------------
/**
 * @file client.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG Clint application.
 * @version 0.1
 * @date 2022-10-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
// for my lib
//------------------------------------------------------------------------------
/* 많이 사용되는 define 정의 모음 */
#include "typedefs.h"
#include "common.h"
#include "./lib_fb/lib_fb.h"
#include "./lib_ui/lib_ui.h"
#include "./lib_uart/lib_uart.h"
#include "./lib_adc/lib_adc.h"

#include "protocol.h"
#include "server.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void find_uart_dev (struct server_t *pserver, int channel)
{

	FILE *fp;
	char rdata[256], *ptr;

	memset  (rdata, 0x00, sizeof(rdata));
	sprintf (rdata, "find %s/ -name ttyUSB* 2<&1", FIND_UART_NODE_BASE);

	// UART (ttyUSB) find...
	if ((fp = popen(rdata, "r")) != NULL) {
		memset (rdata, 0x00, sizeof(rdata));
		while (fgets (rdata, sizeof(rdata), fp) != NULL) {
			if ((ptr = strstr (rdata, pserver->channel[channel].usb_port)) != NULL) {
				if ((ptr = strstr (rdata, "ttyUSB")) != NULL) {
					pserver->channel[channel].is_available = true;
					sprintf (pserver->channel[channel].dev_uart_name, "/dev/ttyUSB%c", *(ptr + 6));
					pclose(fp);
					return;
				}
			}
			memset (rdata, 0x00, sizeof(rdata));
		}
		pclose(fp);
	}
}

//------------------------------------------------------------------------------
void server_cmd_load (struct server_t *pserver)
{
	char cmd_line[CMD_CHAR_MAX], *ptr;
	char cmds[CMD_CHAR_MAX * CMD_COUNT_MAX];

	memset (cmds, 0x00, sizeof(cmds));
	find_appcfg_data ("SERVER_CMD",  cmds);

	for (	pserver->cmd_count = 0;
			pserver->cmd_count < CMD_COUNT_MAX;
			pserver->cmd_count++)	{

		memset (cmd_line, 0x00, sizeof(cmd_line));
		memcpy (cmd_line, &cmds[pserver->cmd_count * CMD_CHAR_MAX], sizeof(cmd_line));

		if (cmd_line[0] != 0x00) {
			ptr = strtok (cmd_line, ",");
			if (ptr == NULL)	continue;
			pserver->cmds[pserver->cmd_count].uid[0] = atoi(ptr);
			
			ptr = strtok (NULL, ",");
			if (ptr == NULL)	continue;
			pserver->cmds[pserver->cmd_count].uid[1] = atoi(ptr);

			ptr = toupperstr (strtok (NULL, ","));
			if (ptr == NULL)	continue;
			strncpy (pserver->cmds[pserver->cmd_count].group, ptr, strlen(ptr));
			
			ptr = toupperstr (strtok (NULL, ","));
			if (ptr == NULL)	continue;
			strncpy (pserver->cmds[pserver->cmd_count].action, ptr, strlen(ptr));
			
			ptr = strtok (NULL, ",");
			if (ptr == NULL)	continue;
			pserver->cmds[pserver->cmd_count].is_info =	atoi(ptr) ? true : false;

			ptr = strtok (NULL, ",");
			if (ptr == NULL)	continue;
			pserver->cmds[pserver->cmd_count].is_str  = atoi(ptr) ? true : false;

			ptr = strtok (NULL, ",");
			if (ptr == NULL)	continue;
			pserver->cmds[pserver->cmd_count].is_adc  = atoi(ptr) ? true : false;

			if (pserver->cmds[pserver->cmd_count].is_adc) {
				ptr = toupperstr (strtok (NULL, ","));
				if (ptr == NULL)	continue;
				strncpy (pserver->cmds[pserver->cmd_count].adc_name, ptr, strlen(ptr));

				ptr = strtok (NULL, ",");
				if (ptr == NULL)	continue;
				pserver->cmds[pserver->cmd_count].max = atoi(ptr);

				ptr = strtok (NULL, ",");
				if (ptr == NULL)	continue;
				pserver->cmds[pserver->cmd_count].min = atoi(ptr);
			}
		}
		else	break;
	}

	{
		int i;
		for (i = 0; i < pserver->cmd_count; i++) {
			info ("CMD %02d, %03d %03d %10s %10s %d %d %d %10s %04d %04d\n",
				i +1,
				pserver->cmds[i].uid[0], pserver->cmds[i].uid[1], 
				pserver->cmds[i].group, pserver->cmds[i].action, 
				pserver->cmds[i].is_info, pserver->cmds[i].is_str, 
				pserver->cmds[i].is_adc, pserver->cmds[i].adc_name, 
				pserver->cmds[i].max, pserver->cmds[i].min);
		}
	}
}

//------------------------------------------------------------------------------
void power_pin_load (struct server_t *pserver)
{
	char cmd_line[CMD_CHAR_MAX], *ptr;
	char cmds[CMD_CHAR_MAX * CMD_COUNT_MAX];

	memset (cmds, 0x00, sizeof(cmds));
	find_appcfg_data ("POWER_PIN",  cmds);

	for (	pserver->power_pin_count = 0;
			pserver->power_pin_count < POWER_PINS_MAX;
			pserver->power_pin_count++)	{

		memset (cmd_line, 0x00, sizeof(cmd_line));
		memcpy (cmd_line, &cmds[pserver->power_pin_count * CMD_CHAR_MAX], sizeof(cmd_line));
		if (cmd_line[0] != 0x00) {
			ptr = toupperstr (strtok (cmd_line, ","));
			if (ptr == NULL)	continue;
			strncpy (pserver->power_pins[pserver->power_pin_count].adc_name, ptr, strlen(ptr));
			
			ptr = strtok (NULL, ",");
			if (ptr == NULL)	continue;
			pserver->power_pins[pserver->power_pin_count].v_max = atoi(ptr);

			ptr = strtok (NULL, ",");
			if (ptr == NULL)	continue;
			pserver->power_pins[pserver->power_pin_count].v_min = atoi(ptr);
		}
		else	break;
	}

	{
		int i;
		for (i = 0; i < pserver->power_pin_count; i++) {
			info ("POWER PIN %02d, %10s, max(%04d), min(%04d)\n",
				i +1,
				pserver->power_pins[i].adc_name, 
				pserver->power_pins[i].v_max, 
				pserver->power_pins[i].v_min);
		}
	}
}

//------------------------------------------------------------------------------
void app_cfg_load (struct server_t *pserver)
{
	char int_str[8];
	if (find_appcfg_data ("SERVER_FB_DEVICE",     pserver->fb_dev))
		sprintf (pserver->fb_dev,    "%s", SERVER_FB_DEVICE);
	if (find_appcfg_data ("SERVER_UI_CONFIG",     pserver->ui_config))
		sprintf (pserver->ui_config, "%s", SERVER_UI_CONFIG);

	if (find_appcfg_data ("SERVER_UART_L_USB_PORT", pserver->channel[CH_L].usb_port))
		sprintf (pserver->channel[CH_L].usb_port,  "%s", SERVER_UART_L_USB_PORT);
	if (find_appcfg_data ("SERVER_UART_R_USB_PORT", pserver->channel[CH_R].usb_port))
		sprintf (pserver->channel[CH_R].usb_port,  "%s", SERVER_UART_R_USB_PORT);

	if (find_appcfg_data ("SERVER_I2C_L_PORT", pserver->channel[CH_L].dev_i2c_name))
		sprintf (pserver->channel[CH_L].dev_i2c_name, "%s", SERVER_I2C_L_PORT);
	if (find_appcfg_data ("SERVER_I2C_R_PORT", pserver->channel[CH_R].dev_i2c_name))
		sprintf (pserver->channel[CH_R].dev_i2c_name, "%s", SERVER_I2C_R_PORT);

	memset (int_str, 0x00, sizeof(int_str));
	if (find_appcfg_data ("ALIVE_DISPLAY_R_ITEM", int_str))
		pserver->alive_r_item = ALIVE_DISPLAY_R_ITEM;
	else
		pserver->alive_r_item = atoi(int_str);
	
	memset (int_str, 0x00, sizeof(int_str));
	if (find_appcfg_data ("FINISH_DISPLAY_R_ITEM_L", int_str))
		pserver->channel[CH_L].finish_r_item = FINISH_DISPLAY_R_ITEM_L;
	else
		pserver->channel[CH_L].finish_r_item = atoi(int_str);

	memset (int_str, 0x00, sizeof(int_str));
	if (find_appcfg_data ("FINISH_DISPLAY_R_ITEM_R", int_str))
		pserver->channel[CH_R].finish_r_item = FINISH_DISPLAY_R_ITEM_R;
	else
		pserver->channel[CH_R].finish_r_item = atoi(int_str);
}

//------------------------------------------------------------------------------
int app_init (struct server_t *pserver) 
{
	info ("---------------------------------\n");
	info ("[ %s : %s ]\n", __FILE__, __func__);

	memset (pserver, 0x00, sizeof(struct server_t));

	// APP config data read
	app_cfg_load    (pserver);
	server_cmd_load (pserver);
	power_pin_load  (pserver);

	info ("SERVER_FB_DEVICE        = %s\n", pserver->fb_dev);
	info ("SERVER_UI_CONFIG        = %s\n", pserver->ui_config);
	info ("ALIVE_DISPLAY_R_ITEM    = %d\n", pserver->alive_r_item);
	info ("FINISH_DISPLAY_R_ITEM_L = %d\n", pserver->channel[CH_L].finish_r_item);
	info ("FINISH_DISPLAY_R_ITEM_R = %d\n", pserver->channel[CH_R].finish_r_item);
	info ("SERVER_UART_L_USB_PORT  = %s\n", pserver->channel[CH_L].usb_port);
	info ("SERVER_UART_R_USB_PORT  = %s\n", pserver->channel[CH_R].usb_port);

	pserver->channel[CH_L].fd_i2c = adc_board_init (pserver->channel[CH_L].dev_i2c_name);
	info ("SERVER_I2C_L_PORT       = %s\n", pserver->channel[CH_L].dev_i2c_name);
	info ("SERVER_I2C_L_PORT_FD    = %d\n", pserver->channel[CH_L].fd_i2c);
	pserver->channel[CH_R].fd_i2c = adc_board_init (pserver->channel[CH_R].dev_i2c_name);
	info ("SERVER_I2C_R_PORT       = %s\n", pserver->channel[CH_R].dev_i2c_name);
	info ("SERVER_I2C_R_PORT_FD    = %d\n", pserver->channel[CH_R].fd_i2c);

	find_uart_dev (pserver, CH_L);
	info ("SERVER_UART_L_DEVICE   = %s\n", pserver->channel[CH_L].dev_uart_name);
	find_uart_dev (pserver, CH_R);
	info ("SERVER_UART_R_DEVICE   = %s\n", pserver->channel[CH_R].dev_uart_name);

	pserver->pfb	= fb_init 	(pserver->fb_dev);
	pserver->pui	= ui_init	(pserver->pfb, pserver->ui_config) ;
	if ((pserver->pfb == NULL) || (pserver->pui == NULL)) {
		err ("SYSTEM Initialize fail(FB/UI)\n");
		exit(0);
	}

	// UART Protocol Inatsll
	{
		int i = 0;
		for (i = 0; i < CH_END; i++) {
			if (pserver->channel[i].is_available) {
				pserver->channel[i].puart = uart_init (pserver->channel[i].dev_uart_name, B115200);
				if (pserver->channel[i].puart) {
					if (ptc_grp_init (pserver->channel[i].puart, 1)) {
						if (!ptc_func_init (pserver->channel[i].puart, 0, sizeof(recv_protocol_u),
												protocol_check, protocol_catch)) {
							err ("UART %s protocol install fail\n", pserver->channel[i].dev_uart_name);
						} else {
							info ("UART %s protocol install success.\n", pserver->channel[i].dev_uart_name);
							protocol_msg_send (pserver->channel[i].puart, 'P', 1, "REBOOT", "-");
						}
					}
				}
				else
					pserver->channel[i].is_available = false;
			}
		}
	}
	info ("---------------------------------\n");
	return 0;
}

//------------------------------------------------------------------------------
void app_exit (struct server_t *pserver)
{
	{
		int i;
		for (i = 0; i < CH_END; i++)
		uart_close(pserver->channel[i].puart);
	}
	ui_close  (pserver->pui);
	fb_clear  (pserver->pfb);
	fb_close  (pserver->pfb);
}

//------------------------------------------------------------------------------
void power_pins_check (struct server_t *pserver)
{
	int i, ch;
	int values[40], cnt, err_cnt;

	for (ch = 0; ch < 2; ch ++) {
		for (i = 0, err_cnt = 0; i < pserver->power_pin_count; i++) {
			
			if (!pserver->channel[ch].fd_i2c) {
				err_cnt++;
				continue;
			}

			adc_read_pin (pserver->channel[ch].fd_i2c,
				pserver->power_pins[i].adc_name, &values[0], &cnt);

			if ((values[0] > pserver->power_pins[i].v_max) ||
				(values[0] < pserver->power_pins[i].v_min)) {
				err_cnt++;
				err ("ch %d, %d > max %d or %d < min %d",
					ch,
					values[0], pserver->power_pins[i].v_max,
					values[0], pserver->power_pins[i].v_min);
			}
		}
		pserver->channel[ch].power_status = err_cnt ? false : true;
		if (!pserver->channel[ch].power_status) {
			pserver->channel[ch].cmd_pos = 0;		pserver->channel[ch].is_busy = 0;
			pserver->channel[ch].is_connect = 0;	pserver->channel[ch].watchdog_cnt = 0;
		}
	}
}

//------------------------------------------------------------------------------
void system_watchdog (struct server_t *pserver)
{
	char ch;
	for (ch = 0; ch < CH_END; ch++) {
		if (!pserver->channel[ch].power_status)
			continue;
		if (!pserver->channel[ch].is_available)
			continue;
		if (pserver->channel[ch].watchdog_cnt++ < WATCHDOG_RESET_COUNT)
			continue;
		pserver->channel[ch].cmd_pos = 0;
		pserver->channel[ch].is_busy = 0;
		pserver->channel[ch].is_connect = 0;
		pserver->channel[ch].watchdog_cnt = 0;
		protocol_msg_send (pserver->channel[ch].puart, 'P', 1, "REBOOT", "-");
		info ("%s : channel = %d\n", __func__, ch);
	}
}

//------------------------------------------------------------------------------
void server_alive_display (struct server_t *pserver)
{
	static bool onoff = false, have_net = false;
	static int cmd = 0;

	if (run_interval_check(&pserver->t, ALIVE_DISPLAY_IMTERVAL)) {
		ui_set_ritem (pserver->pfb, pserver->pui, pserver->alive_r_item,
					onoff ? COLOR_GREEN : pserver->pui->bc.uint, -1);
		onoff = !onoff;

		power_pins_check (pserver);
		system_watchdog (pserver);

		if (!pserver->channel[CH_L].is_available || !pserver->channel[CH_L].fd_i2c) {
			char err_msg[20];
			memset (err_msg, 0x00, sizeof(err_msg));

			ui_set_ritem (pserver->pfb, pserver->pui, STATUS_L_UART_R_ITEM,	COLOR_RED, -1);
			ui_set_str (pserver->pfb, pserver->pui, STATUS_L_UART_R_ITEM,
                  -1, -1, 3, -1, "L_CH : UART=%d I2C=%d", pserver->channel[CH_L].is_available,
												pserver->channel[CH_L].fd_i2c);
		}

		if (!pserver->channel[CH_R].is_available|| !pserver->channel[CH_R].fd_i2c) {
			char err_msg[20];
			memset (err_msg, 0x00, sizeof(err_msg));

			ui_set_ritem (pserver->pfb, pserver->pui, STATUS_R_UART_R_ITEM,	COLOR_RED, -1);
			ui_set_str (pserver->pfb, pserver->pui, STATUS_R_UART_R_ITEM,
                  -1, -1, 3, -1, "R_CH : UART=%d I2C=%d", pserver->channel[CH_R].is_available,
												pserver->channel[CH_R].fd_i2c);
		}

		#if defined(UPTIME_DISPLAY_S_ITEM)
		{
			char uptime[10];

			memset (uptime, 0x00, sizeof(uptime));		uptime_str(uptime);
			ui_set_str (pserver->pfb, pserver->pui, UPTIME_DISPLAY_S_ITEM,
                  -1, -1, 4, -1, "%s", uptime);
		}
		#endif

		#if defined(IPADDR_DISPLAY_S_ITEM)
		{
			char ip_addr[20], mac_addr[20];
			int link_speed;
			memset (ip_addr, 0x00, sizeof(ip_addr));
			get_netinfo(mac_addr, ip_addr, &link_speed);
			ui_set_str (pserver->pfb, pserver->pui, IPADDR_DISPLAY_S_ITEM,
                  -1, -1, 4, -1, "%s", ip_addr);
		}
		#endif
	}
}

//------------------------------------------------------------------------------
bool adc_pattern_check (int *values, int pin_cnt, char pattern_no, int max, int min)
{
	int i, err_cnt;

	for (i = 0, err_cnt = 0; i < pin_cnt; i++)	{
		if (HeaderToGPIO[i]) {
			if (Patterns[pattern_no][i]) {
				if (values[i] < max) {
					err_cnt++;
					err ("Patterm %d : pin value = %d, %d < max %d\n",
						pattern_no, i+1, values[i], max);
				}
			} else {
				if (values[i] > min) {
					err_cnt++;
					err ("Patterm %d : pin value = %d, %d > min %d\n",
						pattern_no, i+1, values[i], min);
				}
			}
		}
	}
	return	err_cnt ? false : true;
}

//------------------------------------------------------------------------------
void client_msg_catch (struct server_t *pserver, char ch, char ret_ack, char *msg)
{
	// 보내진 uid와 지금 uid가 같은 경우 busy flag off
	int uid, status, str_pos, len;
	char msg_str[20];
	channel_t *pchannel = &pserver->channel[ch];

	memset (msg_str, 0x00, sizeof(msg_str));
	memcpy (msg_str, &msg[0], 3);

	uid    = atoi (msg_str);
	status = (msg[3] == '1') ? 1 : 0;
	str_pos = 5;	len = sizeof(msg_str);
	while ((msg[str_pos++] == ' ') && len--);

	memset  (msg_str, 0x00, sizeof(msg_str));
	strncpy (msg_str, &msg [str_pos-1], sizeof(msg_str));

	/* 보내진 UI ID와 받은 UI ID가 맞는지 확인 */
	if (uid == pserver->cmds[pchannel->cmd_pos].uid[ch]) {
		if (pserver->cmds[pchannel->cmd_pos].is_adc) {
			/* ADC Header Pin Max is 40 */
			int values[40], cnt;
			adc_read_pin (pchannel->fd_i2c,
				pserver->cmds[pchannel->cmd_pos].adc_name, &values[0], &cnt);

			if (!strncmp (pserver->cmds[pchannel->cmd_pos].group,
				"HEADER", sizeof("HEADER"))) {
				status = adc_pattern_check (
					values,
					cnt,
					msg_str[0] - '0',
					pserver->cmds[pchannel->cmd_pos].max,
					pserver->cmds[pchannel->cmd_pos].min);

				memset (msg_str, 0x00, sizeof(msg_str));
				sprintf(msg_str, "%s", status ? "PASS" : "FAIL");
			} else {
				if ((pserver->cmds[pchannel->cmd_pos].max < values[0]) ||
					(pserver->cmds[pchannel->cmd_pos].min > values[0]))
					status = 0;
				else
					status = 1;
			}
		}
		info ("%s, %s, %s, UID %d, STATUS %d, MSG : %s\n",
				pchannel->dev_uart_name,
				pserver->cmds[pchannel->cmd_pos].group,
				pserver->cmds[pchannel->cmd_pos].action,
				uid, status, msg_str);

		/* app.cfg의 설정 참조 */
		if (!pserver->cmds[pchannel->cmd_pos].is_info) {
			ui_set_ritem (pserver->pfb, pserver->pui, uid,
						status ? COLOR_GREEN : COLOR_RED, -1);
		}
		/* app.cfg의 설정 참조 */
		if (pserver->cmds[pchannel->cmd_pos].is_str)
			ui_set_str (pserver->pfb, pserver->pui, uid,
                  -1, -1, -1, -1, "%s", msg_str);

		ui_update (pserver->pfb, pserver->pui, uid);
	}
	else
		err ("%s : CH %s, UID mismatch %d, %d\n",
			__func__, pchannel->dev_uart_name, uid, pserver->cmds[pchannel->cmd_pos].uid[ch]);

	if (pchannel->cmd_pos < pserver->cmd_count-1)
		pchannel->cmd_pos++;
	else {
		pchannel->cmd_pos = 0;
		ui_update (pserver->pfb, pserver->pui, -1);
		// finish icon display : total status
	}
	pchannel->watchdog_cnt = 0;
}

//------------------------------------------------------------------------------
void client_msg_parser (struct server_t *pserver)
{
	char msg[CMD_CHAR_MAX], ch, cmd;
	channel_t *pchannel;

	for (ch = 0; ch < CH_END; ch++) {
		pchannel = &pserver->channel[ch];
		if (!pchannel->is_available)
			continue;
		memset (msg, 0x00, sizeof(msg));
		if (protocol_msg_check (pchannel->puart, &cmd, msg)) {
			switch (cmd) {
				case	'P':
					pchannel->is_connect = false;	pchannel->is_busy = false;
				break;
				case	'R':
					pchannel->is_connect = true;	pchannel->is_busy = false;
					pchannel->cmd_pos = 0;
					protocol_msg_send (pchannel->puart, 'A', 1, "BOOT", "-");
				break;
				case	'A':	case	'O':	case	'E':
					// msg parse & display
					if (pchannel->is_connect && pserver->cmd_count) {
						client_msg_catch (pserver, ch, cmd, msg);
						pchannel->is_busy = false;
					}
				break;
				case	'B':
					pchannel->is_busy = false;
					pchannel->cmd_wait_delay = (1000 / CMD_SEND_INTERVAL);
					info ("CH %s : Device Busy\n", pchannel->dev_uart_name);
				break;
				default :
				break;
			}
			memset (msg, 0x00, sizeof(msg));
			pchannel->watchdog_cnt = 0;
		}
	}
}

//------------------------------------------------------------------------------
void cmd_sned_control (struct server_t *pserver)
{
	char ch;
	channel_t *pchannel;

	for (ch = 0; ch < CH_END; ch++) {
		pchannel = &pserver->channel[ch];
		if (run_interval_check(&pchannel->t, CMD_SEND_INTERVAL)) {
			if (!pserver->channel[ch].is_available)
				continue;
			if (pchannel->cmd_wait_delay) {
				pchannel->cmd_wait_delay--;
				continue;
			}
			if (pchannel->power_status) {
				if (!pchannel->is_connect || pchannel->is_busy)
					continue;
				if (!pserver->cmd_count)
					continue;

				protocol_msg_send (pchannel->puart, 'C',
								pserver->cmds[pchannel->cmd_pos].uid[ch],
								pserver->cmds[pchannel->cmd_pos].group,
								pserver->cmds[pchannel->cmd_pos].action);
				pchannel->is_busy = true;
				pchannel->watchdog_cnt = 0;
			}
		}
	}
}

//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	struct server_t	server;

	app_init (&server);

	while (true) {
		server_alive_display (&server);
		client_msg_parser    (&server);
		cmd_sned_control     (&server);
		usleep(APP_LOOP_DELAY);
	}

	app_exit(&server);
	return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
