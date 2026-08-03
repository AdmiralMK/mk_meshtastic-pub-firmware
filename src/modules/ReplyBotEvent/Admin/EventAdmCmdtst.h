#pragma once
#include "mesh/MeshTypes.h"
bool eventAdmCmdtst_matches(const char *msg);
void eventAdmCmdtst_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize);