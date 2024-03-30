#include <stdio.h>
#include <ctype.h>
#include "commands.h"
#include "string.h"
#include "helpers.h"
#include <sys/types.h>

// pred zavolanim se musi zkontrolovat jestli to neni prazdny a radek a kdyztak ho asi ignorovat
CommandType get_command_type(char *command_str) {
    command_str = trim(command_str); // updates pointer to first non-whitespace char

    if (command_str[0] != '/') { // if is message
        return CMD_MESSAGE;
    }

    //POZOR: musi se porovnat i s mezerou za prikazem
    if (strncmp("/auth ", command_str, 6) == 0) {
        return CMD_AUTH;
    } else if (strncmp("/join ", command_str, 6) == 0) {
        return CMD_JOIN;
    } else if (strncmp("/rename ", command_str, 8) == 0) {
        return CMD_RENAME;
    } else if (strncmp("/help", command_str, 5) == 0) {
        return CMD_HELP;
    } 

    return CMD_UNKNOWN;
}

/**
 * @brief parses auth command string and sets auth_message
 * @returns 0 if successful, 1 in case of error
*/
int parse_auth_command(char *command_str, AuthMessage *auth_msg) {
    char username[USERNAME_MAX_LEN + 1]; // +1 pro \0
    char display_name[DISPLAY_NAME_MAX_LEN + 1];
    char secret[SECRET_MAX_LEN + 1];

    int read_values = sscanf(command_str, "/auth %20s %128s %20s", username, secret, display_name);
    if (read_values != 3) {
        return 1;
    }

    // Alokace paměti pro řetězce
    auth_msg->username = malloc(strlen(username) + 1);
    auth_msg->display_name = malloc(strlen(display_name) + 1);
    auth_msg->secret = malloc(strlen(secret) + 1);

    if (auth_msg->username == NULL || auth_msg->display_name == NULL || auth_msg->secret == NULL) {
        // Uvolnění paměti v případě chyby
        free(auth_msg->username);
        free(auth_msg->display_name);
        free(auth_msg->secret);
        return 1; //TODO: asi jinak ukoncit
    }

    // Kopírování řetězců do alokované paměti
    strncpy(auth_msg->username, username, strlen(username) + 1);
    strncpy(auth_msg->display_name, display_name, strlen(display_name) + 1);
    strncpy(auth_msg->secret, secret, strlen(secret) + 1);

    return 0;
}

int parse_join_command(char *command_str, JoinMessage *join_msg, char *display_name) {
    char channel_id[CHANNEL_ID_MAX_LEN + 1];

    //TODO: sscanf nepozna /join server a /join server server2 (oboje projde ale to druhe by nemelo)
    if (sscanf(command_str, "/join %20s", channel_id) != 1) {
        return 1;
    }

    join_msg->display_name = malloc(strlen(display_name) + 1);
    join_msg->channel_id = malloc(strlen(channel_id) + 1);

    if (join_msg->display_name == NULL || join_msg->channel_id == NULL) {
        free(join_msg->display_name);
        free(join_msg->channel_id);
        return 1; //TODO: asi jinak ukoncit
    }

    strncpy(join_msg->channel_id, channel_id, strlen(channel_id) + 1);
    strncpy(join_msg->display_name, display_name, strlen(display_name) + 1);

    return 0;
}

int parse_message_command(char *command_str, Message *msg, char *display_name) {
    msg->message_content = malloc(strlen(command_str) + 1);
    msg->display_name = malloc(strlen(display_name) + 1);
    
    if (msg->message_content == NULL || msg->display_name == NULL) {
        free(msg->message_content);
        free(msg->display_name);
        return 1; //TODO: asi jinak ukoncit
    }

    strncpy(msg->message_content, command_str, strlen(command_str) + 1);
    strncpy(msg->display_name, display_name, strlen(display_name) + 1);
    return 0;
}


int parse_command(char *line, CommandType cmd_type, Command *command, char *display_name) {
    command->command_type = cmd_type;

    switch (cmd_type) {
        case CMD_AUTH:
            //TODO: POZOR tady se alokuje username, secret a display name - free po odeslani
            if (parse_auth_command(line, &command->auth_message) == 1) {
                fprintf(stderr, "ERR: Wrong command format\n");
                return 1;
            }
            command->auth_message.msg_type = MT_AUTH;
            printf("PARSED AUTH\n");
            break;

        case CMD_JOIN:
            if (parse_join_command(line, &command->join_message, display_name) == 1) {
                fprintf(stderr, "ERR: Wrong command format\n");
                return 1;
            }
            command->join_message.msg_type = MT_JOIN;
            printf("PARSED JOIN\n");
            break;

        case CMD_MESSAGE:
            printf("PARSING MESSAGE\n");
            if (parse_message_command(line, &command->message, display_name) == 1) {
                fprintf(stderr, "ERR: Wrong message\n");
                return 1;
            }
            command->message.msg_type = MT_MSG;
            break;

        default:
            fprintf(stderr, "ERR: Unknown command\n");
            return 1;
    }

    return 0;
}

void free_command(Command *command) {
    switch (command->command_type) {
        case CMD_AUTH:
            free(command->auth_message.username);
            free(command->auth_message.display_name);               
            free(command->auth_message.secret);               
            break;

        case CMD_JOIN:
            free(command->join_message.channel_id);
            free(command->join_message.display_name);  
            break;

        case CMD_MESSAGE:
            free(command->message.message_content);
            free(command->message.display_name);  
            break;

        default:
            fprintf(stderr, "ERR: Unknown command\n");
            return 1;
    }

    return 0;
}

uint16_t message_id = 0;

int send_message_from_command(Command *command, int socket_fd) {
    printf("t: %d\n", command->command_type);
    ssize_t result;
    switch (command->command_type) {
        case CMD_AUTH:
            command->auth_message.message_id = message_id++;
            result = send(socket_fd, &command->auth_message, sizeof(command->auth_message), 0);
            if (result == -1) return 1;
            printf("SENT AUTH\n");
            break;
        case CMD_JOIN:
            printf("STARTED SENDING JOIN\n");
            command->join_message.message_id = message_id++;
            result = send(socket_fd, &command->join_message, sizeof(command->join_message), 0);
            if (result == -1) return 1;
            printf("SENT JOIN\n");
            break;
        case CMD_MESSAGE:
            printf("SENDING MESSAGE\n");
            command->message.message_id = message_id++;
            result = send(socket_fd, &command->message, sizeof(command->message), 0);
            if (result == -1) return 1;
            printf("SENT MESSAGE\n");
            break;
        default:
            break;
    }

    return 0;
}