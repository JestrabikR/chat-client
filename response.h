#ifndef RESPONSE_H
#define RESPONSE_H

#include "messages.h"

/**
 * @returns 0 if successfully sets type, 1 if not
*/
int get_response_type(char *response, MessageType *response_type);

#endif