#pragma once
#include "mesh/MeshTypes.h"
#include <cstddef>
#include <cstdint>

bool eventAdmSendmsg_matches(const char *msg);
void eventAdmSendmsg_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize);

/** Запись отложенной broadcast-отправки */
struct PendingSendMsg {
    bool active = false;
    uint32_t sendAtMs = 0;
    char text[200] = {0};
};

/** Забрать задачу, если время пришло. true = нужно отправить text. */
bool eventAdmSendmsg_poll(PendingSendMsg &out);