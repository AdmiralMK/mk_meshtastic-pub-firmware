#include "EventAdmReboot.h"
#include "NodeDB.h"
#include <cstring>
#include <cstdio>
#include <cctype>

static bool s_pendingReboot = false;

bool eventAdmReboot_matches(const char *msg)
{
    return strncmp(msg, "/adm_reboot", 11) == 0;
}

bool eventAdmReboot_consumePending()
{
    if (!s_pendingReboot)
        return false;
    s_pendingReboot = false;
    return true;
}

void eventAdmReboot_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize)
{
    if (!args) {
        snprintf(reply, replySize, "❌ Usage: /adm_reboot !id|all");
        return;
    }

    char target[32];
    if (sscanf(args, "%31s", target) != 1) {
        snprintf(reply, replySize, "❌ Usage: /adm_reboot !id|all");
        return;
    }

    uint32_t ourNode = nodeDB->getNodeNum();
    char ourIdStr[32];
    snprintf(ourIdStr, sizeof(ourIdStr), "!%08x", ourNode);

    if (strcasecmp(target, "all") == 0 || strcasecmp(target, ourIdStr) == 0) {
        // Только флаг: рестарт после sendReply в ReplyBotModule
        s_pendingReboot = true;
        snprintf(reply, replySize, "🔄 **Rebooting node** %s...", target);
    } else {
        //snprintf(reply, replySize, "⏩ Ignored (not for this node)");
        reply[0] = '\0';  // не отправлять
    }
}