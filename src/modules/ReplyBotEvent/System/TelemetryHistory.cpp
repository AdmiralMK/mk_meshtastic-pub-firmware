#include "TelemetryHistory.h"
#include <Arduino.h>

#if defined(ARCH_ESP32)
#include <Preferences.h>
#endif

TelemetryHistory& TelemetryHistory::instance() {
    static TelemetryHistory inst;
    return inst;
}

void TelemetryHistory::load() {
#if defined(ARCH_ESP32)
    Preferences prefs;
    if (prefs.begin("tele_hist", true)) {
        minTemp = prefs.getFloat("min", 999.0f);
        maxTemp = prefs.getFloat("max", -999.0f);
        historyCount = prefs.getUChar("count", 0);
        lastUpdateMs = prefs.getUInt("last_ms", 0);
        for (uint8_t i = 0; i < 6; i++) {
            char key[10];
            snprintf(key, sizeof(key), "h%d", i);
            history[i] = prefs.getFloat(key, 999.0f);
        }
        prefs.end();
    }
#endif
    // Тренд НЕ загружаем из NVS, так как millis() сбрасывается при перезагрузке
    trendTemp = 0.0f;
    trendTimeMs = 0;
    hasTrendData = false;
}

void TelemetryHistory::save() {
#if defined(ARCH_ESP32)
    Preferences prefs;
    if (prefs.begin("tele_hist", false)) {
        prefs.putFloat("min", minTemp);
        prefs.putFloat("max", maxTemp);
        prefs.putUChar("count", historyCount);
        prefs.putUInt("last_ms", lastUpdateMs);
        for (uint8_t i = 0; i < 6; i++) {
            char key[10];
            snprintf(key, sizeof(key), "h%d", i);
            prefs.putFloat(key, history[i]);
        }
        prefs.end();
    }
#endif
}

void TelemetryHistory::update(float currentTemp) {
    // Валидация данных
    if (currentTemp > -50.0f && currentTemp < 100.0f) {
        if (currentTemp < minTemp) minTemp = currentTemp;
        if (currentTemp > maxTemp) maxTemp = currentTemp;
    }

    uint32_t now = millis();
    
    // Часовая история (сдвиг массива раз в час)
    if (lastUpdateMs == 0 || (now - lastUpdateMs) >= 3600000) { // 1 час
        for (int i = 5; i > 0; i--) {
            history[i] = history[i - 1];
        }
        history[0] = currentTemp;
        if (historyCount < 6) historyCount++;
        
        lastUpdateMs = now;
        save();
    }
    
    // Тренд за 10 минут (обновляется фоновым потоком или командой /t)
    if (!hasTrendData) {
        trendTemp = currentTemp;
        trendTimeMs = now;
        hasTrendData = true;
    } else if ((now - trendTimeMs) >= 600000) { // Прошло 10 минут или больше
        trendTemp = currentTemp;
        trendTimeMs = now;
    }
}

// Текстовые стрелки вместо эмодзи, порог 1.0°C
const char* TelemetryHistory::getTrendArrow(float currentTemp) {
    // UTF-8 текстовые стрелки (по 3 байта каждая):
    // ↑ = \xE2\x86\x91 (U+2191)
    // ↓ = \xE2\x86\x93 (U+2193)
    // → = \xE2\x86\x92 (U+2192)
    
    // Если нет текущих данных или нет данных для сравнения
    if (currentTemp > 100.0f || currentTemp < -50.0f || !hasTrendData) {
        return "\xE2\x86\x92"; // →
    }
    
    uint32_t now = millis();
    // Если данные устарели (прошло больше 20 минут с последнего среза)
    if ((now - trendTimeMs) > 1200000) {
        return "\xE2\x86\x92"; // →
    }
    
    float diff = currentTemp - trendTemp;
    
    // Порог 1.0°C
    if (diff > 1.0f)  return "\xE2\x86\x91"; // ↑ растёт
    if (diff < -1.0f) return "\xE2\x86\x93"; // ↓ падает
    
    return "\xE2\x86\x92"; // → стабильна
}

float TelemetryHistory::getHistory(uint8_t index) const {
    if (index < 6 && index < historyCount) {
        return history[index];
    }
    return 999.0f;
}
