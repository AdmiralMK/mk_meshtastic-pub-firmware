#include "EventHard.h"
#include "PowerStatus.h"
#include "NodeDB.h"
#include "memGet.h"
#include "RTC.h"
#include <Arduino.h>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <ctime>

#ifdef ARCH_ESP32
#include "esp_system.h"
#include "esp_rom_sys.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

bool eventHard_matches(const char *msg)
{
    return strncmp(msg, "/hard", 5) == 0 && (msg[5] == '\0' || isspace((unsigned char)msg[5]));
}

void eventHard_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize)
{
    // --- Last reset ---
    const char *lastEvent = "Unknown";
#ifdef ARCH_ESP32
    switch (esp_reset_reason()) {
    case ESP_RST_POWERON:
        lastEvent = "Power-on/Reset";
        break;
    case ESP_RST_EXT:
        lastEvent = "External";
        break;
    case ESP_RST_SW:
        lastEvent = "Software";
        break;
    case ESP_RST_PANIC:
        lastEvent = "Panic";
        break;
    case ESP_RST_INT_WDT:
        lastEvent = "Int WDT";
        break;
    case ESP_RST_TASK_WDT:
        lastEvent = "Task WDT";
        break;
    case ESP_RST_WDT:
        lastEvent = "WDT";
        break;
    case ESP_RST_DEEPSLEEP:
        lastEvent = "Deep sleep";
        break;
    case ESP_RST_BROWNOUT:
        lastEvent = "Brownout";
        break;
    case ESP_RST_SDIO:
        lastEvent = "SDIO";
        break;
    default:
        break;
    }
#endif

    // --- Volt ---
    // --- getBatteryChargePercent() специально ограничен 0…100 ---
    // float volts = 0.0f;
    // int percent = 0;
    // if (powerStatus) {
    //     volts = powerStatus->getBatteryVoltageMv() / 1000.0f;
    //     percent = powerStatus->getBatteryChargePercent();
    // }

    // --- Volt ---
    // 5.00 V = 100%, без ограничения сверху/снизу
    float volts = 0.0f;
    int percent = 0;
    if (powerStatus) {
        int mv = powerStatus->getBatteryVoltageMv();
        volts = mv / 1000.0f;
        if (mv > 0) {
            percent = (int)(((long)mv * 100L) / 5000L);
            // 5.00 V → 100%, 5.25 V → 105%, 4.00 V → 80%
        }
    }

    // --- Heap ---
    uint32_t freeHeap = memGet.getFreeHeap();
    uint32_t totalHeap = memGet.getHeapSize();
    uint32_t freeKB = freeHeap / 1024;
    uint32_t totalKB = totalHeap / 1024;
    int heapPct = (totalHeap > 0) ? (int)((freeHeap * 100) / totalHeap) : 0;

    // --- PSRAM (занято / всего) ---
    uint32_t freePsram = memGet.getFreePsram();
    uint32_t totalPsram = memGet.getPsramSize();
    uint32_t usedPsram = (totalPsram > freePsram) ? (totalPsram - freePsram) : 0;
    float usedPsramMB = usedPsram / (1024.0f * 1024.0f);
    float totalPsramMB = totalPsram / (1024.0f * 1024.0f);
    int psramPct = (totalPsram > 0) ? (int)((usedPsram * 100) / totalPsram) : 0;

    // --- CPU / Tasks ---
    unsigned long cpuMhz = 0;
    unsigned long tasks = 0;
#ifdef ARCH_ESP32
    cpuMhz = (unsigned long)ets_get_cpu_frequency();
    tasks = (unsigned long)uxTaskGetNumberOfTasks();
#endif

    // --- Uptime (как EventUptime) ---
    uint32_t ms = millis();
    uint32_t s = ms / 1000;
    uint32_t m = s / 60;
    s %= 60;
    uint32_t h = m / 60;
    m %= 60;
    uint32_t d = h / 24;
    h %= 24;

    // --- Time (как EventTime) ---
    int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;
    uint32_t nowSec = getValidTime(RTCQualityNTP);
    if (nowSec > 0) {
        time_t t = (time_t)nowSec;
        struct tm *tm_info = gmtime(&t);
        if (tm_info) {
            year = tm_info->tm_year + 1900;
            mon = tm_info->tm_mon + 1;
            day = tm_info->tm_mday;
            hour = tm_info->tm_hour;
            min = tm_info->tm_min;
            sec = tm_info->tm_sec;
        }
    }

    if (nowSec > 0) {
        snprintf(reply, replySize,
            "⚙️ **HARDWARE:**\n"
            "Heap: %uKB/%uKB (%d%%)\n"
            "PSRAM: %.2f/%.2f MB (%d%%)\n"
            "CPU: %luMHz, Tasks: %lu\n"
            "Volt: %.2fV (%d%%)\n"
            "Last: %s\n"
            "Reboots: %lu\n"
            "Uptime: %lud %luh %lum %lus\n"
            "Time: %04d-%02d-%02d %02d:%02d:%02d UTC",
            freeKB, totalKB, heapPct, 
            usedPsramMB, totalPsramMB, psramPct, 
            cpuMhz, tasks,           
            volts, percent, 
            lastEvent, 
            (unsigned long)myNodeInfo.reboot_count,
            (unsigned long)d, (unsigned long)h, (unsigned long)m, (unsigned long)s,
            year, mon, day, hour, min, sec
            );
    } else {
        // Нет GPS/NTP
        snprintf(reply, replySize,
            "⚙️ **HARDWARE:**\n"
            "Heap: %uKB/%uKB (%d%%)\n"
            "PSRAM: %.2f/%.2f MB (%d%%)\n"
            "CPU: %luMHz, Tasks: %lu\n"
            "Volt: %.2fV (%d%%)\n"
            "Last: %s\n"
            "Reboots: %lu\n"
            "Uptime: %lud %luh %lum %lus\n"
            "Time: not set (no GPS/NTP)",
            freeKB, totalKB, heapPct,
            usedPsramMB, totalPsramMB, psramPct,
            cpuMhz, tasks,
            volts, percent,
            lastEvent,
            (unsigned long)myNodeInfo.reboot_count,
            (unsigned long)d, (unsigned long)h, (unsigned long)m, (unsigned long)s);
    }
}