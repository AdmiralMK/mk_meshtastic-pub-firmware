#pragma once
#include "mesh/MeshTypes.h"

bool eventHelp_matches(const char *msg);
void eventHelp_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);