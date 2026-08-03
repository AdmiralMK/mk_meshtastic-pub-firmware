#pragma once
#include "mesh/MeshTypes.h"

bool eventAdmReboot_matches(const char *msg);
void eventAdmReboot_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize);

/** true = после отправки ответа нужен reboot */
bool eventAdmReboot_consumePending();