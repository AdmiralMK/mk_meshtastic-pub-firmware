#include "configuration.h"
#if !MESHTASTIC_EXCLUDE_REPLYBOT

#include "Channels.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "ReplyBotModule.h"
#include "ReplyBotEvent/BotState.h"
#include "ReplyBotEvent/EventRegistry.h"
#include "ReplyBotEvent/Admin/EventAdmReboot.h"
#include "ReplyBotEvent/Admin/EventAdmSendmsg.h"
#include "ReplyBotEvent/Admin/EventAdmHere.h"
#include "mesh/MeshTypes.h"
#include "ReplyBotEvent/TelemetryTrendUpdater.h"
#include <Arduino.h>
#include <cctype>
#include <cstring>

#ifdef ARCH_ESP32
extern uint32_t rebootAtMsec;
#endif

// ============================================================================
// Конструктор и базовые методы
// ============================================================================
ReplyBotModule::ReplyBotModule()
    : SinglePortModule("replybot", meshtastic_PortNum_TEXT_MESSAGE_APP), concurrency::OSThread("ReplyBot")
{
    isPromiscuous = true;
    setIntervalFromNow(500); // опрос очереди /adm_sendmsg
}

void ReplyBotModule::setup()
{
    BotState::instance().load();
    telemetryTrendUpdater.arm(); // <-- ДОБАВЛЕНО: запуск фонового потока тренда
}

bool ReplyBotModule::wantPacket(const meshtastic_MeshPacket *p)
{
    return (p && p->decoded.portnum == ourPortNum);
}

int32_t ReplyBotModule::runOnce()
{
    PendingSendMsg job;
    if (eventAdmSendmsg_poll(job)) {
        sendPrimaryBroadcast(job.text);
    }

    PendingHereMsg hereJob;
    if (eventAdmHere_poll(hereJob)) {
        sendAdmbotBroadcast(hereJob.text);
    }

    return 500; // снова через 0.5 с
}

// ============================================================================
// Проверка канала ADMBOT
// ============================================================================
bool ReplyBotModule::isAdminChannel(uint8_t chIndex)
{
    const char *name = channels.getName(chIndex);
    if (!name || name[0] == '\0')
        return false;
    return strcasecmp(name, "ADMBOT") == 0;
}

// ============================================================================
// Rate limiting (кулдаун)
// ============================================================================
bool ReplyBotModule::isRateLimited(uint32_t from, bool isDm)
{
    uint32_t now = millis();
    uint32_t limit = isDm ? DM_COOLDOWN_MS : BC_COOLDOWN_MS;
    for (auto &e : cooldowns) {
        if (e.from == from) {
            if ((uint32_t)(now - e.lastMs) < limit)
                return true;
            e.lastMs = now;
            return false;
        }
    }
    cooldowns[cooldownIdx].from = from;
    cooldowns[cooldownIdx].lastMs = now;
    cooldownIdx = (cooldownIdx + 1) % COOLDOWN_SLOTS;
    return false;
}

