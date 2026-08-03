#pragma once
#include "mesh/MeshTypes.h"
#include <cstddef>
#include <cstdint>

bool eventAdmHere_matches(const char *msg);
void eventAdmHere_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize);

struct PendingHereMsg {
    bool active = false;
    uint32_t sendAtMs = 0;
    char text[200] = {0};
};

/** true = пора отправить text в ADMBOT */
bool eventAdmHere_poll(PendingHereMsg &out);