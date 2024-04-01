#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h> //close
#include <stdbool.h>
#include <signal.h>

#include "main.h"
#include "messages.h"
#include "communication.h"
#include "helpers.h"
#include "commands.h"
#include "response.h"
#include "sent_messages_queue.h"
#include "tcp.h"

#define MAX_EVENTS 3

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

void sigint_handler(int sig_no) {
    (void)sig_no; // only SIGINT signal can happen, this line is to suppress warning
    if (arguments->is_udp) {
        udp_send_bye_wait_for_confirm();
    }
    else {
        tcp_send_bye(socket_fd);
    }

    clean_up();

    exit(EXIT_SUCCESS);
}

int setup_socket() {
    if (arguments->is_udp) {
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    } else {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    }

    if (socket_fd == -1)
        return 1;

    socket_event.events = EPOLLIN;
    stdin_event.events = EPOLLIN;

    socket_event.data.fd = socket_fd;
    stdin_event.data.fd = stdin_fd;

    return 0;
}

int setup_epoll() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        return 1;

    int ret_value = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &socket_event);
    if (ret_value == -1)
        return 1;
    ret_value = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, stdin_fd, &stdin_event);
    if (ret_value == -1)
        return 1;
    
    return 0;
}

int setup_udp() {
    if (setup_socket() == 1)
        return 1;

    if (setup_epoll() == 1)
        return 1;

    address.sin_family = AF_INET;
    address.sin_port = htons(arguments->server_port);
    inet_pton(AF_INET, arguments->server_ip_or_hostname, &address.sin_addr.s_addr);
    
    return 0;
}

int setup_tcp() {
    if (setup_socket() == 1)
        return 1;
    
    if (setup_epoll() == 1)
        return 1;

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(arguments->server_port);
    inet_pton(AF_INET, arguments->server_ip_or_hostname, &server_address.sin_addr);

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
        return 1;

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
    bool authenticated = false;

    ret_value = parse_arguments(argc, argv, arguments);
    if (ret_value == 1) return 1;

    if (get_ip_address(arguments->server_ip_or_hostname, &ip_address) == 1)
        return 1;
    
    if (arguments->is_udp)
        sm_queue_init(&sm_queue);

    signal(SIGINT, sigint_handler);

    if (arguments->is_udp) {
        if(setup_udp() != 0) {
            current_state = S_ERROR;
        }
    } else {
        if (setup_tcp() != 0) {
            current_state = S_ERROR;
        }
    }

    unsigned int addr_len = 0;

    while (true) {
        if (current_state == S_ERROR) {
            if (arguments->is_udp) {
                udp_send_bye_wait_for_confirm();
            } else {
                tcp_send_bye(socket_fd);
            }
            clean_up();
            return 0;
        }

        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) {
            handle_error();
            continue;
        }

        if (events[0].data.fd == stdin_fd) {
            char line[STDIN_BUFF_SIZE];
            fgets(line, STDIN_BUFF_SIZE, stdin); // fgets stores \n
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
            
            else if (cmd_type == CMD_AUTH && authenticated) {
                fprintf(stderr, "ERR: You are already authenticated\n");
                continue;

            } else if (cmd_type == CMD_MESSAGE && current_state != S_OPEN) {
                fprintf(stderr, "ERR: Cannot send message right now (you have to be authenticated)\n");
                continue;
            
            } else if (cmd_type == CMD_JOIN && current_state != S_OPEN) {
                fprintf(stderr, "ERR: Cannot join channel right now (you have to be authenticated)\n");
                continue;
            }

            Command command;
            if (parse_command(line, cmd_type, &command, local_display_name) == 1)
                continue; // wrong command, skip, wait for another

            if (arguments->is_udp) {    
                if (udp_send_message_from_command(&command, socket_fd, &address, &sm_queue) == 1) {
                    free_command(&command);
                    handle_error();
                    continue;
                }
            } else {
                if (tcp_send_message_from_command(&command, socket_fd) == 1) {
                    handle_error();
                    free_command(&command);
                    continue;
                }

            }

            if (cmd_type == CMD_AUTH) {
                if (arguments->is_udp == false) {
                    authenticated = true;
                    current_state = S_OPEN;
                }
                auth_sent = true;
            }
            
            free_command(&command);
        } else {
            char response_msg[RES_BUFF_SIZE];

            if (arguments->is_udp) {
                ret_value = recvfrom(socket_fd,
                                response_msg,
                                RES_BUFF_SIZE, 0,
                                (struct sockaddr *)&address,
                                &addr_len);
            } else {
                ret_value = recv(socket_fd,
                                response_msg,
                                RES_BUFF_SIZE, 0);
            }

            if (ret_value == -1) {
                handle_error();
                continue;
            }
            if ((unsigned int)ret_value > RES_BUFF_SIZE) {
                handle_error();
                continue;
            }

            MessageType response_type;

            // spatny typ zpravy nebo typ poslan ve spatnem stavu
            if (get_response_type(response_msg, &response_type) == 1 ||
                (current_state == S_OPEN && response_type != MT_MSG &&
                response_type != MT_REPLY && response_type != MT_ERR &&
                response_type != MT_BYE && response_type != MT_CONFIRM)) {
                    fprintf(stderr, "ERR: Wrong type of message sent - not allowed in this state\n");

                    if (arguments->is_udp) {
                        udp_send_error_wait_for_confirm("Wrong type of message sent - not allowed in this state");
                    } else {
                        tcp_send_error(socket_fd, "Wrong type of message sent - not allowed in this state", local_display_name);
                    }

                    current_state = S_ERROR; // BYE will be sent in next iteration and program will end
                    continue;
                    
                }
                
            Response response;
            if (parse_response(response_msg, response_type, &response, &sm_queue) == 1) {
                handle_error();
                continue;
            }

            if (response_type == MT_REPLY && response.reply_message.result && auth_sent) {
                current_state = S_OPEN;
                auth_sent = false;
                authenticated = true;
            } 

            if (response_type == MT_REPLY) {
                if (response.reply_message.result) {
                    fprintf(stderr, "Success: %s\n", response.reply_message.message_content);
                } else {
                    fprintf(stderr, "Failure: %s\n", response.reply_message.message_content);
                }
                if (arguments->is_udp) {
                    udp_send_confirm(response.reply_message.message_id);
                }

            } else if (response_type == MT_MSG) {
                printf("%s: %s\n", response.message.display_name, response.message.message_content);
                if (arguments->is_udp) {
                    udp_send_confirm(response.message.message_id);
                }

            } else if (response_type == MT_ERR) {
                fprintf(stderr, "ERR FROM %s: %s\n", response.err_message.display_name, response.err_message.message_content);
                udp_send_confirm(response.err_message.message_id);
                current_state = S_ERROR; // BYE will be sent in next iteration and program will end

            } else if (response_type == MT_BYE) {
                if (arguments->is_udp) {
                    udp_send_confirm(response.bye_message.message_id);
                }

                clean_up();
                free_response(&response);
                return 0;
            }

            free_response(&response);
        }
    }

    clean_up();

    return 0;
}

