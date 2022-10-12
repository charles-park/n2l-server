//------------------------------------------------------------------------------
/**
 * @file common.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG common used function & define.
 * @version 0.1
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#ifndef	__COMMON_H__
#define	__COMMON_H__

//------------------------------------------------------------------------------
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/fb.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <linux/sockios.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>

//------------------------------------------------------------------------------
// Function prototype
//------------------------------------------------------------------------------
extern  int get_ip_addr 		(const char *eth_name, char *ip, int *link_speed);
extern  int get_mac_addr 		(char *mac_str);

extern  char *remove_space_str 	(char *str);
extern	char *toupperstr        (char *str);
extern	char *tolowerstr        (char *str);

extern  int fread_int           (char *filename);
extern  int fread_line          (char *filename, char *line, int l_size);

extern  int fwrite_bool			(char *filename, char status);
extern  int fwrite_str 			(char *filename, char *wstr);
extern  int find_appcfg_data    (char *fkey, char *fdata);

//------------------------------------------------------------------------------
#endif	// __COMMON_H__
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
