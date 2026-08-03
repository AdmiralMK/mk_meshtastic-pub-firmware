#pragma once
#include "mesh/MeshTypes.h"

bool eventNodes_matches(const char *msg);
void eventNodes_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);

