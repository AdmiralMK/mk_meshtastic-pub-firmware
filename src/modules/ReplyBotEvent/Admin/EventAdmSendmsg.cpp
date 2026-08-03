#include "EventAdmSendmsg.h"
#include "NodeDB.h"
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <Arduino.h>

static PendingSendMsg s_pending;

bool eventAdmSendmsg_matches(const char *msg)
{
    return strncmp(msg, "/adm_sendmsg", 12) == 0;
}

bool eventAdmSendmsg_poll(PendingSendMsg &out)
{
    if (!s_pending.active)
        return false;
    if ((int32_t)(millis() - s_pending.sendAtMs) < 0)
        return false;

    out = s_pending;
    s_pending.active = false;
    s_pending.text[0] = '\0';
    return true;
}

void eventAdmSendmsg_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize)
{
    if (!args) {
        snprintf(reply, replySize, "❌ Usage: /adm_sendmsg !id|all <text>");
        return;
    }

    char target[32];
    int consumed = 0;
    if (sscanf(args, "%31s%n", target, &consumed) != 1) {
        snprintf(reply, replySize, "❌ Usage: /adm_sendmsg !id|all <text>");
        return;
    }

    const char *text = args + consumed;
    while (*text == ' ' || *text == '\t')
        text++;
    if (*text == '\0') {
        snprintf(reply, replySize, "❌ Message text is empty");
        return;
    }

    uint32_t ourNode = nodeDB->getNodeNum();
    char ourIdStr[32];
    snprintf(ourIdStr, sizeof(ourIdStr), "!%08x", ourNode);

    if (strcasecmp(target, "all") != 0 && strcasecmp(target, ourIdStr) != 0) {
        snprintf(reply, replySize, "⏩ Ignored (not for this node)");
        return;
    }

    // Уже есть незавершённая задача
    if (s_pending.active) {
        snprintf(reply, replySize, "⏳ Send already queued, try later");
        return;
    }

    // Копируем текст
    size_t len = strlen(text);
    if (len >= sizeof(s_pending.text))
        len = sizeof(s_pending.text) - 1;
    memcpy(s_pending.text, text, len);
    s_pending.text[len] = '\0';

    // all → jitter 1..20 с; точечный !id → сразу
    if (strcasecmp(target, "all") == 0) {
        uint32_t delayMs = (uint32_t)((rand() % 19000) + 1000);
        s_pending.sendAtMs = millis() + delayMs;
        s_pending.active = true;
        snprintf(reply, replySize, "⏳ Queued to primary in %lu ms from %s",
                 (unsigned long)delayMs, target);
    } else {
        s_pending.sendAtMs = millis(); // сразу на следующем poll
        s_pending.active = true;
        snprintf(reply, replySize, "✅ Queued to primary channel from %s", target);
    }
}