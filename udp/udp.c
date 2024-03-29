#include "udp.h"

int udp_connect(int socket_fd, const char *ip, int port) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr.s_addr);
    
    int ret_value = connect(socket_fd, (struct sockaddr *)&address, sizeof(address));
    if (ret_value == -1) return 1;

    printf("connected\n"); //TODO: REMOVE
    return 0;
}