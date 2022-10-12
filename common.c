//------------------------------------------------------------------------------
/**
 * @file common.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG common used function.
 * @version 0.1
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#include "common.h"
#include "typedefs.h"

//------------------------------------------------------------------------------
#define DUPLEX_HALF 0x00
#define DUPLEX_FULL 0x01

#define ETHTOOL_GSET 0x00000001 /* Get settings command for ethtool */

struct ethtool_cmd {
	unsigned int cmd;
	unsigned int supported; /* Features this interface supports */
	unsigned int advertising; /* Features this interface advertises */
	unsigned short speed; /* The forced speed, 10Mb, 100Mb, gigabit */
	unsigned char duplex; /* Duplex, half or full */
	unsigned char port; /* Which connector port */
	unsigned char phy_address;
	unsigned char transceiver; /* Which tranceiver to use */
	unsigned char autoneg; /* Enable or disable autonegotiation */
	unsigned int maxtxpkt; /* Tx pkts before generating tx int */
	unsigned int maxrxpkt; /* Rx pkts before generating rx int */
	unsigned int reserved[4];
};

//------------------------------------------------------------------------------
struct eth_info_t {
	char	name[20];
	char	ip[20];
	char	mac[20];
	int		link_speed;
};

struct system_test_t {
	int		mem_size;	// GB size
	int		fb_x, fb_y;
	int		lcd_x, lcd_y;

	// Ethernet info
	struct eth_info_t eth_info;
};

//------------------------------------------------------------------------------
// function prototype define
//------------------------------------------------------------------------------
int 	get_ip_addr 		(const char *eth_name, char *ip, int *link_speed);
int 	get_mac_addr 		(char *mac_str);

char 	*remove_space_str 	(char *str);
char	*toupperstr			(char *str);
char	*tolowerstr			(char *str);
int		fread_int       	(char *filename);
int 	fread_line          (char *filename, char *line, int l_size);
int		fwrite_bool			(char *filename, char status);
int 	fwrite_str 			(char *filename, char *wstr);
int		find_appcfg_data	(char *fkey, char *fdata);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int get_ip_addr (const char *eth_name, char *ip, int *link_speed)
{
	int fd;
	struct ifreq ifr;
	struct ethtool_cmd ecmd;

	/* this entire function is almost copied from ethtool source code */
	/* Open control socket. */
	*link_speed = 0;
	sprintf (ip, "---.---.---.---");

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	strncpy(ifr.ifr_name, eth_name, IFNAMSIZ);
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
		return -1;

	inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2, ip, sizeof(struct sockaddr));

	/* Pass the "get info" command to eth tool driver */
	ecmd.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t)&ecmd;

	/* ioctl failed: */
	if (ioctl(fd, SIOCETHTOOL, &ifr))
	{
		close(fd);
		return -1;
	}
	close(fd);

	*link_speed = ecmd.speed;
	return 0;
}

