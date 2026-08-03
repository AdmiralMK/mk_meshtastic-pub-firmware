#pragma once
#include "configuration.h"
#if !MESHTASTIC_EXCLUDE_REPLYBOT

#include "SinglePortModule.h"
#include "concurrency/OSThread.h"
#include "mesh/generated/meshtastic/mesh.pb.h"
#include "ReplyBotEvent/EventRegistry.h"

class ReplyBotModule : public SinglePortModule, private concurrency::OSThread
{
  public:
    ReplyBotModule();
    void setup() override;
    bool wantPacket(const meshtastic_MeshPacket *p) override;
    ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;

  protected:
    int32_t runOnce() override;

  private:
    void sendReply(const meshtastic_MeshPacket &rx, const char *text, CommandCategory category,
                   bool isAdminChannel);

    /** Broadcast текста в primary (для /adm_sendmsg) */
    void sendPrimaryBroadcast(const char *text);

    /** Broadcast текста в канал ADMBOT (для /adm_here) */
    void sendAdmbotBroadcast(const char *text); 
        
    bool isAdminChannel(uint8_t chIndex);

    struct CooldownEntry {
        uint32_t from = 0;
        uint32_t lastMs = 0;
    };
    static constexpr uint8_t COOLDOWN_SLOTS = 16;
    static constexpr uint32_t DM_COOLDOWN_MS = 5 * 1000;
    static constexpr uint32_t BC_COOLDOWN_MS = 5 * 1000;

    CooldownEntry cooldowns[COOLDOWN_SLOTS];
    uint8_t cooldownIdx = 0;
    bool isRateLimited(uint32_t from, bool isDm);
};

#endif