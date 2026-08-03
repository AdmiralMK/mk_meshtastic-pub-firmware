#pragma once
#include "mesh/MeshTypes.h"
bool eventPubPing_matches(const char *msg);
void eventPubPing_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);