#ifndef SM_QUEUE_H
#define SM_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Node {
    uint16_t data;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
} SM_Queue;

void sm_queue_init(SM_Queue *sm_queue);

void sm_queue_free(SM_Queue *sm_queue);

bool sm_queue_is_empty(SM_Queue *sm_queue);

void sm_queue_enqueue(SM_Queue *sm_queue, uint16_t message_id);

bool sm_queue_dequeue(SM_Queue *sm_queue, uint16_t *message_id);

#endif
