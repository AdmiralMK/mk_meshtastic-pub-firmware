#pragma once
#include "mesh/MeshTypes.h"
bool eventAdmCmdsys_matches(const char *msg);
void eventAdmCmdsys_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize);