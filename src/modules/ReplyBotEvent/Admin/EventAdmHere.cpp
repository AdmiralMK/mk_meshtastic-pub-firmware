#include "EventAdmHere.h"
#include "NodeDB.h"
#include "PowerStatus.h"
#include <Arduino.h>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cstdlib>

static PendingHereMsg s_pending;

bool eventAdmHere_matches(const char *msg)
{
    return strncmp(msg, "/adm_here", 9) == 0;
}

bool eventAdmHere_poll(PendingHereMsg &out)
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

void eventAdmHere_handle(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize)
{
    (void)mp;
    (void)args;

    // Не шлём немедленный reply — только очередь (иначе будет двойная отправка)
    reply[0] = '\0';

    if (s_pending.active) {
        // Уже ждём отправку своего /adm_here — второй раз не ставим
        return;
    }

    const char *longName = owner.long_name;
    if (!longName || longName[0] == '\0')
        longName = "Unknown";

    const char *shortName = owner.short_name;
    if (!shortName || shortName[0] == '\0')
        shortName = "????";

    float volts = 0.0f;
    if (powerStatus) {
        int mv = powerStatus->getBatteryVoltageMv();
        if (mv > 0)
            volts = mv / 1000.0f;
    }

    uint32_t ms = millis();
    uint32_t sec = ms / 1000;
    uint32_t min = sec / 60;
    sec %= 60;
    uint32_t hour = min / 60;
    min %= 60;
    uint32_t day = hour / 24;
    hour %= 24;

    uint32_t ourNode = nodeDB->getNodeNum();

    snprintf(s_pending.text, sizeof(s_pending.text),
             "🙋 **I'm here, online!**\n"
             "📟 %s (%s)\n"
             "🆔 ID: !%08x\n"
             "🔋 Volt: %.2fV\n"
             "⏱️ Uptime: %lud %luh %lum %lus",
             longName, shortName, (unsigned int)ourNode, volts, (unsigned long)day, (unsigned long)hour,
             (unsigned long)min, (unsigned long)sec);

    // Jitter 500..5000 ms, чтобы ноды не били в эфир одновременно
    uint32_t delayMs = (uint32_t)((rand() % 4501) + 500);
    s_pending.sendAtMs = millis() + delayMs;
    s_pending.active = true;

}