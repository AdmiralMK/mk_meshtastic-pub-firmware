#include "EventAdmCmdtst.h"
#include "../BotState.h"
#include "NodeDB.h"
#include <cstring>
#include <cstdio>
#include <cctype>

bool eventAdmCmdtst_matches(const char *msg)
{
    return strncmp(msg, "/adm_cmdtst", 11) == 0;
}

void eventAdmCmdtst_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize)
{
    if (!args) {
        snprintf(reply, replySize, "❌ Usage: /adm_cmdtst !id|all pub|dm on|off");
        return;
    }

    char target[32], scope[16], state[16];
    if (sscanf(args, "%31s %15s %15s", target, scope, state) != 3) {
        snprintf(reply, replySize, "❌ Usage: /adm_cmdtst !id|all pub|dm on|off");
        return;
    }

    if (strcasecmp(scope, "pub") != 0 && strcasecmp(scope, "dm") != 0) {
        snprintf(reply, replySize, "❌ Usage: /adm_cmdtst !id|all pub|dm on|off");
        return;
    }
    if (strcasecmp(state, "on") != 0 && strcasecmp(state, "off") != 0) {
        snprintf(reply, replySize, "❌ Usage: /adm_cmdtst !id|all pub|dm on|off");
        return;
    }

    bool turnOn = (strcasecmp(state, "on") == 0);
    bool &flag = (strcasecmp(scope, "pub") == 0) ? BotState::instance().flags.test_pub_enabled
                                                  : BotState::instance().flags.test_dm_enabled;

    uint32_t ourNode = nodeDB->getNodeNum();
    char ourIdStr[32];
    snprintf(ourIdStr, sizeof(ourIdStr), "!%08x", ourNode);

    if (strcasecmp(target, "all") == 0 || strcasecmp(target, ourIdStr) == 0) {
        flag = turnOn;
        BotState::instance().save();
        snprintf(reply, replySize, "✅ Test net commands (%s) turned %s on node %s", scope, turnOn ? "ON" : "OFF", target);
    } else {
        //snprintf(reply, replySize, "⏩ Ignored (not for this node)");
        reply[0] = '\0';  // не отправлять
    }
}