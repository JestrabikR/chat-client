#include <string.h>
#include <stdio.h>

#include "response.h"
#include "messages.h"

int get_response_type(char *response, MessageType *response_type) {

    memcpy(response_type, response, sizeof(MessageType));

    //pokud bude prvni byte bude jiny nez typy z enumu
    if (*response_type != MT_AUTH && *response_type != MT_BYE &&
        *response_type != MT_CONFIRM && *response_type != MT_ERR &&
        *response_type != MT_JOIN && *response_type != MT_MSG &&
        *response_type != MT_REPLY) {
            return 1;
        }

    return 0;
}