#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>

/**
 * @brief creates and setups socket, epoll_fd, stdin/socket events
 * returns 0 on success, 1 in case of error
*/
int setup_udp();

#endif