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
int parse_command(char *line, CommandType cmd_type, Command *command, char *disp_name);

int send_message_from_command(Command *command, int socket_fd);

/**
 * @return 0 on success, 1 if wrong command is passed
*/
int free_command(Command *command);

/**
 * @brief allocates and arranges message_string to send
 * @returns 0 on success, 1 on error
*/
int create_msg_string_from_command(Command *command, char **message_string, int *msg_size);

#endif