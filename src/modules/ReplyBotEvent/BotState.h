#pragma once
#include <cstdint>

#if defined(ARCH_ESP32)
#include <Preferences.h>
#endif

struct BotFlags {
    bool bot_enabled = true;           // Глобальный флаг (аварийная остановка)
    bool system_pub_enabled = true;    // Служебные в primary broadcast
    bool system_dm_enabled = true;     // Служебные в DM
    bool test_pub_enabled = true;      // Тестовые в primary broadcast
    bool test_dm_enabled = true;       // Тестовые в DM
};

class BotState {
public:
    static BotState& instance() {
        static BotState inst;
        return inst;
    }

    void load() {
#if defined(ARCH_ESP32)
        Preferences prefs;
        if (prefs.begin("replybot", true)) { // read-only
            flags.bot_enabled = prefs.getBool("bot_en", true);
            flags.system_pub_enabled = prefs.getBool("sys_pub", true);
            flags.system_dm_enabled = prefs.getBool("sys_dm", true);
            flags.test_pub_enabled = prefs.getBool("tst_pub", true);
            flags.test_dm_enabled = prefs.getBool("tst_dm", true);
            prefs.end();
        }
#endif
    }

    void save() {
#if defined(ARCH_ESP32)
        Preferences prefs;
        if (prefs.begin("replybot", false)) { // read-write
            prefs.putBool("bot_en", flags.bot_enabled);
            prefs.putBool("sys_pub", flags.system_pub_enabled);
            prefs.putBool("sys_dm", flags.system_dm_enabled);
            prefs.putBool("tst_pub", flags.test_pub_enabled);
            prefs.putBool("tst_dm", flags.test_dm_enabled);
            prefs.end();
        }
#endif
    }

    BotFlags flags;
};