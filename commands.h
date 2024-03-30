#ifndef COMMANDS_H
#define COMMANDS_H

#include "messages.h"

typedef enum {
    CMD_AUTH,
    CMD_JOIN,
    CMD_RENAME,
    CMD_HELP,
    CMD_MESSAGE,
    CMD_UNKNOWN
} CommandType;

typedef struct {
    CommandType command_type;
    union {
        JoinMessage join_message;
        AuthMessage auth_message;
        Message message;
    };
} Command;

/**
 * @brief returns type of command
*/
CommandType get_command_type(char *command_str);

/**
 * @brief returns correct command in parameter
 * @returns 0 on success, 1 on error
*/
int parse_command(char *line, CommandType cmd_type, Command *command);

int send_message_from_command(Command *command, int socket_fd);

#endif