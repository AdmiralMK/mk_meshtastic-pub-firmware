#pragma once
#include "mesh/MeshTypes.h"
bool eventTest_matches(const char *msg);
void eventTest_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);