//
// Created by Ben Li on 2020-03-19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

int peer_add_to_send(peer_t *peer, Message *message) {
    if (peer->socket != NO_SOCKET) {
        return message_enqueue(&peer->send_buffer, message);
    } else {
        return -1;
    }
}

/* Receive message from peer and handle it with message_handler(). */
int peer_receive_msg(peer_t *peer, int32_t  expect_payload_size, unsigned char * payload) {
    log_debug("Ready for recv() from %s.", peer_get_addres_str(peer));

    int32_t  received_count;
    int32_t  received_total = 0;
    unsigned char buffer[MAX_SEND_SIZE];
    int32_t  len_to_receive;

    while (received_total < expect_payload_size) {

        len_to_receive = expect_payload_size - received_total;
        if (len_to_receive > MAX_SEND_SIZE) {
            len_to_receive = MAX_SEND_SIZE;
        }
        log_debug("Let's try to recv() %d bytes... ", len_to_receive);
        memset(buffer, '\0', sizeof(buffer));
        received_count = recv(peer->socket, buffer, len_to_receive, MSG_DONTWAIT);

        if (received_count < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                log_debug("peer is not ready right now, try again later. %d", received_count);
                return -2;
            } else {
                perror("recv() from peer error");
                return -1;
            }
        }
        // If recv() returns 0, it means that peer gracefully shutdown. Shutdown client.
        else if (received_count == 0) {
            log_debug("recv() 0 bytes. Peer gracefully shutdown.");
            return -1;
        } else if (received_count > 0) {
            if (received_count == 4) {
                log_info("size = %d", (int)buffer);
            }
            memcpy(payload + received_total, buffer, received_count);
            received_total += received_count;
            log_debug("recv() %d bytes", received_count);
        }
    };
    log_debug("Total recv()'ed %d bytes.", received_total);
    return 0;
}

/* Receive message from peer and handle it with message_handler(). */
int peer_receive_from_peer(peer_t *peer, int (*message_handler)(Message *)) {
    log_debug("Ready for recv() from %s.", peer_get_addres_str(peer));

    int32_t  received_total = 0;
    Message message;
    int received_result;
    int header = 0;
    do {
        memset(&message, 0, sizeof(Message));
        //receive header to determine size
        header = 0;
        received_result = peer_receive_msg(peer, HEADER_SIZE, &header);
        if (received_result == -1) {
            return -1;
        } else if (received_result == -2) {
            break;
        } else {
            int32_t  payload_size = (int32_t )ntohl(header);
            log_debug("payload size %d -> %d", header, payload_size);
            //receive the data payload
            received_result = peer_receive_msg(peer, payload_size, peer->receiving_buffer);
            if (received_result == -1) {
                return -1;
            }else if (received_result == -2) {
                break;
            }else {
                //receive the tail to ensure we are in good condition.
                received_result = peer_receive_msg(peer, END_OF_MESSAGE_PAYLOAD_SIZE, peer->receiving_tail);
                if (received_result == -1) {
                    return -1;
                } else if (received_result == -2) {
                    break;
                } else {
                    if (memcmp(peer->receiving_tail, END_OF_MESSAGE_PAYLOAD, END_OF_MESSAGE_PAYLOAD_SIZE) != 0) {
                        log_error("message is misaligned, go find the end of the payload signature");
                        peer_delete(peer);
                        return -1;
                    }
                }
            }
            message_bytes_to_message(peer->receiving_buffer, payload_size, &message);
            message_handler(&message);
        }
    } while (received_result > 0);

    log_debug("Total recv()'ed %d bytes.", received_total);
    return 0;
}

int peer_send_to_peer(peer_t *peer) {
    log_debug("Ready for send() to %s.", peer_get_addres_str(peer));

    int32_t  len_to_send;
    int32_t  send_count;
    int32_t  send_total = 0;
    peer->total_sending_buffer_size = 0;
    Message current;
    do {
        // If sending message has completely sent and there are messages in queue, why not send them?
        if (peer->current_sending_byte < 0 || peer->current_sending_byte >= peer->total_sending_buffer_size) {

            log_debug("There is no pending to send() message, maybe we can find one in queue... ");
            memset(&current, '\0', sizeof(Message));
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

        log_debug("Let's try to send() %d bytes... from position %d: total bytes %d",
                len_to_send,
                peer->current_sending_byte,
                peer->total_sending_buffer_size);
        send_count = send(peer->socket, (unsigned char *) peer->sending_buffer + peer->current_sending_byte, len_to_send, 0);
        if (send_count < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                log_debug("peer is not ready right now, try again later.");
            } else {
                return -1;
            }
        } else if (send_count == 0) {
            log_debug("send()'ed 0 bytes. It seems that peer can't accept data right now. Try again later.");
            break;
        } else if (send_count > 0) {
            peer->current_sending_byte += send_count;
            send_total += send_count;
            log_debug("send()'ed %d bytes.", send_count);
        }
    } while (send_total < peer->total_sending_buffer_size);

    log_debug("Total send()'ed %d bytes.", send_total);
    return 0;
}

void peer_enqueue_heart_beat(peer_t * peer, const char * name, bool shouldSend) {
    while(1) {
        if (peer->socket != NO_SOCKET) {
            Message message;
            memset(&message, '\0', sizeof(Message));
            char data[DATA_MAXSIZE];
            sprintf(data, "heartbeat from %s", name);
            prepare_message("header", data, strlen(data), &message);
            message.type = TYPE_HEART_BEAT;
            message.data_size = strlen(data) + 1;
            peer_add_to_send(peer, &message);
            if(shouldSend) {
                peer_send_to_peer(peer);
            }
        }
        sleep(HEART_BEAT_TIME_IN_SEC);
    }
}
