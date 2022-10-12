//------------------------------------------------------------------------------
/**
 * @file protocol.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG UART protocol application.
 * @version 0.1
 * @date 2022-10-3
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include "lib_uart/lib_uart.h"
/* protocol control 함수 */
#include "protocol.h"

//------------------------------------------------------------------------------
//
// https://docs.google.com/spreadsheets/d/18J4B4bqgUbMBA8XeoDkMcKPVEAeNCP2jQm5TOlgKUAo/edit#gid=744952288
//
//------------------------------------------------------------------------------
int protocol_check (ptc_var_t *var)
{
	/* head & tail check with protocol size */
	if(var->buf[(var->p_sp + var->size -1) % var->size] != '#')	return 0;
	if(var->buf[(var->p_sp               ) % var->size] != '@')	return 0;
	return 1;
}

//------------------------------------------------------------------------------
int protocol_catch (ptc_var_t *var)
{
	char cmd = var->buf[(var->p_sp + 1) % var->size];

	switch (cmd) {
		case 'R':	case 'P':	case 'A':
		case 'O':	case 'E':	case 'B':
		info ("Receive cmd = %c\n", cmd);
		return 1;
		default :
		break;
	}
	err ("Unknown command = %c\n", cmd);
	return	0;
}

//------------------------------------------------------------------------------
void protocol_msg_send (ptc_grp_t *puart, char cmd, int uid, char *group, char *action)
{
	int i;
	char s_uid[4], s_group[11], s_data[11];
	send_protocol_u	send;

	memset(&send, ' ', sizeof(send_protocol_u));

	memset (s_uid, 0x00, sizeof(s_uid));
	sprintf (s_uid, "%d", uid);
	strncpy(send.p.uid, s_uid, sizeof(send.p.uid));

	memset (s_group, 0x00, sizeof(s_group));
	sprintf (s_group, "%s", group);
	strncpy(send.p.group, s_group, sizeof(send.p.group));

	memset (s_data, 0x00, sizeof(s_data));
	sprintf (s_data, "%s", action);
	strncpy (send.p.data, s_data, sizeof(send.p.data));

	send.p.head = '@';	send.p.tail = '#';
	send.p.cmd  = cmd;	// cmd

	fprintf(stdout, "Send to client [fd = %d] -> ", puart->fd);
	for (i = 0; i < sizeof(send_protocol_u); i++) {
		queue_put(&puart->tx_q, &send.bytes[i]);
		fprintf(stdout, "%c", send.bytes[i]);
	}
	fprintf(stdout, "\n");

	{
		char LF = '\n', CR = '\r';
		queue_put(&puart->tx_q, &LF);
		queue_put(&puart->tx_q, &CR);
	}
}

//------------------------------------------------------------------------------
int protocol_msg_check (ptc_grp_t *puart, char *recv_cmd, char *recv_msg)
{
	__u8 idata, p_cnt;

	/* uart data processing */
	if (queue_get (&puart->rx_q, &idata))
		ptc_event (puart, idata);

	for (p_cnt = 0; p_cnt < puart->pcnt; p_cnt++) {
		if (puart->p[p_cnt].var.pass) {
			ptc_var_t *var = &puart->p[p_cnt].var;
			int i;

			puart->p[p_cnt].var.pass = false;
			puart->p[p_cnt].var.open = true;

			/* recv cmd */
			*recv_cmd = var->buf[(var->p_sp + 1) % var->size];
			/* uuid(3) + group(10) + item(10) char size = 23 */
			for (i = 0; i < 20; i++) {
				// uuid start position is 2
				recv_msg [i] = var->buf[(var->p_sp + 2 + i) % var->size];
			}
			return	true;
		}
	}
	return	false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
