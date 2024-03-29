#include "messages.h"

uint16_t message_id = 0;

uint16_t get_message_id() {
    return message_id++;
}