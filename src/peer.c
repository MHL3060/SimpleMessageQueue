//
// Created by Ben Li on 2020-03-19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <avro.h>
#include <pthread.h>
#include "common.h"
#include "log.h"
#include "message.h"
#include "message_queue.h"
#include "peer.h"
// peer -----------------------------------------------------------------------

void peer_delete(peer_t *peer) {
    close(peer->socket);
    delete_message_queue(&peer->send_buffer);
}

int peer_create(peer_t *peer) {
    message_create_queue(MAX_MESSAGES_BUFFER_SIZE, &peer->send_buffer);

    peer->current_sending_byte = -1;
    peer->current_receiving_byte = 0;

    return 0;
}

char *peer_get_addres_str(peer_t *peer) {
    static char ret[INET_ADDRSTRLEN + 10];
    char peer_ipv4_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peer->addres.sin_addr, peer_ipv4_str, INET_ADDRSTRLEN);
    sprintf(ret, "%s:%d", peer_ipv4_str, peer->addres.sin_port);

    return ret;
}

int peer_add_to_send(peer_t *peer, message_t *message) {
    if (peer->socket != NO_SOCKET) {

        return message_enqueue(&peer->send_buffer, message);
    } else {
        return -1;
    }

}

/* Receive message from peer and handle it with message_handler(). */
int peer_receive_from_peer(peer_t *peer, int (*message_handler)(message_t *)) {
    log_debug("Ready for recv() from %s.", peer_get_addres_str(peer));
    size_t len_to_receive;
    ssize_t received_count;
    size_t received_total = 0;
    unsigned char * payload[8192];

    do {
        // Is completely received?
        if (peer->current_receiving_byte >= sizeof(peer->receiving_buffer)) {
            message_handler(&peer->receiving_buffer);
            peer->current_receiving_byte = 0;
        }

        // Count bytes to send.
        len_to_receive = sizeof(peer->receiving_buffer) - peer->current_receiving_byte;
        if (len_to_receive > MAX_SEND_SIZE)
            len_to_receive = MAX_SEND_SIZE;

        log_debug("Let's try to recv() %zd bytes... ", len_to_receive);
        received_count = recv(peer->socket, (char *) &peer->receiving_buffer + peer->current_receiving_byte,
                              len_to_receive, MSG_DONTWAIT);
        if (received_count < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                log_debug("peer is not ready right now, try again later.");
            } else {
                perror("recv() from peer error");
                return -1;
            }
        } else if (received_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        }
            // If recv() returns 0, it means that peer gracefully shutdown. Shutdown client.
        else if (received_count == 0) {
            log_debug("recv() 0 bytes. Peer gracefully shutdown.");
            return -1;
        } else if (received_count > 0) {
            peer->current_receiving_byte += received_count;
            received_total += received_count;
            log_debug("recv() %zd bytes", received_count);
        }
    } while (received_count > 0);

    log_debug("Total recv()'ed %zu bytes.", received_total);
    return 0;
}

