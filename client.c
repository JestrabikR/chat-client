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
#include "response.h"
#include "sent_messages_queue.h"

#define MAX_EVENTS 3

#define PRINT_LINE() printf("%d\n", __LINE__) //REMOVE for testing only

int inet_pton(int af, const char *restrict src, void *restrict dst);

int kill(pid_t pid, int sig);

const int stdin_fd = 0;

const unsigned int RES_BUFF_SIZE = 1500;
const unsigned int STDIN_BUFF_SIZE = 1402;

char local_display_name[DISPLAY_NAME_MAX_LEN];

CommunicationState current_state = S_START;

int socket_fd;
struct sockaddr_in address;
int epoll_fd;
struct epoll_event events[MAX_EVENTS];
struct epoll_event socket_event;
struct epoll_event stdin_event;

CmdArguments *arguments;

SM_Queue sm_queue;
    
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

        if (send_message_from_command(&bye_command, socket_fd, &address, &sm_queue) == 1) {
            sm_queue_free(&sm_queue);
            free_command(&bye_command);
            exit(99); //TODO
        }
        sm_queue_free(&sm_queue);
        free_command(&bye_command);

        while (true) {
            int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            if (event_count == -1) exit(99); //TODO

            if (events[0].data.fd == stdin_fd) {
                continue; //TODO: asi ignorovat?
            } else {
                char response_msg[RES_BUFF_SIZE];
                unsigned int addr_len;

                int ret_value = recvfrom(socket_fd,
                                    response_msg,
                                    RES_BUFF_SIZE, 0,
                                    (struct sockaddr *)&address,
                                    &addr_len);

                if (ret_value == -1) exit(99); //TODO
                if ((unsigned int)ret_value > RES_BUFF_SIZE) exit(1); //TODO: asi neni potreba exit ne?

                MessageType response_type;
                if (get_response_type(response_msg, &response_type) == 1)
                    exit(99); //TODO spatny typ => asi SEND ERR

                Response response;
                if (parse_response(response_msg, response_type, &response, &sm_queue) == 1) {
                    exit(99); //TODO
                }
                if (response_type == MT_CONFIRM) {
                    break;
                }
            }

        }
    }
    else {
        //TODO: send bye 
        printf("using tcp\n");
    }

    exit(EXIT_SUCCESS);
}

int setup_udp() {
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1)
        return 1;

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        return 1;

    socket_event.events = EPOLLIN;
    stdin_event.events = EPOLLIN;

    socket_event.data.fd = socket_fd;
    stdin_event.data.fd = stdin_fd;

    int ret_value = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &socket_event);
    if (ret_value == -1)
        return 1;
    ret_value = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, stdin_fd, &stdin_event);
    if (ret_value == -1)
        return 1;

    address.sin_family = AF_INET;
    address.sin_port = htons(arguments->server_port);
    inet_pton(AF_INET, arguments->server_ip_or_hostname, &address.sin_addr.s_addr);
    
    return 0;
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

    bool auth_sent = false;
    
    ret_value = parse_arguments(argc, argv, arguments);
    if (ret_value == 1) exit(99); //TODO

    if (get_ip_address(arguments->server_ip_or_hostname, &ip_address) == 1) exit(99); //TODO: handle correctly
    
    if (arguments->is_udp)
        sm_queue_init(&sm_queue);

    signal(SIGINT, sigint_handler);

    ret_value = setup_udp();

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
            } else if (cmd_type == CMD_MESSAGE && current_state != S_OPEN) {
                fprintf(stderr, "ERR: Cannot send message right now (you have to be authenticated)\n");
                continue;
            } else if (cmd_type == CMD_JOIN && current_state != S_OPEN) {
                fprintf(stderr, "ERR: Cannot join channel right now (you have to be authenticated)\n");
                continue;
            }

            Command command;
            if (parse_command(line, cmd_type, &command, local_display_name) == 1) // spatny command, cekej dal
                continue;

            if (send_message_from_command(&command, socket_fd, &address, &sm_queue) == 1) {
                free_command(&command);
                return 1;//TODO
            }
            if (cmd_type == CMD_AUTH) auth_sent = true;
            
            free_command(&command);

            printf("stdin event happened first: '%s'\n", line); //TODO REMOVE
        } else {
            char response_msg[RES_BUFF_SIZE];

            ret_value = recvfrom(socket_fd,
                                response_msg,
                                RES_BUFF_SIZE, 0,
                                (struct sockaddr *)&address,
                                &addr_len);

            if (ret_value == -1) exit(99); //TODO
            if ((unsigned int)ret_value > RES_BUFF_SIZE) exit(1); //TODO: asi neni potreba exit ne?
            printf("socket event happened first: '%s'\n", response_msg); //TODO REMOVE

            MessageType response_type;
            if (get_response_type(response_msg, &response_type) == 1)
                exit(99); //TODO spatny typ => asi SEND ERR

            Response response;
            if (parse_response(response_msg, response_type, &response, &sm_queue) == 1) {
                exit(99); //TODO
            }

            if (response_type == MT_CONFIRM && auth_sent) {
                printf("switched to open\n");
                current_state = S_OPEN;
            } 

            if (response_type == MT_REPLY) {
                //TODO: The contents of an incomming REPLY message are required to be printed to standard error stream (stderr) and formatted as follows (there are two variants of the reply message):
                // Success: {MessageContent}\n
                // Failure: {ReaMessageContenton}\n

            }

        }
    }

    //TODO: vytvorit nejakou clean_up() funkci
    free(arguments);
    sm_queue_free(&sm_queue);
    int epoll_closed = close(epoll_fd);
    int socket_closed = close(epoll_fd);
    if (epoll_closed != 0 || socket_closed != 0) exit(99); //TODO

    return 0;
}