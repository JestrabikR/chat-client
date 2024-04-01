#include "tcp.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int tcp_create_msg_string_from_command(Command *command, char **message_string) {
    size_t message_size;
    switch (command->command_type) {
        case CMD_AUTH: {
            //AUTH {Username} AS {DisplayName} USING {Secret}\r\n
            AuthMessage msg = command->auth_message;
            message_size = strlen("AUTH ") + strlen(msg.username) +
                           strlen(" AS ") + strlen(msg.display_name) +
                           strlen(" USING ") + strlen(msg.secret) +
                           strlen("\r\n") + 1; // +1 for terminating character

            *message_string = (char *)malloc(message_size);
            if (*message_string == NULL) return 1;

            snprintf(*message_string, message_size, "AUTH %s AS %s USING %s\r\n",
                     msg.username, msg.display_name, msg.secret);
            break;
        }

        case CMD_JOIN: {
            //JOIN {ChannelID} AS {DisplayName}\r\n
            JoinMessage msg = command->join_message;
            message_size = strlen("JOIN ") + strlen(msg.channel_id) +
                           strlen(" AS ") + strlen(msg.display_name) +
                           strlen("\r\n") + 1; // +1 for terminating character

            *message_string = (char *)malloc(message_size);
            if (*message_string == NULL) return 1; 

            snprintf(*message_string, message_size, "JOIN %s AS %s\r\n",
                     msg.channel_id, msg.display_name);
            break;
        }

        case CMD_MESSAGE: {
            //MSG FROM {DisplayName} IS {MessageContent}\r\n
            Message msg = command->message;
            message_size = strlen("MSG FROM ") + strlen(msg.display_name) +
                           strlen(" IS ") + strlen(msg.message_content) +
                           strlen("\r\n") + 1; // +1 for terminating character

            *message_string = (char *)malloc(message_size);
            if (*message_string == NULL) return 1; 

            snprintf(*message_string, message_size, "MSG FROM %s IS %s\r\n",
                     msg.display_name, msg.message_content);
            break;
        }

        case CMD_EXIT: {
            //BYE\r\n
            *message_string = (char *)malloc(strlen("BYE\r\n") + 1); // +1 for terminating character
            if (*message_string == NULL) return 1; 

            strcpy(*message_string, "BYE\r\n");
            break;
        }
        default:
            return 1;
    }
    return 0;
}

int tcp_send_message_from_command(Command *command, int socket_fd) {
    char *message_string;
    int result = tcp_create_msg_string_from_command(command, &message_string);

    result = send(socket_fd, message_string, strlen(message_string) + 1, 0);
    if (result == -1) {
        free(message_string);
        return 1;
    }

    free(message_string);

    return 0;
}

void tcp_send_bye(int socket_fd) {
    Command bye_command = {
            .command_type = CMD_EXIT
        };

    tcp_send_message_from_command(&bye_command, socket_fd);
}

int tcp_send_error(int socket_fd, char *message_content, char *display_name) {
    //ERR FROM {DisplayName} IS {MessageContent}\r\n
    size_t message_size = strlen("ERR FROM ") + strlen(display_name) +
                    strlen(" IS ") + strlen(message_content) +
                    strlen("\r\n") + 1; // +1 for terminating character

    char *message_string = (char *)malloc(message_size);
    if (message_string == NULL) return 1; 

    snprintf(message_string, message_size, "ERR FROM %s IS %s\r\n",
                display_name, message_content);

    int result = send(socket_fd, message_string, strlen(message_string) + 1, 0);
    if (result == -1) {
        free(message_string);
        return 1;
    }

    free(message_string);
    return 0;
}
