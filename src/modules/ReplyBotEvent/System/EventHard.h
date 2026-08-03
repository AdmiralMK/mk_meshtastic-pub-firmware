#pragma once
#include "mesh/MeshTypes.h"

bool eventHard_matches(const char *msg);
void eventHard_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);