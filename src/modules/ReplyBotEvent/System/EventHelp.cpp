#include "EventHelp.h"
#include <cstring>
#include <cstdio>
#include <cctype>

bool eventHelp_matches(const char *msg) {
    return strncmp(msg, "/help", 5) == 0 && (msg[5] == '\0' || isspace((unsigned char)msg[5]));
}

void eventHelp_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize) {
    // Компактный список всех служебных команд
    snprintf(reply, replySize, 
             "📜 **Commands:**\n"
             "`/hard /mesh /nodes`\n"
             "`/ping /test /hello`"
             );
}