#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>
#include <stdbool.h>

#define USERNAME_MAX_LEN 21 //TODO: 20 + 1 na '\0'?
#define CHANNEL_ID_MAX_LEN 21
#define SECRET_MAX_LEN 129
#define DISPLAY_NAME_MAX_LEN 21
#define MESSAGE_CONTENT_MAX_LEN 1401

typedef enum {
    MT_CONFIRM = 0x0,
    MT_REPLY = 0x01,
    MT_AUTH = 0x02,
    MT_JOIN = 0x03,
    MT_MSG = 0x04,
    MT_ERR	= 0xFE,
    MT_BYE	= 0xFF
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

typedef struct{
    MessageType msg_type;
    uint16_t message_id;
    char *username; //TODO: predelat na dynamicky alokovane char[] VSECHNO
    char *display_name;
    char *secret;
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