int peer_send_to_peer(peer_t *peer) {
    log_debug("Ready for send() to %s.", peer_get_addres_str(peer));

    size_t len_to_send;
    ssize_t send_count;
    size_t send_total = 0;
    peer->total_sending_buffer_size = 0;
    message_t current;
    do {
        // If sending message has completely sent and there are messages in queue, why not send them?

        if (peer->current_sending_byte < 0 || peer->current_sending_byte >= peer->total_sending_buffer_size) {
            log_debug("There is no pending to send() message, maybe we can find one in queue... ");
            memset(&current, '\0', sizeof(message_t));
            if (message_dequeue(&peer->send_buffer, &current) != 0) {
                peer->current_sending_byte = -1;
                log_debug("No, there is nothing to send() anymore.");
                break;
            }
            message_to_bytes(&current, peer->sending_buffer, &peer->total_sending_buffer_size);
            log_debug("Yes, pop and send() one of them.");
            peer->current_sending_byte = 0;
        }

        // Count bytes to send.
        len_to_send = peer->total_sending_buffer_size - peer->current_sending_byte;
        if (len_to_send > MAX_SEND_SIZE) {
            len_to_send = MAX_SEND_SIZE;
        }

        log_debug("Let's try to send() %zd bytes... ", len_to_send);
        send_count = send(peer->socket, (unsigned char *) &peer->sending_buffer + peer->current_sending_byte, len_to_send, 0);
        if (send_count < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                log_debug("peer is not ready right now, try again later.");
            } else {
                perror("send() from peer error");
                return -1;
            }
        }
            // we have read as many as possible
        else if (send_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        } else if (send_count == 0) {
            log_debug("send()'ed 0 bytes. It seems that peer can't accept data right now. Try again later.");
            break;
        } else if (send_count > 0) {
            peer->current_sending_byte += send_count;
            send_total += send_count;
            log_debug("send()'ed %zd bytes.", send_count);
        }
    } while (send_count > 0);

    log_debug("Total send()'ed %zu bytes.", send_total);
    return 0;
}

int peer_send_to_peer2(peer_t *peer) {
    log_debug("Ready for send() to %s.", peer_get_addres_str(peer));

    size_t len_to_send;
    ssize_t send_count;
    size_t send_total = 0;
    //do we need lock in here?
    while (peer->send_buffer.current > 0) {
        pthread_mutex_lock(&peer->send_buffer.lock);
        message_t current;
        memset(&current, '\0', sizeof(message_t));
        if (message_peak(&peer->send_buffer, &current) == 0) {
            message_to_bytes(&current, peer->sending_buffer, &peer->total_sending_buffer_size);
            peer->current_sending_byte = 0;
            while (peer->current_sending_byte < peer->total_sending_buffer_size) {
                len_to_send = peer->total_sending_buffer_size - peer->current_sending_byte;
                if (len_to_send > MAX_SEND_SIZE) {
                    len_to_send = MAX_SEND_SIZE;
                }
                log_debug("Let's try to send() %zd bytes... ", len_to_send);
                send_count = send(peer->socket, (char *) &peer->sending_buffer + peer->current_sending_byte,
                                  len_to_send, 0);
                if (send_count < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        log_debug("peer is not ready right now, try again later.");
                    } else {
                        perror("send() from peer error");
                        pthread_mutex_unlock(&peer->send_buffer.lock);
                        return -1;
                    }
                }    // we have read as many as possible
                else if (send_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    break;
                } else if (send_count == 0) {
                    log_debug("send()'ed 0 bytes. It seems that peer can't accept data right now. Try again later.");
                } else if (send_count > 0) {
                    peer->current_sending_byte += send_count;
                    send_total += send_count;
                    log_debug("send()'ed %zd bytes.", send_count);
                }
            }
            if (peer->current_sending_byte >= peer->total_sending_buffer_size) {
                log_info("we have successfully send this message. remove from the queue");
                message_dequeue_no_lock(&peer->send_buffer, &current);
            }
        }
        pthread_mutex_unlock(&peer->send_buffer.lock);

    }


    log_debug("Total send()'ed %zu bytes.", send_total);
    return 0;
}

void peer_enqueue_heart_beat(peer_t * peer, char * name, bool shouldSend, int sleepTimeInMilliSeconds) {
    while(1) {
        if (peer->socket != NO_SOCKET) {
            message_t message;
            memset(&message, '\0', sizeof(message_t));
            char data[DATA_MAXSIZE];
            sprintf(data, "heartbeat from %s", name);
            prepare_message("header", data, &message);
            message.type = TYPE_HEART_BEAT;
            message.data_size = strlen(data) + 1;
            peer_add_to_send(peer, &message);
            if(shouldSend) {
                peer_send_to_peer(peer);
            }
        }
        usleep(sleepTimeInMilliSeconds * 1000);
    }
}
