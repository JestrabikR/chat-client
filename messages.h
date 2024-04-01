#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>
#include <stdbool.h>

#define USERNAME_MAX_LEN 21 // 20 + 1 for '\0'?
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
}__attribute__((packed)) MessageType;

typedef struct {
    MessageType msg_type;
    uint16_t ref_message_id;
}__attribute__((packed)) ConfirmMessage;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
    bool result;
    uint16_t ref_message_id;
    char *message_content;
}__attribute__((packed)) ReplyMessage;

typedef struct{
    MessageType msg_type;
    uint16_t message_id;
    char *username;
    char *display_name;
    char *secret;
}__attribute__((packed)) AuthMessage;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
    char *channel_id;
    char *display_name;
}__attribute__((packed)) JoinMessage;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
    char *display_name;
    char *message_content;
}__attribute__((packed)) Message;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
    char *display_name;
    char *message_content;
}__attribute__((packed)) ErrMessage;

typedef struct {
    MessageType msg_type;
    uint16_t message_id;
}__attribute__((packed)) ByeMessage;

#endif
