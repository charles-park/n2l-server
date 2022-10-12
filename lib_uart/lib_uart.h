//------------------------------------------------------------------------------
/**
 * @file lib_uart.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief UART control library
 * @version 0.1
 * @date 2022-05-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#ifndef __LIB_UART_H__
#define __LIB_UART_H__

//------------------------------------------------------------------------------
#include <errno.h>      // Error integer and strerror() function
#include <fcntl.h>      // Contains file controls like O_RDWR
#include <pthread.h>
#include <termios.h>    // Contains POSIX terminal control definitions
#include <unistd.h>     // write(), read(), close()

#include "../typedefs.h"
//------------------------------------------------------------------------------
#define DEFAULT_QUEUE_SIZE      1024

//------------------------------------------------------------------------------
typedef struct queue__t {
    __u32   sp;
    __u32   ep;
    __u32   size;
    __u8    *buf;
}   queue_t;

typedef struct protocol_variable__t {
	__u32	p_sp;
	__u32	p_ep;
	__u32	size;
	bool	open;
	bool	pass;
	__u8	*buf;
}   ptc_var_t;

typedef struct protocol_function__t {
    ptc_var_t   var;
    int         (*pcheck)(ptc_var_t *p);
    int         (*pcatch)(ptc_var_t *p);
}   ptc_func_t;

typedef struct protocol_group__t {
    int         fd;
    __u8        pcnt;
	ptc_func_t  *p;
    pthread_t   rx_thread, tx_thread;
    queue_t     tx_q, rx_q;
}   ptc_grp_t;

//------------------------------------------------------------------------------
extern  bool        queue_get       (queue_t *q, __u8 *d);
extern  bool        queue_put       (queue_t *q, __u8 *d);
extern  void        ptc_set_status  (ptc_grp_t *ptc_grp, __u8 ptc_num, bool status);
extern  void        ptc_q           (ptc_grp_t *ptc_grp, __u8 ptc_num, __u8 idata);
extern  void        ptc_event       (ptc_grp_t *ptc_grp, __u8 idata);
extern  bool        ptc_func_init   (ptc_grp_t *ptc_grp, __u8 ptc_num, __u8 ptc_size, 
                int (*chk_func)(ptc_var_t *var), int (*cat_func)(ptc_var_t *var));
extern  bool        ptc_grp_init    (ptc_grp_t *ptc_grp, __u8 ptc_count);
extern  void        ptc_grp_close   (ptc_grp_t *ptc_grp);
//------------------------------------------------------------------------------
extern  ptc_grp_t   *uart_init      (const char *dev_name, speed_t baud);
extern  void        uart_close      (ptc_grp_t *ptc_grp);

//------------------------------------------------------------------------------
#endif  // #define __LIB_UART_H__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
