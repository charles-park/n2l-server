//------------------------------------------------------------------------------
/**
 * @file protocol.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG UART protocol application.
 * @version 0.1
 * @date 2022-10-3
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#ifndef	__PROTOCOL_H__
#define	__PROTOCOL_H__

#include "typedefs.h"

//------------------------------------------------------------------------------
/* Data send to Client : protocol size is 32 bytes */
//------------------------------------------------------------------------------
#pragma packet(1)
typedef struct send_protocol__t {
	/* @ : start protocol signal */
	__s8	head;

	/*
		command description:
			server to client : 'C'ommand, 'R'eady(boot)
			client to server : 'O'kay, 'A'ck, 'R'eady(boot), 'E'rror, 'B'usy
	*/
	__s8	cmd;
	__s8	uid[3];
	__s8	group[10];
	/* msg no, msg group, msg data1, msg data2, ... */
	__s8	data[10];

	__s8	reserved[6];
	/* # : end protocol signal */
	__s8	tail;
}	send_protocol_t;

#pragma packet(1)
typedef union send_protocol__u {
	send_protocol_t 	p;
	__u8				bytes[sizeof(send_protocol_t)];
}	send_protocol_u;

//------------------------------------------------------------------------------
/* Data receive from Client : protocol size is 32 bytes */
#pragma packet(1)
typedef struct recv_protocol__t {
	/* @ : start protocol signal */
	__s8	head;

	/*
		command description:
			server to client : 'C'ommand, 'R'eady(boot)
			client to server : 'O'kay, 'A'ck, 'R'eady(boot), 'E'rror, 'B'usy
	*/
	__s8	resp;
	__s8	uid[3];
	__s8	status;

	/* msg no, msg group, msg data1, msg data2, ... */
	__s8	data[16];

	__s8	reserved[9];
	/* # : end protocol signal */
	__s8	tail;
}	recv_protocol_t;

#pragma packet(1)
typedef union recv_protocol__u {
	recv_protocol_t 	p;
	__u8				bytes[sizeof(recv_protocol_t)];
}	recv_protocol_u;

//------------------------------------------------------------------------------
// function prototype define
//------------------------------------------------------------------------------
extern	int 	protocol_catch		(ptc_var_t *var);
extern	int 	protocol_check		(ptc_var_t *var);
extern	void 	protocol_msg_send	(ptc_grp_t *puart, char cmd, int uid, char *group, char *action);
extern	int 	protocol_msg_check 	(ptc_grp_t *puart, char *recv_cmd, char *recv_msg);

//------------------------------------------------------------------------------
#endif	// #define	__PROTOCOL_H__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
