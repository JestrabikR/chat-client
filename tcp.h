#ifndef TCP_H
#define TCP_H

#include "commands.h"

int tcp_send_message_from_command(Command *command, int socket_fd);

/**
 * @brief allocates and arranges message_string to send, length of message_string can be obtained by strlen()
 * @returns 0 on success, 1 on error
*/
int tcp_create_msg_string_from_command(Command *command, char **message_string);

void tcp_send_bye(int socket_fd);

int tcp_send_error(int socket_fd, char *message_content, char *display_name);

#endif