#ifndef _SERVER_H
#define _SERVER_H

#define MAX_MEMBER 100

typedef struct _CHAT_ROOM {
    char name[SIZE_OF_SHORT_STRING];
    Mail_box_t *client_box;
} chat_room;

#endif