//------------------------------------------------------------------------------
int get_mac_addr (char *mac_str)
{
	int sock, if_count, i;
	struct ifconf ifc;
	struct ifreq ifr[10];
	unsigned char mac[6];

	memset(&ifc, 0, sizeof(struct ifconf));

	sprintf(mac_str, "xx:xx:xx:xx:xx:xx");
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return	-1;

	// 검색 최대 개수는 10개
	ifc.ifc_len = 10 * sizeof(struct ifreq);
	ifc.ifc_buf = (char *)ifr;

	// 네트웨크 카드 장치의 정보 얻어옴.
	ioctl(sock, SIOCGIFCONF, (char *)&ifc);

	// 읽어온 장치의 개수 파악
	if_count = ifc.ifc_len / (sizeof(struct ifreq));
	for (i = 0; i < if_count; i++) {
		if (ioctl(sock, SIOCGIFHWADDR, &ifr[i]) == 0) {
			memcpy(mac, ifr[i].ifr_hwaddr.sa_data, 6);
			info ("find device (%s), mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
				ifr[i].ifr_name, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

			if (!strncmp(ifr[i].ifr_name, "eth", sizeof("eth")-1)) {
				memset (mac_str, 0, 20);
				sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
							mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				close (sock);
				return 0;
			}
		}
	}
	close (sock);
	return -1;
}

//------------------------------------------------------------------------------
char *remove_space_str (char *str)
{
	int len = strlen(str);

	while ((*str++ == 0x20) && len--)

	return	str -1;
}

//------------------------------------------------------------------------------
char *tolowerstr (char *str)
{
	int i, len = strlen(str);

	while ((*str++ == 0x20) && len--);

	for (i = 0; i < len; i++, str++)
		*str = tolower(*str);

	/* calculate input string(*str) start pointer */
	return	(str -len -1);
}

//------------------------------------------------------------------------------
char *toupperstr (char *str)
{
	int i, len = strlen(str);

	while ((*str++ == 0x20) && len--);

	for (i = 0; i < len; i++, str++)
		*str = toupper(*str);

	/* calculate input string(*str) start pointer */
	return	(str -len -1);
}

//------------------------------------------------------------------------------
int fread_int (char *filename)
{
	__s8	rdata[16];
	FILE 	*fp;

	if (access (filename, R_OK) != 0)
		return -1;

	// adc raw value get
	if ((fp = fopen(filename, "r")) != NULL) {
		fgets (rdata, sizeof(rdata), fp);
		fclose(fp);
	}
	
	return	(atoi(rdata));
}

//------------------------------------------------------------------------------
int fread_line (char *filename, char *line, int l_size)
{
	FILE 	*fp;

	if (access (filename, R_OK) != 0)
		return -1;

	// adc raw value get
	if ((fp = fopen(filename, "r")) != NULL) {
		fgets (line, l_size, fp);
		fclose(fp);
	}
	info ("filename = %s, rdata = %s\n", filename, line);
	
	return	((strlen (line) > 0) ? 0 : -1);
}

//------------------------------------------------------------------------------
int fwrite_bool (char *filename, char status)
{
	__s8	rdata;
	FILE 	*fp;

	// fan control set
	if ((fp = fopen(filename, "w")) != NULL) {
		fputc (status ? '1' : '0', fp);
		fclose(fp);
	}
	
	// fan control get
	if ((fp = fopen(filename, "r")) != NULL) {
		rdata = fgetc (fp);
		fclose(fp);
	}

	info ("rdata = %c, fan.enable = %c\n", rdata, status + '0');
	
	return	(rdata != (status + '0')) ? false : true;
}

//------------------------------------------------------------------------------
int fwrite_str (char *filename, char *wstr)
{
	__s8	rdata[128];
	FILE 	*fp;

	// fan control set
	if ((fp = fopen(filename, "w")) != NULL) {
		fputs (wstr, fp);
		fclose(fp);
	}
	
	// fan control get
	if ((fp = fopen(filename, "r")) != NULL) {
		fgets (rdata, sizeof(rdata), fp);
		fclose(fp);
	}

	info ("rdata = %s, wstr = %s\n", rdata, wstr);
	return	(strncmp (wstr, rdata, strlen(wstr)) != 0) ? true : false;
}

//------------------------------------------------------------------------------
#define CONFIG_APP_FILE     "app.cfg"

int find_appcfg_data (char *fkey, char *fdata)
{
	FILE *fp;
	char read_line[128], *ptr;
	bool appcfg = false, multiline = false;
	int cmd_cnt = 0, pos = 0;;
	if (access (CONFIG_APP_FILE, R_OK) == 0) {
		if ((fp = fopen (CONFIG_APP_FILE, "r")) != NULL) {
			memset (read_line, 0x00, sizeof(read_line));
			if (!strncmp ("SERVER_CMD", fkey, sizeof("SERVER_CMD")-1))
				multiline = true;
			if (!strncmp ("POWER_PIN", fkey, sizeof("POWER_PIN")-1))
				multiline = true;

			while (fgets(read_line, sizeof(read_line), fp) != NULL) {

				if (!appcfg) {
					appcfg = strncmp ("ODROID-APP-CONFIG", read_line,
									strlen(read_line)-1) == 0 ? true : false;
					memset (read_line, 0x00, sizeof(read_line));
					continue;
				}
				if (read_line[0] != '#') {
					if ((ptr = strstr (read_line, fkey)) != NULL) {
						ptr = strstr (ptr +1, "=");
						ptr = ptr +1;
						while ((ptr != NULL) && (*ptr == ' '))	ptr++;

						pos = cmd_cnt * sizeof(read_line);
						strncpy(&fdata[pos], ptr, strlen (ptr) -1);

						if (multiline)	cmd_cnt++;
						else {
							fclose (fp);
							return 0;
						}
						//info ("%s : (%d) %s\n", __func__, (int)strlen(fdata), fdata);
					}
				}
				memset (read_line, 0x00, sizeof(read_line));
			}
			fclose (fp);
		}
	}
	return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
