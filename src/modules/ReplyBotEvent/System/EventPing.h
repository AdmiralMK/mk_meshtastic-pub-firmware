#pragma once
#include "mesh/MeshTypes.h"

bool eventPing_matches(const char *msg);
void eventPing_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);