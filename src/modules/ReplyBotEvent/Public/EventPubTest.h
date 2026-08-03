#pragma once
#include "mesh/MeshTypes.h"
bool eventPubTest_matches(const char *msg);
void eventPubTest_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);