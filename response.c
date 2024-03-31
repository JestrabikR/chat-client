#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "response.h"
#include "messages.h"
#include "arpa/inet.h"
#include "sent_messages_queue.h"

int get_response_type(char *response_msg, MessageType *response_type) {

    memcpy(response_type, response_msg, sizeof(MessageType));

    //pokud bude prvni byte bude jiny nez typy z enumu -> chyba
    if (*response_type != MT_AUTH && *response_type != MT_BYE &&
        *response_type != MT_CONFIRM && *response_type != MT_ERR &&
        *response_type != MT_JOIN && *response_type != MT_MSG &&
        *response_type != MT_REPLY) {
            return 1;
        }

    return 0;
}

int parse_response(char *response_msg, MessageType message_type, Response *response, SM_Queue *sm_queue) {
    response->response_type = message_type;

    switch (message_type) {
        case MT_CONFIRM: {
            response->confirm_message.msg_type = MT_CONFIRM;
            memcpy(&response->confirm_message.ref_message_id,
                    response_msg + sizeof(response->confirm_message.msg_type),
                    sizeof(response->confirm_message.ref_message_id));
            // prevod little na big endian
            response->confirm_message.ref_message_id = ntohs(response->confirm_message.ref_message_id);
            

            if (sm_queue_is_empty(sm_queue) == false) {
                uint16_t msg_id;
                sm_queue_peek(sm_queue, &msg_id);

                // spravny confirm
                if (response->confirm_message.ref_message_id == msg_id) {
                    // odstraneni z queue
                    sm_queue_dequeue(sm_queue, &msg_id);
                } //jinak se ceka na dalsi a tento se ignoruje
            } //TODO else

            break;
        }

        case MT_REPLY: {
            char *pos = response_msg;

            response->reply_message.msg_type = MT_REPLY;

            pos += sizeof(response->reply_message.msg_type);
            
            // uint16_t message_id;
            memcpy(&response->reply_message.message_id,
                    pos,
                    sizeof(response->reply_message.message_id));
            // prevod little na big endian
            response->reply_message.message_id = ntohs(response->reply_message.message_id);
            
            pos += sizeof(response->reply_message.message_id);

            // bool result;
            memcpy(&response->reply_message.result,
                    pos,
                    sizeof(response->reply_message.result));

            pos += sizeof(response->reply_message.result);

            // uint16_t ref_message_id;
            memcpy(&response->reply_message.ref_message_id,
                    pos,
                    sizeof(response->reply_message.ref_message_id));

            pos += sizeof(response->reply_message.ref_message_id);

            // char message_content[MESSAGE_CONTENT_MAX_LEN];
            response->reply_message.message_content = malloc(strlen(pos) + 1);
            if (response->reply_message.message_content == NULL) {
                exit(99); //TODO: cleanup() / err / bye ??
            }
            strcpy(response->reply_message.message_content, pos);

            break;
        }

        case MT_ERR: {

            break;
        }

        default:
            //TODO
            break;
    }

    return 0;
}