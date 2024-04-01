#include <stdio.h>
#include <ctype.h>
#include "commands.h"
#include "string.h"
#include "helpers.h"
#include "sent_messages_queue.h"

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
int parse_auth_command(char *command_str, AuthMessage *auth_msg, char *disp_name) {
    char username[USERNAME_MAX_LEN + 1]; // +1 pro \0
    char display_name[DISPLAY_NAME_MAX_LEN + 1];
    char secret[SECRET_MAX_LEN + 1];

    int read_values = sscanf(command_str, "/auth %20s %128s %20s", username, secret, display_name);
    if (read_values != 3) {
        return 1;
    }

    // Nastaví hodnotu display name
    strcpy(disp_name, display_name);

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

int parse_join_command(char *command_str, JoinMessage *join_msg, char *disp_name) {
    char channel_id[CHANNEL_ID_MAX_LEN + 1];

    //TODO: sscanf nepozna /join server a /join server server2 (oboje projde ale to druhe by nemelo)
    if (sscanf(command_str, "/join %20s", channel_id) != 1) {
        return 1;
    }

    join_msg->display_name = malloc(strlen(disp_name) + 1);
    join_msg->channel_id = malloc(strlen(channel_id) + 1);

    if (join_msg->display_name == NULL || join_msg->channel_id == NULL) {
        free(join_msg->display_name);
        free(join_msg->channel_id);
        return 1; //TODO: asi jinak ukoncit
    }

    strncpy(join_msg->channel_id, channel_id, strlen(channel_id) + 1);
    strncpy(join_msg->display_name, disp_name, strlen(disp_name) + 1);

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

int parse_command(char *line, CommandType cmd_type, Command *command, char *disp_name) {
    command->command_type = cmd_type;

    switch (cmd_type) {
        case CMD_AUTH:
            // POZOR tady se alokuje username, secret a display name - free po odeslani
            if (parse_auth_command(line, &command->auth_message, disp_name) == 1) {
                fprintf(stderr, "ERR: Wrong command format\n");
                return 1;
            }
            command->auth_message.msg_type = MT_AUTH;
            break;

        case CMD_JOIN:
            if (parse_join_command(line, &command->join_message, disp_name) == 1) {
                fprintf(stderr, "ERR: Wrong command format\n");
                return 1;
            }
            command->join_message.msg_type = MT_JOIN;
            break;

        case CMD_MESSAGE:
            if (parse_message_command(line, &command->message, disp_name) == 1) {
                fprintf(stderr, "ERR: Wrong message\n");
                return 1;
            }
            command->message.msg_type = MT_MSG;
            break;
        case CMD_EXIT:
            break;
        default:
            fprintf(stderr, "ERR: Unknown command\n");
            return 1;
    }

    return 0;
}

int create_msg_string_from_command(Command *command, char **message_string, int *msg_size) {
    size_t message_size;
    switch (command->command_type) {
        case CMD_AUTH: {
            AuthMessage msg = command->auth_message;
            message_size = sizeof(msg.message_id) +
                sizeof(msg.msg_type) +
                strlen(msg.display_name) + 1 +
                strlen(msg.username) + 1 +
                strlen(msg.secret) + 1;
            
            *message_string = (char *)malloc(message_size);         
            if (*message_string == NULL)
                return 1; //TODO: asi jinak ukoncit at poznam ze je to malloc fail

            // Kopírování hodnot z msg do pole bytů
            char *ptr = *message_string;

            // Kopírování msg_type (1B)
            memcpy(ptr, &(msg.msg_type), sizeof(msg.msg_type));
            ptr += sizeof(msg.msg_type);

            // Kopírování message_id (2B)
            uint16_t message_id_reversed = htons(msg.message_id);
            memcpy(ptr, &message_id_reversed, sizeof(message_id_reversed));
            ptr += sizeof(message_id_reversed);

            // Kopírování username (nB)
            strcpy(ptr, msg.username);
            ptr += strlen(msg.username) + 1;

            // Kopírování display_name (nB)
            strcpy(ptr, msg.display_name);
            ptr += strlen(msg.display_name) + 1;

            // Kopírování secret (nB)
            strcpy(ptr, msg.secret);
            ptr += strlen(msg.secret) + 1;

            *msg_size = message_size;

            break;
        }

        case CMD_JOIN: {
            JoinMessage msg = command->join_message;
            message_size = sizeof(msg.message_id) +
                sizeof(msg.msg_type) +
                strlen(msg.display_name) + 1 +
                strlen(msg.channel_id) + 1;

            *message_string = (char *)malloc(message_size);         
            if (*message_string == NULL)
                return 1; //TODO: asi jinak ukoncit at poznam ze je to malloc fail

            // Kopírování hodnot z msg do pole bytů
            char *ptr = *message_string;

            // Kopírování msg_type (1B)
            memcpy(ptr, &(msg.msg_type), sizeof(msg.msg_type));
            ptr += sizeof(msg.msg_type);

            // Kopírování message_id (2B)
            uint16_t message_id_reversed = htons(msg.message_id);
            memcpy(ptr, &message_id_reversed, sizeof(message_id_reversed));
            ptr += sizeof(message_id_reversed);

            // Kopírování channel_id (nB)
            strcpy(ptr, msg.channel_id);
            ptr += strlen(msg.channel_id) + 1;

            // Kopírování display_name (nB)
            strcpy(ptr, msg.display_name);
            ptr += strlen(msg.display_name) + 1;

            *msg_size = message_size;

            break;
        }

        case CMD_MESSAGE: {
            Message msg = command->message;
            message_size = sizeof(msg.message_id) +
                sizeof(msg.msg_type) +
                strlen(msg.display_name) + 1 +
                strlen(msg.message_content) + 1;

            *message_string = (char *)malloc(message_size);         
            if (*message_string == NULL)
                return 1; //TODO: asi jinak ukoncit at poznam ze je to malloc fail

            // Kopírování hodnot z msg do pole bytů
            char *ptr = *message_string;

            // Kopírování msg_type (1B)
            memcpy(ptr, &(msg.msg_type), sizeof(msg.msg_type));
            ptr += sizeof(msg.msg_type);

            // Kopírování message_id (2B)
            uint16_t message_id_reversed = htons(msg.message_id);
            memcpy(ptr, &message_id_reversed, sizeof(message_id_reversed));
            ptr += sizeof(message_id_reversed);

            // Kopírování display_name (nB)
            strcpy(ptr, msg.display_name);
            ptr += strlen(msg.display_name) + 1;

            // Kopírování channel_id (nB)
            strcpy(ptr, msg.message_content);
            ptr += strlen(msg.message_content) + 1;

            *msg_size = message_size;

            break;
        }
        case CMD_EXIT: {
            ByeMessage msg = command->bye_message;
            message_size = sizeof(msg.message_id) +
                sizeof(msg.msg_type);

            *message_string = (char *)malloc(message_size);         
            if (*message_string == NULL)
                return 1; //TODO: asi jinak ukoncit at poznam ze je to malloc fail

            // Kopírování hodnot z msg do pole bytů
            char *ptr = *message_string;

            // Kopírování msg_type (1B)
            memcpy(ptr, &(msg.msg_type), sizeof(msg.msg_type));
            ptr += sizeof(msg.msg_type);

            // Kopírování message_id (2B)
            uint16_t message_id_reversed = htons(msg.message_id);
            memcpy(ptr, &message_id_reversed, sizeof(message_id_reversed));
            ptr += sizeof(message_id_reversed);

            *msg_size = message_size;

            break;
        }

        default:
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
        case CMD_EXIT:
            break;
        default:
            break;
    }
}

uint16_t message_id = 0;

int send_message_from_command(Command *command, int socket_fd, struct sockaddr_in *socket_address, SM_Queue *sm_queue) {
    
    sm_queue_enqueue(sm_queue, message_id);
    
    ssize_t result;
    switch (command->command_type) {
        case CMD_AUTH:
            command->auth_message.message_id = message_id++;
            break;
        case CMD_JOIN:
            command->join_message.message_id = message_id++;

            break;
        case CMD_MESSAGE:
            command->message.message_id = message_id++;
            break;
        case CMD_EXIT:
            command->bye_message.msg_type = MT_BYE;
            command->bye_message.message_id = message_id++;
            break;
        default:
            break;
    }

    char *message_string;
    int msg_size;
    create_msg_string_from_command(command, &message_string, &msg_size);

    result = sendto(socket_fd,
                     message_string,
                     msg_size, 0,
                     (struct sockaddr *)socket_address,
                     sizeof(*socket_address));
    if (result == -1) {
        free(message_string);
        return 1;
    }

    free(message_string);

    return 0;
}