// ============================================================================
// Основной обработчик входящих сообщений
// ============================================================================
ProcessMessage ReplyBotModule::handleReceived(const meshtastic_MeshPacket &mp)
{
    const uint32_t ourNode = nodeDB->getNodeNum();
    const bool isDM = (mp.to == ourNode);
    const bool isPrimaryBC = (mp.channel == channels.getPrimaryIndex()) && isBroadcast(mp.to);
    const bool isAdmCh = isAdminChannel(mp.channel);

    // 1. Базовая фильтрация каналов
    if (!isDM && !isPrimaryBC && !isAdmCh)
        return ProcessMessage::CONTINUE;
    if (mp.decoded.payload.size == 0)
        return ProcessMessage::CONTINUE;

    char buf[260];
    size_t n = mp.decoded.payload.size;
    if (n > sizeof(buf) - 1)
        n = sizeof(buf) - 1;
    memcpy(buf, mp.decoded.payload.bytes, n);
    buf[n] = '\0';

    const char *msg = buf;
    while (*msg == ' ' || *msg == '\t')
        msg++;
    if (*msg == '\0')
        return ProcessMessage::CONTINUE;

    // ========================================================================
    // 2. АДМИНСКИЕ КОМАНДЫ (только из ADMBOT, без кулдауна)
    // ========================================================================
    if (isAdmCh && msg[0] == '/') {
        const auto *reg = getAdminRegistry();
        size_t count = getAdminRegistrySize();
        for (size_t i = 0; i < count; i++) {
            if (strncmp(msg, reg[i].command, strlen(reg[i].command)) == 0) {
                size_t cmdLen = strlen(reg[i].command);
                if (msg[cmdLen] == '\0' || msg[cmdLen] == ' ' || msg[cmdLen] == '\t') {
                    char reply[meshtastic_Constants_DATA_PAYLOAD_LEN] = {0};
                    const char *args = (msg[cmdLen] != '\0') ? (msg + cmdLen + 1) : nullptr;
                    while (args && (*args == ' ' || *args == '\t'))
                        args++;

                    reg[i].handle(mp, args, reply, sizeof(reply));
                    if (reply[0] != '\0') {
                        sendReply(mp, reply, CommandCategory::ADMIN, true);
                    }
                    // /adm_reboot: ответ уже в очереди mesh — рестарт через ~2 с
                    if (eventAdmReboot_consumePending()) {
#ifdef ARCH_ESP32
                        rebootAtMsec = millis() + 2000;
#endif
                    }
                    return ProcessMessage::CONTINUE;
                }
            }
        }
    }

    // ========================================================================
    // 3. Глобальная проверка включения бота
    // ========================================================================
    if (!BotState::instance().flags.bot_enabled)
        return ProcessMessage::CONTINUE;

    // ========================================================================
    // 4. СЛУЖЕБНЫЕ КОМАНДЫ (начинаются с /)
    // ========================================================================
    if (msg[0] == '/') {
        bool sysAllowed =
            isAdmCh ? true
                    : (isDM ? BotState::instance().flags.system_dm_enabled : BotState::instance().flags.system_pub_enabled);
        if (sysAllowed) {
            const auto *reg = getSystemRegistry();
            size_t count = getSystemRegistrySize();
            for (size_t i = 0; i < count; i++) {
                if (reg[i].matches(msg)) {
                    if (!isAdmCh && isRateLimited(mp.from, isDM))
                        return ProcessMessage::CONTINUE;

                    char reply[meshtastic_Constants_DATA_PAYLOAD_LEN] = {0};
                    reg[i].handle(mp, reply, sizeof(reply));
                    if (reply[0] != '\0')
                        sendReply(mp, reply, CommandCategory::SYSTEM, isAdmCh);
                    return ProcessMessage::CONTINUE;
                }
            }
        }
    }
    // ========================================================================
    // 5. ПУБЛИЧНЫЕ КОМАНДЫ (без /, регистронезависимо, начало строки)
    // ========================================================================
    else {
        bool tstAllowed =
            isAdmCh ? true
                    : (isDM ? BotState::instance().flags.test_dm_enabled : BotState::instance().flags.test_pub_enabled);
        if (tstAllowed) {
            const auto *reg = getPublicRegistry();
            size_t count = getPublicRegistrySize();
            for (size_t i = 0; i < count; i++) {
                if (reg[i].matches(msg)) {
                    if (!isAdmCh && isRateLimited(mp.from, isDM))
                        return ProcessMessage::CONTINUE;

                    char reply[meshtastic_Constants_DATA_PAYLOAD_LEN] = {0};
                    reg[i].handle(mp, reply, sizeof(reply));
                    if (reply[0] != '\0')
                        sendReply(mp, reply, CommandCategory::PUBLIC, isAdmCh);
                    return ProcessMessage::CONTINUE;
                }
            }
        }
    }

    return ProcessMessage::CONTINUE;
}

