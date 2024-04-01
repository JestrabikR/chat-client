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

            // char *message_content;
            response->reply_message.message_content = malloc(strlen(pos) + 1);
            if (response->reply_message.message_content == NULL) {
                exit(99); //TODO: cleanup() / err / bye ??
            }
            strcpy(response->reply_message.message_content, pos);

            break;
        }

        case MT_MSG: {
            // MessageType msg_type;
            char *pos = response_msg;

            response->message.msg_type = MT_MSG;

            pos += sizeof(response->message.msg_type);
            
            // uint16_t message_id;
            memcpy(&response->message.message_id,
                    pos,
                    sizeof(response->message.message_id));
            // prevod little na big endian
            response->message.message_id = ntohs(response->message.message_id);
            
            pos += sizeof(response->message.message_id);
            
            // char *display_name;
            response->message.display_name = malloc(strlen(pos) + 1);
            if (response->message.display_name == NULL) {
                exit(99); //TODO: cleanup() / err / bye ??
            }
            strcpy(response->message.display_name, pos);
            
            // char *message_content;
            response->message.message_content = malloc(strlen(pos) + 1);
            if (response->message.message_content == NULL) {
                exit(99); //TODO: cleanup() / err / bye ??
            }
            strcpy(response->message.message_content, pos);

            break;
        }

        case MT_ERR: {
            // MessageType msg_type;
            char *pos = response_msg;

            response->err_message.msg_type = MT_ERR;

            pos += sizeof(response->err_message.msg_type);
            
            // uint16_t message_id;
            memcpy(&response->err_message.message_id,
                    pos,
                    sizeof(response->err_message.message_id));
            // prevod little na big endian
            response->err_message.message_id = ntohs(response->err_message.message_id);
            
            pos += sizeof(response->err_message.message_id);
            
            // char *display_name;
            response->err_message.display_name = malloc(strlen(pos) + 1);
            if (response->err_message.display_name == NULL) {
                exit(99); //TODO: cleanup() / err / bye ??
            }
            strcpy(response->err_message.display_name, pos);
            
            // char *message_content;
            response->err_message.message_content = malloc(strlen(pos) + 1);
            if (response->err_message.message_content == NULL) {
                exit(99); //TODO: cleanup() / err / bye ??
            }
            strcpy(response->err_message.message_content, pos);
        }

        default:
            //TODO
            break;
    }

    return 0;
}

void free_response(Response *response) {
    switch (response->response_type) {
        case MT_MSG:
            free(response->message.display_name);
            free(response->message.message_content);
            break;
        case MT_ERR:
            free(response->err_message.display_name);
            free(response->err_message.message_content);
            break;
        case MT_REPLY:
            free(response->reply_message.message_content);
            break;
        default:
            break;
    }
}