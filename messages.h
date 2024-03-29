#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>

#define USERNAME_MAX_LEN 21 //TODO: 20 + 1 na '\0'?
#define CHANNEL_ID_MAX_LEN 21
#define SECRET_MAX_LEN 129
#define DISPLAY_NAME_MAX_LEN 21
#define MESSAGE_CONTENT_MAX_LEN 1401

typedef enum MessageType {
    CONFIRM = 0x0,
    REPLY = 0x01,
    AUTH = 0x02,
    JOIN = 0x03,
    MSG = 0x04,
    ERR	= 0xFE,
    BYE	= 0xFF
} MessageType;

typedef struct {
    MessageType msg_type;
    uint16_t ref_message_id;
} ConfirmMessage;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
    bool result;
    uint16_t ref_message_id;
    char message_content[MESSAGE_CONTENT_MAX_LEN];
} ReplyMessage;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
    char username[USERNAME_MAX_LEN];
    //TODO: jak se tam dodelaji ty separujici 0?
    char display_name[DISPLAY_NAME_MAX_LEN];
    char secret[SECRET_MAX_LEN];
} AuthMessage;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
    char channel_id[CHANNEL_ID_MAX_LEN];
    char display_name[DISPLAY_NAME_MAX_LEN];
} JoinMessage;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
    char display_name[DISPLAY_NAME_MAX_LEN];
    char message_content[MESSAGE_CONTENT_MAX_LEN];
} Message;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
    char display_name[DISPLAY_NAME_MAX_LEN];
    char message_content[MESSAGE_CONTENT_MAX_LEN];
} ErrMessage;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
} ByeMessage;

#endif