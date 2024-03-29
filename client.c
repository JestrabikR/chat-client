#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "client.h"
#include "messages.h"
#include "communication.h"
#include "udp/udp.h"
#include "helpers.h"

#define PRINT_LINE() printf("%d\n", __LINE__) //REMOVE for testing only

const int stdin_fd = 0;

const unsigned int MAX_EVENTS = 1;
const unsigned int RES_BUFF_SIZE = 128;
const unsigned int STDIN_BUFF_SIZE = 128;

//TODO: kde vezmu username, display name a secret?
const char *USERNAME ="xjestr04";
const char *DISPLAY_NAME = "xjestr04";

int main(int argc, char *argv[]) {

    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        print_help();
        return 0;
    }

    struct in_addr *ip_address;

    int ret_value;

    CmdArguments *arguments = (CmdArguments *)malloc(sizeof(CmdArguments));
    
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

    char *msg = "this is a message from client";

    //TODO: proc to pise connected i kdyz ten server nebezi?
    ret_value = udp_connect(socket_fd,
                            inet_ntoa(*ip_address),
                            arguments->server_port);
    if (ret_value == 1) return 1; //TODO

    AuthMessage auth_message = {
        .msg_type = AUTH,
        .message_id = get_message_id(),
    };
    strcpy(auth_message.username, USERNAME);
    strcpy(auth_message.display_name, DISPLAY_NAME);
    strcpy(auth_message.secret, "TODO");
    
    ssize_t result = send(socket_fd, msg, strlen(msg), 0);
    if (result == -1) return 1; //TODO

    // int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    // if (event_count == -1) exit(99); //TODO

    // if (events[0].data.fd == stdin_fd) {
    //     char line[STDIN_BUFF_SIZE];
    //     fgets(line, STDIN_BUFF_SIZE, stdin);
    //     line[strcspn(line, "\n")] = '\0';
    //     printf("stdin event happened first: '%s'\n", line);
    // } else {
    //     char response[RES_BUFF_SIZE];
    //     ret_value = recv(socket_fd, response, RES_BUFF_SIZE, 0);
    //     //TODO: na konci response jsou spatne znaky
    //     if (ret_value == -1) exit(99); //TODO
    //     if ((unsigned int)ret_value > RES_BUFF_SIZE) exit(1); //TODO: asi neni potreba exit ne?
    //     printf("socket event happened first: '%s'\n", response);
    // }

    free(arguments);
    int epoll_closed = close(epoll_fd);
    int socket_closed = close(epoll_fd);
    if (epoll_closed != 0 || socket_closed != 0) exit(99); //TODO

    return 0;
}