#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h> //close
#include <stdbool.h>
#include <signal.h>

#include "client.h"
#include "messages.h"
#include "communication.h"
#include "helpers.h"
#include "commands.h"

#define PRINT_LINE() printf("%d\n", __LINE__) //REMOVE for testing only

int inet_pton(int af, const char *restrict src, void *restrict dst);

int kill(pid_t pid, int sig);

const int stdin_fd = 0;

const unsigned int MAX_EVENTS = 3;
const unsigned int RES_BUFF_SIZE = 128;
const unsigned int STDIN_BUFF_SIZE = 1402;

char local_display_name[DISPLAY_NAME_MAX_LEN];

CommunicationState current_state = S_START;

int socket_fd;
struct sockaddr_in address;

CmdArguments *arguments;
    
void sigint_handler(int sig_no)
{
    (void)sig_no; // odchytava se pouze SIGINT, tak aby se odstranilo varovani pri prekladu
    if (arguments->is_udp) {
        Command bye_command = {
            .command_type = CMD_EXIT
        };


        char *message_string;
        int msg_size;
        create_msg_string_from_command(&bye_command, &message_string, &msg_size);

        if (send_message_from_command(&bye_command, socket_fd, &address) == 1) {
            free_command(&bye_command);
            exit(99); //TODO
        }

        // while (true) posilat bye dokud nedojde recvfrom confirm
    }
    else {
        //TODO: send bye 
        printf("using tcp\n");
    }

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        print_help();
        return 0;
    }

    arguments = (CmdArguments *)malloc(sizeof(CmdArguments));
    if (arguments == NULL)
        fprintf(stderr, "ERR: Internal error\n");

    struct in_addr *ip_address;

    int ret_value;
    
    ret_value = parse_arguments(argc, argv, arguments);
    if (ret_value == 1) exit(99); //TODO

    if (get_ip_address(arguments->server_ip_or_hostname, &ip_address) == 1) exit(99); //TODO: handle correctly
    
    signal(SIGINT, sigint_handler);

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
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

    address.sin_family = AF_INET;
    address.sin_port = htons(arguments->server_port);
    inet_pton(AF_INET, arguments->server_ip_or_hostname, &address.sin_addr.s_addr);

    unsigned int addr_len;

    while (true) {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) exit(99); //TODO

        if (events[0].data.fd == stdin_fd) {
            char line[STDIN_BUFF_SIZE];
            fgets(line, STDIN_BUFF_SIZE, stdin); // fgets uklada i \n
            line[strcspn(line, "\n")] = '\0';

            if (is_empty(line) == true)
                continue;

            CommandType cmd_type = get_command_type(line);

            if (cmd_type == CMD_HELP) {
                print_command_help();
                continue;
            } else if (cmd_type == CMD_RENAME) {
                //Locally changes the display name of the user to be sent with new messages/selected commands
                char new_name[DISPLAY_NAME_MAX_LEN];
                if (sscanf(line, "/rename %20s", new_name) != 1) {
                    continue;
                }
                strcpy(local_display_name, new_name);
                continue;
            }

            Command command;
            if (parse_command(line, cmd_type, &command, local_display_name) == 1) // spatny command, cekej dal
                continue;

            if (send_message_from_command(&command, socket_fd, &address) == 1) {
                free_command(&command);
                return 1;//TODO
            }
            
            free_command(&command);

            printf("stdin event happened first: '%s'\n", line);
        } else {
            char response[RES_BUFF_SIZE];
            //predat parametr strukturu s sockaddr
            ret_value = recvfrom(socket_fd, response, RES_BUFF_SIZE, 0, (struct sockaddr *)&address, &addr_len);
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