#include <string.h>
#include <stdio.h>

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
                }
            } //TODO else

            break;
        }

        default:
            //TODO
            break;
    }

    return 0;
}