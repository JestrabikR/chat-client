#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>

void udp_send_bye_wait_for_confirm();

/**
 * @brief creates and setups socket, epoll_fd, stdin/socket events
 * returns 0 on success, 1 in case of error
*/
int setup_udp();

#endif