#ifndef RESPONSE_H
#define RESPONSE_H

#include "messages.h"
#include "sent_messages_queue.h"

typedef struct {
    MessageType response_type;
    union {
        ErrMessage err_message;
        ReplyMessage reply_message;
        ConfirmMessage confirm_message;
        Message message;
    };
} Response;

/**
 * @returns 0 if successfully sets type, 1 if not
*/
int get_response_type(char *response_msg, MessageType *response_type);

/**
 * @brief sets Response structure
 * @returns 0 if valid response, else 0
*/
int parse_response(char *response_msg, MessageType message_type, Response *response, SM_Queue *sm_queue);

#endif