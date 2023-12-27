

#ifndef _IF_ADDRS_H__

#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CURRENT_WLAN_NAME	"wlan0"
#define CURRENT_ETH_NAME	"eth0"

__BEGIN_DECLS
int get_localip(const char * eth_name, char *local_ip_addr);
__END_DECLS
#endif//_IF_ADDRS_H__
