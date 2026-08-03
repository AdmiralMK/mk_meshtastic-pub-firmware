#pragma once
#include "mesh/MeshTypes.h"

bool eventTelemetry_matches(const char *msg);
void eventTelemetry_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);
