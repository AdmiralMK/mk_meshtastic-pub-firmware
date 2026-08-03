#include "EventTelemetry.h"
#include "TelemetryHistory.h"
#include "NodeDB.h"
#include "mesh/generated/meshtastic/telemetry.pb.h"
#include <cstring>
#include <cstdio>
#include <cctype>

// Символы для sparkline (от низкого к высокому)
static const char* SPARK_CHARS[] = {
    "\xE2\x96\x81",  // ▁
    "\xE2\x96\x82",  // ▂
    "\xE2\x96\x83",  // ▃
    "\xE2\x96\x84",  // ▄
    "\xE2\x96\x85",  // ▅
    "\xE2\x96\x86",  // ▆
    "\xE2\x96\x87",  // ▇
    "\xE2\x96\x88"   // █
};
static const int SPARK_LEVELS = 8;

bool eventTelemetry_matches(const char *msg) {
    return (strcmp(msg, "/t") == 0 || 
            strcmp(msg, "/tele") == 0 || 
            strcmp(msg, "/telemetry") == 0);
}

// Построение sparkline-графика
static void buildSparkline(char *out, size_t outSize, const float *values, uint8_t count) {
    if (count == 0) {
        snprintf(out, outSize, "N/A");
        return;
    }
    
    float minV = values[0], maxV = values[0];
    for (uint8_t i = 1; i < count; i++) {
        if (values[i] < minV) minV = values[i];
        if (values[i] > maxV) maxV = values[i];
    }
    
    out[0] = '\0';
    float range = maxV - minV;
    
    for (uint8_t i = 0; i < count; i++) {
        int level;
        if (range < 0.1f) {
            level = 3; // Если все значения почти одинаковые
        } else {
            level = (int)((values[i] - minV) / range * (SPARK_LEVELS - 1));
            if (level < 0) level = 0;
            if (level >= SPARK_LEVELS) level = SPARK_LEVELS - 1;
        }
        if (strlen(out) + 4 >= outSize) break;
        strcat(out, SPARK_CHARS[level]);
    }
}

void eventTelemetry_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize) {
    auto *node = nodeDB->getNode(nodeDB->getNodeNum());
    if (!node) {
        snprintf(reply, replySize, "❌ Node data not found");
        return;
    }

    TelemetryHistory::instance().load();
    
    char tempNowStr[16] = "N/A";
    char humidStr[16] = "N/A";
    char illumStr[16] = "N/A";
    
    float currentTemp = 999.0f;

    // Обработка Environment Metrics
    if (node->has_environment_metrics) {
        const auto &env = node->environment_metrics;
        
        if (env.has_temperature) {
            currentTemp = env.temperature;
            snprintf(tempNowStr, sizeof(tempNowStr), "%+0.1f°C", currentTemp);
            TelemetryHistory::instance().update(currentTemp);
        }
        
        if (env.has_relative_humidity) {
            snprintf(humidStr, sizeof(humidStr), "%u%%", (uint32_t)env.relative_humidity);
        }
        
        if (env.has_light_lux) {
            snprintf(illumStr, sizeof(illumStr), "%u Lux", (uint32_t)env.light_lux);
        }
    }

    // Собираем историю от СТАРЫХ к НОВЫМ
    uint8_t count = TelemetryHistory::instance().getHistoryCount();
    float histValues[6];
    char histValuesStr[64] = "N/A";
    char sparkline[32] = "N/A";
    
    if (count > 0) {
        for (uint8_t i = 0; i < count; i++) {
            histValues[i] = TelemetryHistory::instance().getHistory(count - 1 - i);
        }
        
        buildSparkline(sparkline, sizeof(sparkline), histValues, count);
        
        char tempBuf[64] = "";
        for (uint8_t i = 0; i < count; i++) {
            char chunk[12];
            snprintf(chunk, sizeof(chunk), "%+0.0f", histValues[i]);
            strcat(tempBuf, chunk);
            if (i < count - 1) strcat(tempBuf, ";");
        }
        snprintf(histValuesStr, sizeof(histValuesStr), "%s°C", tempBuf);
    }

    // Мин/Макс температура за 24 часа
    char minStr[16] = "N/A";
    char maxStr[16] = "N/A";
    float minT = TelemetryHistory::instance().getMin();
    float maxT = TelemetryHistory::instance().getMax();
    if (minT != 999.0f) snprintf(minStr, sizeof(minStr), "%+0.0f°C", minT);
    if (maxT != -999.0f) snprintf(maxStr, sizeof(maxStr), "%+0.0f°C", maxT);

    // Получаем стрелку тенденции за последние 10 минут
    const char* trendArrow = TelemetryHistory::instance().getTrendArrow(currentTemp);

    // Финальная сборка сообщения (Вариант A)
    snprintf(reply, replySize, 
        "🌡️ Temp now: %s\n"
        "📈 Hist 6h: %s %s\n"
        "📊 Val temp: %s\n"
        "❄️ Min 24h: %s | 🔥 Max 24h: %s\n"
        "💧 Humid: %s | 💡 Illum: %s",
        tempNowStr, sparkline, trendArrow, histValuesStr, minStr, maxStr, humidStr, illumStr
    );
}
