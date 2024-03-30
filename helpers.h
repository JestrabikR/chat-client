#ifndef HELPERS_H
#define HELPERS_H


#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>

int getopt(int argc, char * const argv[],
           const char *optstring);

extern char *optarg;
extern int optind, opterr, optopt;

typedef struct {
    bool is_udp;
    char server_ip_or_hostname[256];
    unsigned int server_port;
    unsigned int udp_confirmation_timeout;
    unsigned int max_udp_retransmissions;
} CmdArguments;

int get_ip_address(char *hostname, struct in_addr **ip_address);

void print_help();

void print_command_help();

/**
 * @brief validates and parses arguments
 * @param arguments fills this structure with data if ended successfully
 * @returns returns 1 in case of error else 0
*/
int parse_arguments(int argc, char *argv[], CmdArguments *arguments);

char *ltrim(char *s);

char *rtrim(char *s);

char *trim(char *s);

/**
 * @returns Returns nonzero if line is a string containing only whitespace (or is empty)
*/ 
int is_empty (char *line);


#endif
