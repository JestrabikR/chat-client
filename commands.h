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

/**
 * @brief returns type of
*/
CommandType get_command_type(char *command_str);

/**
 * @brief parses auth command string and sets auth_message
 * @returns 0 if successful, 1 in case of error
*/
int parse_auth_command(char *command_str, AuthMessage *auth_msg);

#endif