// ============================================================================
// Broadcast в primary (/adm_sendmsg)
// ============================================================================
void ReplyBotModule::sendPrimaryBroadcast(const char *text)
{
    if (!text || text[0] == '\0' || !service)
        return;

    meshtastic_MeshPacket *p = allocDataPacket();
    if (!p)
        return;

    p->to = NODENUM_BROADCAST;
    p->channel = channels.getPrimaryIndex();
    p->want_ack = false;
    p->decoded.want_response = false;
    p->decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;

    size_t len = strlen(text);
    if (len > sizeof(p->decoded.payload.bytes))
        len = sizeof(p->decoded.payload.bytes);
    p->decoded.payload.size = len;
    memcpy(p->decoded.payload.bytes, text, len);

    service->sendToMesh(p);
    LOG_INFO("ReplyBot: adm_sendmsg broadcast (%u bytes)", (unsigned)len);
}

void ReplyBotModule::sendAdmbotBroadcast(const char *text)
{
    if (!text || text[0] == '\0' || !service)
        return;

    // Ищем индекс канала ADMBOT
    int admIdx = -1;
    for (int i = 0; i < channels.getNumChannels(); i++) {
        if (isAdminChannel((uint8_t)i)) {
            admIdx = i;
            break;
        }
    }

    /*
        Возможная проблема при сборке!
        channels.getNumChannels() в части веток Meshtastic может отсутствовать. Если компилятор ругается, замените цикл на:

        C++for (int i = 0; i < MAX_NUM_CHANNELS; i++) {
                if (isAdminChannel((uint8_t)i)) {
                    admIdx = i;
                    break;
                }
            }

        или, если константы нет:
        C++for (int i = 0; i < 8; i++) {  // типичный лимит каналов
    */


    if (admIdx < 0) {
        LOG_WARN("ReplyBot: /adm_here — channel ADMBOT not found");
        return;
    }

    meshtastic_MeshPacket *p = allocDataPacket();
    if (!p)
        return;

    p->to = NODENUM_BROADCAST;
    p->channel = (uint8_t)admIdx;
    p->want_ack = false;
    p->decoded.want_response = false;
    p->decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;

    size_t len = strlen(text);
    if (len > sizeof(p->decoded.payload.bytes))
        len = sizeof(p->decoded.payload.bytes);
    p->decoded.payload.size = len;
    memcpy(p->decoded.payload.bytes, text, len);

    service->sendToMesh(p);
    LOG_INFO("ReplyBot: adm_here broadcast (%u bytes) ch=%d", (unsigned)len, admIdx);
}


// ============================================================================
// Отправка ответа с учётом категории и канала
// ============================================================================
void ReplyBotModule::sendReply(const meshtastic_MeshPacket &rx, const char *text, CommandCategory category,
                               bool isAdminChannel)
{
    if (!text || text[0] == '\0')
        return;

    meshtastic_MeshPacket *p = allocDataPacket();
    if (!p)
        return;

    p->want_ack = false;
    p->decoded.want_response = false;

    size_t len = strlen(text);
    if (len > sizeof(p->decoded.payload.bytes))
        len = sizeof(p->decoded.payload.bytes);
    p->decoded.payload.size = len;
    memcpy(p->decoded.payload.bytes, text, len);

    if (isAdminChannel) {
        // Ответ инициатору в ADMBOT
        p->to = NODENUM_BROADCAST;
        p->channel = rx.channel;
    } else if (category == CommandCategory::SYSTEM) {
        // Служебные — всегда в DM
        p->to = rx.from;
        p->channel = rx.channel;
    } else if (category == CommandCategory::PUBLIC) {
        if (isBroadcast(rx.to)) {
            p->to = NODENUM_BROADCAST;
            p->channel = channels.getPrimaryIndex();
        } else {
            p->to = rx.from;
            p->channel = rx.channel;
        }
        p->decoded.reply_id = rx.id;
    }

    service->sendToMesh(p);
}

#endif // MESHTASTIC_EXCLUDE_REPLYBOT