int wait_for_confirm() {
    while (true) {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1)
            return 1;

        if (events[0].data.fd == stdin_fd) {
            continue; // ignoruje se
        } else {
            char response_msg[RES_BUFF_SIZE];
            unsigned int addr_len = 0;

            int ret_value = recvfrom(socket_fd,
                                response_msg,
                                RES_BUFF_SIZE, 0,
                                (struct sockaddr *)&address,
                                &addr_len);

            if (ret_value == -1) {
                return 1;
            }

            MessageType response_type;
            if (get_response_type(response_msg, &response_type) == 1) {
                continue; // wrong type -> ignore it
            }

            Response response;
            if (parse_response(response_msg, response_type, &response, &sm_queue) == 1) {
                return 1;
            }
            if (response_type == MT_CONFIRM) {
                free_response(&response);
                break;
            }
        }
    }
    return 0;
}

int udp_send_bye_wait_for_confirm() {
    Command bye_command = {
            .command_type = CMD_EXIT
        };

    if (udp_send_message_from_command(&bye_command, socket_fd, &address, &sm_queue) == 1) {
        return 1;
    }

    if (wait_for_confirm() == 1)
        return 1;
    
    return 0;
}

int udp_send_confirm(uint16_t message_id) {
    ConfirmMessage confirm_message = {
        .msg_type = MT_CONFIRM,
        .ref_message_id = htons(message_id) // conversion to server endianity (big endian)
    };

    int result = sendto(socket_fd,
                     &confirm_message,
                     sizeof(confirm_message), 0,
                     (struct sockaddr *)&address,
                     sizeof(address));
    if (result == -1) {
        return 1;
    }

    return 0;
}

int udp_send_error_wait_for_confirm(char *message_content) {
    ErrMessage error_message = {
        .msg_type = MT_ERR,
        .message_id = htons(get_message_id_and_inc()), // conversion to server endianity (big endian)
    };

    error_message.display_name = malloc(strlen(local_display_name) + 1);
    if (error_message.display_name == NULL) {
        return 1;
    }

    strcpy(error_message.display_name, local_display_name);

    error_message.message_content = malloc(strlen(message_content) + 1);
    if (error_message.message_content == NULL) {
        free(error_message.message_content);
        return 1;
    }

    strcpy(error_message.message_content, message_content);

    int result = sendto(socket_fd,
                     &error_message,
                     sizeof(error_message), 0,
                     (struct sockaddr *)&address,
                     sizeof(address));
    if (result == -1) {
        free(error_message.display_name);
        free(error_message.message_content);
        return 1;
    }

    free(error_message.display_name);
    free(error_message.message_content);

    if (wait_for_confirm() == 1)
        return 1;

    return 0;
}

void handle_error() {
    current_state = S_ERROR;
    fprintf(stderr, "ERR: Internal error\n");
    if (arguments->is_udp) {
        udp_send_error_wait_for_confirm("Wrong type of message sent - not allowed in this state");
    } else {
        tcp_send_error(socket_fd, "Wrong type of message sent - not allowed in this state", local_display_name);
    }
}

void clean_up() {
    if (arguments->is_udp) {
        sm_queue_free(&sm_queue);
    }
    
    free(arguments);
    close(epoll_fd);
    close(socket_fd);
}