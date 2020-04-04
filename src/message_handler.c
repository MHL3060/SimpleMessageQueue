//
// Created by Ben Li on 2020-04-01.
//

#include "message_handler.h"
#include "common.h"
#include <ao/ao.h>

static bool audio_initialize = false;
static ao_device * audio_device;
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

void init_audio() {
    if (!audio_initialize) {
        log_info("initialize audio");
        ao_initialize();
        audio_device = open_ao_live();
    }
}
void as_audio(const Message * message) {
    if (audio_initialize == false) {
        init_audio();
        audio_initialize = true;
    } else {
        ao_play(audio_device, message->data, sizeof(message->data_size));
    }
}

int32_t handle_message(Message * message) {
    if (message->type == TYPE_DATA) {
        wire_tap(message->data);
    } else if (message->type == TYPE_AUDIO) {
       // printf("%s", message->data);
        // as_audio(message);
        wire_tap(message->data);
    }

    return 0;
}

