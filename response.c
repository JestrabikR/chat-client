#include <string.h>
#include "response.h"
#include "messages.h"

int parse_response(char *response, int response_size) {

// zkontrolovat jestli je to confirm
    MessageType msg_type;
    memcpy(msg_type, response, sizeof(MessageType));

    printf("RES TYPE: %d\n", msg_type);
// zkontrolovat podle prvniho bytu jaky je to typ zpravy

}