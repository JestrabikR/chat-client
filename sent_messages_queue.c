#include "sent_messages_queue.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


void sm_queue_init(SM_Queue *sm_queue) {
    sm_queue->front = NULL;
    sm_queue->rear = NULL;
}


void sm_queue_free(SM_Queue *sm_queue) {
    while (sm_queue->front != NULL) {
        Node* temp = sm_queue->front;
        sm_queue->front = sm_queue->front->next;
        free(temp);
    }
    sm_queue->rear = NULL;
}


bool sm_queue_is_empty(SM_Queue *sm_queue) {
    return (sm_queue->front == NULL);
}


void sm_queue_enqueue(SM_Queue *sm_queue, uint16_t message_id) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        return;
    }
    new_node->data = message_id;
    new_node->next = NULL;
    if (sm_queue->rear == NULL) {
        
        sm_queue->front = new_node;
    } else {
        
        sm_queue->rear->next = new_node;
    }
    
    sm_queue->rear = new_node;
}


bool sm_queue_dequeue(SM_Queue *sm_queue, uint16_t *message_id) {
    if (sm_queue->front == NULL) {
        return false;
    }
    
    *message_id = sm_queue->front->data;
    
    Node* temp = sm_queue->front;
    
    sm_queue->front = sm_queue->front->next;
    
    if (sm_queue->front == NULL) {
        sm_queue->rear = NULL;
    }
    
    free(temp);
    return true;
}

bool sm_queue_peek(SM_Queue *sm_queue, uint16_t *message_id) {
    if (sm_queue->front == NULL) {
        // Pokud je fronta prázdná, není co nahlížet
        return false;
    }
    // Uložíme hodnotu z prvního prvku fronty
    *message_id = sm_queue->front->data;
    return true;
}
