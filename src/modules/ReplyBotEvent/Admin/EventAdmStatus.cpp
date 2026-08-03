#include "EventAdmStatus.h"
#include "../BotState.h"
#include "NodeDB.h"
#include <cstring>
#include <cstdio>
#include <cctype>

bool eventAdmStatus_matches(const char *msg) {
    return strncmp(msg, "/adm_status", 11) == 0;
}

void eventAdmStatus_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize) {
    if (!args) { snprintf(reply, replySize, "❌ Usage: /adm_status !id"); return; }
    
    char target[32];
    if (sscanf(args, "%31s", target) != 1) {
        snprintf(reply, replySize, "❌ Usage: /adm_status !id");
        return;
    }

    uint32_t ourNode = nodeDB->getNodeNum();
    char ourIdStr[32];
    snprintf(ourIdStr, sizeof(ourIdStr), "!%08x", ourNode);
    
    if (strcasecmp(target, ourIdStr) == 0) {
        auto &f = BotState::instance().flags;
        snprintf(reply, replySize, "📋 **Node bot status** %s:\nBot: %s\nSystem (primary): %s\nSystem (DM): %s\nTest net (primary): %s\nTest net (DM): %s", 
                 target,
                 f.bot_enabled ? "ON" : "OFF",
                 f.system_pub_enabled ? "ON" : "OFF",
                 f.system_dm_enabled ? "ON" : "OFF",
                 f.test_pub_enabled ? "ON" : "OFF",
                 f.test_dm_enabled ? "ON" : "OFF");
    } else {
        //snprintf(reply, replySize, "⏩ Ignored (not for this node)");
        reply[0] = '\0';  // не отправлять
    }
}