#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>

void clean_up();

void wait_for_confirm();

void udp_send_bye_wait_for_confirm();

void udp_send_error_wait_for_confirm(char *message_content);

void udp_send_confirm(uint16_t message_id);

/**
 * @brief creates and setups socket, epoll_fd, stdin/socket events
 * returns 0 on success, 1 in case of error
*/
int setup_udp();

int setup_tcp();

int setup_epoll();

#endif