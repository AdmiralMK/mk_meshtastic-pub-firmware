#include "EventAdmBotOff.h"
#include "../BotState.h"
#include "NodeDB.h"
#include <cstring>
#include <cstdio>
#include <cctype>

bool eventAdmBotOff_matches(const char *msg) {
    return strncmp(msg, "/adm_bot_off", 12) == 0;
}

void eventAdmBotOff_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize) {
    if (!args) { snprintf(reply, replySize, "❌ Usage: /adm_bot_off !id|all"); return; }
    
    char target[32];
    if (sscanf(args, "%31s", target) != 1) {
        snprintf(reply, replySize, "❌ Usage: /adm_bot_off !id|all");
        return;
    }

    uint32_t ourNode = nodeDB->getNodeNum();
    char ourIdStr[32];
    snprintf(ourIdStr, sizeof(ourIdStr), "!%08x", ourNode);
    
    if (strcasecmp(target, "all") == 0 || strcasecmp(target, ourIdStr) == 0) {
        BotState::instance().flags.bot_enabled = false;
        BotState::instance().save();
        snprintf(reply, replySize, "🛑 Bot stopped on node %s", target);
    } else {
        //snprintf(reply, replySize, "⏩ Ignored (not for this node)");
        reply[0] = '\0';  // не отправлять
    }
}