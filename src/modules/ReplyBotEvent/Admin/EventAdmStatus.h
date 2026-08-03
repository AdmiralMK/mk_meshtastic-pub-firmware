#pragma once
#include "mesh/MeshTypes.h"
bool eventAdmStatus_matches(const char *msg);
void eventAdmStatus_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize);