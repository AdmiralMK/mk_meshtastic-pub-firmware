#pragma once
#include "mesh/MeshTypes.h"
bool eventPubHello_matches(const char *msg);
void eventPubHello_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);