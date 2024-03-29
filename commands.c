#include <stdio.h>
#include <ctype.h>
#include "commands.h"
#include "string.h"
#include "helpers.h"

//TODO: pred zavolanim se musi zkontrolovat jestli to neni prazdny a radek a kdyztak ho asi ignorovat
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
    } else if (strncmp("/help ", command_str, 6) == 0) {
        return CMD_HELP;
    } 

    return CMD_UNKNOWN;
}

int parse_auth_command(char *command_str, AuthMessage *auth_msg) {
    char username[USERNAME_MAX_LEN];
    char display_name[DISPLAY_NAME_MAX_LEN];
    char secret[SECRET_MAX_LEN];

    int read_values = sscanf(command_str, "/auth %s %s %s", username, display_name, secret);
    if (read_values != 3) {
        return 1;
    }

    // alokace pameti pro ukladani retezcu
    auth_msg->username = malloc(strlen(username) + 1); // +1 pro nulovy ukoncovaci znak
    auth_msg->display_name = malloc(strlen(display_name) + 1);
    auth_msg->secret = malloc(strlen(secret) + 1);

    if (auth_msg->username == NULL || auth_msg->display_name == NULL || auth_msg->secret == NULL) {
        free(auth_msg->username);
        free(auth_msg->display_name);
        free(auth_msg->secret);
        return 1;
    }

    strcpy(auth_msg->username, username);
    strcpy(auth_msg->display_name, display_name);
    strcpy(auth_msg->secret, secret);

    return 0;
}
