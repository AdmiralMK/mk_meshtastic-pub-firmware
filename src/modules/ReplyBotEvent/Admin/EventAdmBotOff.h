
#pragma once
#include "mesh/MeshTypes.h"
bool eventAdmBotOff_matches(const char *msg);
void eventAdmBotOff_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize);