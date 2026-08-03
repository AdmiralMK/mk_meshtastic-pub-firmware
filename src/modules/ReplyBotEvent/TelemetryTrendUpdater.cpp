#include "TelemetryTrendUpdater.h"
#include "System/TelemetryHistory.h"
#include "NodeDB.h"
#include <Arduino.h>
#include <climits>

// Интервал проверки: 60 секунд (1 минута)
static constexpr uint32_t CHECK_INTERVAL_MS = 60000;

TelemetryTrendUpdater::TelemetryTrendUpdater() 
    : concurrency::OSThread("TeleTrend"), armed(false) {}

void TelemetryTrendUpdater::arm() {
    if (armed) return;
    armed = true;
    // Первый вызов runOnce() через 60 секунд после старта
    setInterval(CHECK_INTERVAL_MS);
}

int32_t TelemetryTrendUpdater::runOnce() {
    if (!armed) return INT32_MAX;
    
    checkAndUpdate();
    
    // Продолжать работать каждую минуту (бесконечный цикл)
    return CHECK_INTERVAL_MS;
}

void TelemetryTrendUpdater::checkAndUpdate() {
    // Читаем текущую температуру из nodeDB
    auto *node = nodeDB->getNode(nodeDB->getNodeNum());
    if (!node || !node->has_environment_metrics) return;
    
    const auto &env = node->environment_metrics;
    if (!env.has_temperature) return;
    
    float temp = env.temperature;
    
    // Валидация и обновление истории/тренда
    if (temp > -50.0f && temp < 100.0f) {
        TelemetryHistory::instance().update(temp);
    }
}

// Создание глобального экземпляра
TelemetryTrendUpdater telemetryTrendUpdater;
