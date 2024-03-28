#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>

int get_ip_address(char *hostname, struct in_addr **ip_address);

int udp_connect(int socket_fd, const char *ip, int port);

#endif