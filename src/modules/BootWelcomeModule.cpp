#include "BootWelcomeModule.h"
#include "Channels.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "PowerStatus.h"
#include "memGet.h"
#include "meshUtils.h"
#include "main.h"

#include <cstdio>
#include <cstring>

BootWelcomeModule *bootWelcomeModule;

BootWelcomeModule::BootWelcomeModule()
    : SinglePortModule("bootwelcome", meshtastic_PortNum_TEXT_MESSAGE_APP),
      concurrency::OSThread("BootWelcome")
{
    // Первая попытка через ~15 с после старта (радио и каналы уже подняты)
    setIntervalFromNow(15 * 1000);
}

int32_t BootWelcomeModule::runOnce()
{
    if (sent) {
        return INT32_MAX; // больше не запускать
    }

    if (!service || !nodeDB) {
        return 5000; // ещё рано — повторить через 5 с
    }

    // Ищем канал ADMBOT. Если нет — ничего не шлём.
    // Важно: getByName() при отсутствии имени возвращает primary,
    // поэтому обязательно сверяем реальное имя канала.
    meshtastic_Channel &ch = channels.getByName("ADMBOT");
    if (ch.index < 0 || !ch.has_settings || ch.role == meshtastic_Channel_Role_DISABLED ||
        strcasecmp(ch.settings.name, "ADMBOT") != 0) {
        LOG_INFO("BootWelcome: channel ADMBOT not present — skip");
        sent = true;
        return INT32_MAX;
    }

    ChannelIndex chIndex = ch.index;

    // Батарея
    float volts = 0.0f;
    int percent = 0;
    if (powerStatus) {
        volts = powerStatus->getBatteryVoltageMv() / 1000.0f;
        percent = powerStatus->getBatteryChargePercent();
    }

    // Heap
    uint32_t freeHeap = memGet.getFreeHeap();
    uint32_t totalHeap = memGet.getHeapSize();
    uint32_t freeKB = freeHeap / 1024;
    uint32_t totalKB = totalHeap / 1024;
    int heapPct = (totalHeap > 0) ? (int)((freeHeap * 100) / totalHeap) : 0;

    // Причина старта
    const char *lastEvent = "Power/Reset";
#ifdef ARCH_ESP32
    switch (esp_reset_reason()) {
    case ESP_RST_POWERON:
        lastEvent = "Power/Reset";
        break;
    case ESP_RST_SW:
        lastEvent = "Software reset";
        break;
    case ESP_RST_PANIC:
        lastEvent = "Panic";
        break;
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
        lastEvent = "Watchdog";
        break;
    case ESP_RST_BROWNOUT:
        lastEvent = "Brownout";
        break;
    case ESP_RST_DEEPSLEEP:
        lastEvent = "Deep-sleep wake";
        break;
    default:
        lastEvent = "Reset";
        break;
    }
#endif

    // Long name + short name
    const char *longName = owner.long_name;
    if (!longName || longName[0] == '\0') {
        longName = "Unknown";
    }

    const char *shortName = owner.short_name;
    if (!shortName || shortName[0] == '\0') {
        shortName = "????";
    }

    // PSRAM: занято / всего в MB
    uint32_t freePsram = memGet.getFreePsram();
    uint32_t totalPsram = memGet.getPsramSize();
    uint32_t usedPsram = (totalPsram > freePsram) ? (totalPsram - freePsram) : 0;
    float usedPsramMB = usedPsram / (1024.0f * 1024.0f);
    float totalPsramMB = totalPsram / (1024.0f * 1024.0f);
    int psramPct = (totalPsram > 0) ? (int)((usedPsram * 100) / totalPsram) : 0;

    char msg[meshtastic_Constants_DATA_PAYLOAD_LEN];
    
#if defined(BOOT_WELCOME_MARKDOWN) && BOOT_WELCOME_MARKDOWN
    // Для новых Android-приложений (markdown: **жирный**, *курсив*)
    snprintf(msg, sizeof(msg),
             "ℹ️ **STARTED** *%s (%s)*:\n"
             " - 📝 **Last:** %s;\n"
             " - 🔋 **Volt:** %.2fV (%d%%);\n"
             " - 💾 **Heap:** %uKB/%uKB (%d%%);\n"
             " - 📦 **PSRAM:** %.2f/%.2f MB (%d%%).",
             longName, shortName, lastEvent, volts, percent,
             freeKB, totalKB, heapPct,
             usedPsramMB, totalPsramMB, psramPct);
#else
    // Без markdown — совместимость со старыми клиентами
    snprintf(msg, sizeof(msg),
             "ℹ️ STARTED %s (%s):\n"
             " - 📝 Last: %s;\n"
             " - 🔋 Volt: %.2fV (%d%%);\n"
             " - 💾 Heap: %uKB/%uKB (%d%%);\n"
             " - 📦 PSRAM: %.2f/%.2f MB (%d%%).",
             longName, shortName, lastEvent, volts, percent,
             freeKB, totalKB, heapPct,
             usedPsramMB, totalPsramMB, psramPct);
#endif

    meshtastic_MeshPacket *p = allocDataPacket();
    if (!p) {
        LOG_WARN("BootWelcome: no packet from pool, retry later");
        return 10000;
    }

    p->to = NODENUM_BROADCAST;
    p->channel = chIndex;
    p->want_ack = false;
    p->decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
    p->decoded.payload.size = strlen(msg);
    memcpy(p->decoded.payload.bytes, msg, p->decoded.payload.size);
    p->priority = meshtastic_MeshPacket_Priority_BACKGROUND;

    service->sendToMesh(p, RX_SRC_LOCAL, true);
    LOG_INFO("BootWelcome sent on channel %u: %s", chIndex, msg);

    sent = true;
    return INT32_MAX;
}