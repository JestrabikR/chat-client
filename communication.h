#ifndef COMMUNICATION_H
#define COMMUNICATION_H

typedef enum {
    S_START,
    S_AUTH,
    S_OPEN,
    S_ERROR,
    S_END
} CommunicationState;

#endif