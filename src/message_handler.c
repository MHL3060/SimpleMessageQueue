//
// Created by Ben Li on 2020-04-01.
//

#include "message_handler.h"
#include "common.h"
#include "message_queue.h"
#include <ao/ao.h>
#include <pthread.h>

static bool audio_initialize = false;
static ao_device * audio_device;
static pthread_t audio_thread;

MessageQueue messageQueue;
void wire_tap(const unsigned char *message) {
    log_info("Message: \" %s\"", message);
    // printf("%s", message);
}


static ao_sample_format sample_format = {
        16,
        44100,
        2,
        AO_FMT_NATIVE,
        "L,R"
};

static ao_device *open_ao_live()
{
    return ao_open_live(ao_default_driver_id(), &sample_format, NULL);
}


void play_audio() {

    Message message;
    while (true) {
        memset(&message, 0, sizeof(Message));
        if (message_dequeue(&messageQueue, &message) != -1) {
            ao_play(audio_device, message.data, message.data_size);
        }
    }
}

void init_audio() {
    if (!audio_initialize) {
        log_info("initialize audio");

        message_create_queue(2000, &messageQueue);
        ao_initialize();
        audio_device = open_ao_live();
        pthread_create(&audio_thread, NULL, (void *)&play_audio, NULL);
    }
}

void as_audio(const Message * message) {
    if (audio_initialize == false) {
        init_audio();
        audio_initialize = true;
    }
    while(message_enqueue(&messageQueue, message) == -1) {
        log_info("queue is full. wait");
        usleep(5000);
    }
}


int32_t handle_message(Message * message) {
    if (message->type == TYPE_DATA) {
        wire_tap(message->data);
    } else if (message->type == TYPE_AUDIO) {
       // printf("%s", message->data);
        as_audio(message);
       // wire_tap(message->data);
    }

    return 0;
}





