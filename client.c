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

#define PRINT_LINE() printf("%d\n", __LINE__) //REMOVE for testing only

int getopt(int argc, char * const argv[],
           const char *optstring);

extern char *optarg;
extern int optind, opterr, optopt;

int get_ip_address(char *hostname, struct in_addr **ip_address) {
    struct hostent *host;

    host = gethostbyname(hostname);
    if (host == NULL) {
        return 1;
    }

    *ip_address = (struct in_addr *)host->h_addr_list[0];
    return 0;
}

int udp_connect(int socket_fd, const char *ip, int port) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr.s_addr);
    
    int ret_value = connect(socket_fd, (struct sockaddr *)&address, sizeof(address));
    if (ret_value == -1) return 1;

    printf("connected\n");
    return 0;

}

void print_help() {
    const char *help_string = "-t	User provided	tcp or udp	Transport protocol used for connection\n"
        "-s	User provided	IP address or hostname	Server IP or hostname\n"
        "-p	4567	uint16	Server port\n"
        "-d	250	uint16	UDP confirmation timeout\n"
        "-r	3	uint8	Maximum number of UDP retransmissions\n"
        "-h			Prints program help output and exits\n";
    printf(help_string);
}

const int stdin_fd = 0;

const int PORT = 6969;
const char *IP = "0.0.0.0"; //TODO: for testing purposes only

const unsigned int MAX_EVENTS = 1;
const unsigned int RES_BUFF_SIZE = 128;
const unsigned int STDIN_BUFF_SIZE = 128;

typedef struct {
    bool is_udp;
    char server_ip_or_hostname[256];
    unsigned int server_port;
    unsigned int udp_confirmation_timeout;
    unsigned int max_udp_retransmissions;
} CmdArguments;

/**
 * @brief validates and parses arguments
 * @param arguments fills this structure with data if ended successfully
 * @returns returns 1 in case of error else 0
*/
int parse_arguments(int argc, char *argv[], CmdArguments *arguments) {
    if (argc < 5) { //program_name -t value -s value
        fprintf(stderr, "Wrong number of arguments, run with -h to display help\n");
        return 1;
    }

    // set default values
    arguments->server_port = 4567;
    arguments->udp_confirmation_timeout = 250;
    arguments->max_udp_retransmissions = 3;

    int c;

    //TODO: p,d,r muze user zadat, ale jinak ma defaultni hodnotu?
    while ((c = getopt (argc, argv, "t:s:p:d:r:")) != -1)
        switch (c) {
            case 't':
                arguments->is_udp = strcmp(optarg, "udp") == 0;
                break;
            case 's':
                strcpy(arguments->server_ip_or_hostname, optarg);
                break;
            case 'p':
                arguments->server_port = atoi(optarg);
                break;
            case 'd':
                arguments->udp_confirmation_timeout = atoi(optarg);
                break;
            case 'r':
                arguments->max_udp_retransmissions = atoi(optarg);
                break;
            case 'h':
                print_help();
                break;
            case '?':
                return 1;
            default:
                return 1;
        }

    return 0;
}

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