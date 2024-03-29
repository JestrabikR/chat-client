#ifndef UDP_H
#define UDP_H

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int udp_connect(int socket_fd, const char *ip, int port);

#endif