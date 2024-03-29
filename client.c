#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h> //close
#include <stdbool.h>

#include "client.h"
#include "messages.h"
#include "communication.h"
#include "udp/udp.h"
#include "helpers.h"
#include "commands.h"

#define PRINT_LINE() printf("%d\n", __LINE__) //REMOVE for testing only

const int stdin_fd = 0;

const unsigned int MAX_EVENTS = 3;
const unsigned int RES_BUFF_SIZE = 128;
const unsigned int STDIN_BUFF_SIZE = 1402;

uint16_t message_id = 0;

int main(int argc, char *argv[]) {

    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        print_help();
        return 0;
    }

    CmdArguments *arguments = (CmdArguments *)malloc(sizeof(CmdArguments));
    if (arguments == NULL)
        fprintf(stderr, "ERR: Internal error\n");

    struct in_addr *ip_address;

    int ret_value;
    
    ret_value = parse_arguments(argc, argv, arguments);
    if (ret_value == 1) exit(99); //TODO

    if (get_ip_address(arguments->server_ip_or_hostname, &ip_address) == 1) exit(99); //TODO: handle correctly

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1) exit(99); //TODO: handle

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) exit(99); //TODO: handle

    struct epoll_event socket_event;
    struct epoll_event stdin_event;

    socket_event.events = EPOLLIN;
    stdin_event.events = EPOLLIN;

    socket_event.data.fd = socket_fd;
    stdin_event.data.fd = stdin_fd;

    ret_value = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &socket_event);
    if (ret_value == -1) exit(99); //TODO
    ret_value = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, stdin_fd, &stdin_event);
    if (ret_value == -1) exit(99); //TODO

    struct epoll_event events[MAX_EVENTS];

    //TODO: proc to pise connected i kdyz ten server nebezi?
    ret_value = udp_connect(socket_fd,
                            inet_ntoa(*ip_address),
                            arguments->server_port);
    if (ret_value == 1) return 1; //TODO

    
    //TODO: strncpy jenom strlen(username) napr

    //TODO: po kazde zprave se musi zvysit asi rucne message_id

    //printf("%d\n", sizeof(auth_message));

    char val[10];

    while (true) {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) exit(99); //TODO

        if (events[0].data.fd == stdin_fd) {
            char line[STDIN_BUFF_SIZE];
            fgets(line, STDIN_BUFF_SIZE, stdin); // fgets uklada i \n
            line[strcspn(line, "\n")] = '\0';

            if (is_empty(line) == true)
                continue;

            //TODO: mohlo by jit predelat do struktury s unii a typem, ktera se vrati z funkce at to neni tady
            CommandType cmd_type = get_command_type(line);
            printf("%d\n", cmd_type);

            switch (cmd_type) {
                case CMD_AUTH:
                    AuthMessage auth_message;
                    //TODO: pozor tady se alokuji stringy ve strukture je potreba free
                    if (parse_auth_command(line, &auth_message) == 1) {
                        fprintf(stderr, "ERR: Wrong command format\n");
                        continue;
                    }
                    printf("OK");
                    auth_message.msg_type = MT_AUTH;
                    auth_message.message_id = message_id++;
                    ssize_t result = send(socket_fd, &auth_message, sizeof(auth_message), 0);
                    if (result == -1) return 1; //TODO
                    break;
                case CMD_JOIN:
                    break;
                case CMD_RENAME:
                    break;
                case CMD_MESSAGE:
                    break;
                case CMD_HELP:
                    break;
                default:
                    fprintf(stderr, "ERR: Unknown command\n");
                    continue;
            }

            printf("stdin event happened first: '%s'\n", line);
        } else {
            char response[RES_BUFF_SIZE];
            ret_value = recv(socket_fd, response, RES_BUFF_SIZE, 0);
            //TODO: na konci response jsou spatne znaky 
            if (ret_value == -1) exit(99); //TODO
            if ((unsigned int)ret_value > RES_BUFF_SIZE) exit(1); //TODO: asi neni potreba exit ne?
            printf("socket event happened first: '%s'\n", response);
        }
    }

    free(arguments);
    int epoll_closed = close(epoll_fd);
    int socket_closed = close(epoll_fd);
    if (epoll_closed != 0 || socket_closed != 0) exit(99); //TODO

    return 0;
}