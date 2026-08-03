#pragma once
#include "mesh/MeshTypes.h"
bool eventHello_matches(const char *msg);
void eventHello_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);