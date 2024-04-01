#ifndef MAIN_H
#define MAIN_H

#include <netinet/in.h>

void clean_up();

/**
 * Sends ERR message and sets state to S_ERROR
*/
void handle_error();

/**
 * @returns 0 on success, 1 in case of error
*/
int wait_for_confirm();

/**
 * @returns 0 on success, 1 in case of error
*/
int udp_send_bye_wait_for_confirm();

/**
 * @returns 0 on success, 1 in case of error
*/
int udp_send_error_wait_for_confirm(char *message_content);

int udp_send_confirm(uint16_t message_id);

/**
 * @brief creates and setups socket, epoll_fd, stdin/socket events
 * @returns 0 on success, 1 in case of error
*/
int setup_udp();

int setup_tcp();

int setup_epoll();

#endif