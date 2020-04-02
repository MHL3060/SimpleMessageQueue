//
// Created by Ben Li on 2020-03-19.
//

#ifndef PEER_H
#define PEER_H

#include <arpa/inet.h>
#include "common.h"
#include "message.h"
#include "message_queue.h"


void peer_delete(peer_t *peer);
int peer_create(peer_t *peer);
char *peer_get_addres_str(peer_t *peer);

int peer_add_to_send(peer_t *peer, Message *message);

/* Receive message from peer and handle it with message_handler(). */
int peer_receive_from_peer(peer_t *peer, int (*message_handler)(Message *));
int peer_send_to_peer(peer_t *peer);
void peer_enqueue_heart_beat(peer_t * peer, char * name, bool shouldSend);
#endif //PEER_H
