#include <ctype.h>
#include "helpers.h"

int get_ip_address(char *hostname, struct in_addr **ip_address) {
    struct hostent *host;

    host = gethostbyname(hostname);
    if (host == NULL) {
        return 1;
    }

    *ip_address = (struct in_addr *)host->h_addr_list[0];
    return 0;
}

void print_help() {
    const char *help_string = "-t	User provided	tcp or udp	Transport protocol used for connection\n"
        "-s	User provided	IP address or hostname	Server IP or hostname\n"
        "-p	4567	uint16	Server port\n"
        "-d	250	uint16	UDP confirmation timeout\n"
        "-r	3	uint8	Maximum number of UDP retransmissions\n"
        "-h			Prints program help output and exits\n";
    printf("%s", help_string);
}

void print_command_help() {
    const char *help_command_string = "/auth	{Username} {Secret} {DisplayName}	Sends AUTH message with the data provided from the command to the server (and correctly handles the Reply message), locally sets the DisplayName value (same as the /rename command)\n\n"
        "/join	{ChannelID}	Sends JOIN message with channel name from the command to the server (and correctly handles the Reply message)\n\n"
        "/rename	{DisplayName}	Locally changes the display name of the user to be sent with new messages/selected commands\n\n"
        "/help		Prints out supported local commands with their parameters and a description\n";

    printf("%s\n", help_command_string);
}

int parse_arguments(int argc, char *argv[], CmdArguments *arguments) {
    if (argc < 5) { //program_name -t value -s value
        fprintf(stderr, "ERR: Wrong number of arguments, run with -h to display help\n");
        return 1;
    }

    // set default values
    arguments->server_port = 4567;
    arguments->udp_confirmation_timeout = 250;
    arguments->max_udp_retransmissions = 3;

    int c;

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

char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s)); 
}

int is_empty (char *line)
{
  int is_blank = 1;

  // Iterate through each character.
  for (char *ch = line; *ch != '\0'; ++ch)
  {
    if (!isspace(*ch))
    {
      // Found a non-whitespace character.
      is_blank = 0;
      break;
    }
  }

  return is_blank;
}

