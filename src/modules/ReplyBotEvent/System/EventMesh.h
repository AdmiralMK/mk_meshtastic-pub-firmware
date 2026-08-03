#pragma once
#include "mesh/MeshTypes.h"

bool eventMesh_matches(const char *msg);
void eventMesh_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);