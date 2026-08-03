#pragma once
#include "concurrency/OSThread.h"

// Фоновый поток для регулярного обновления тренда температуры.
// Работает независимо от запросов /t, обеспечивая точный 10-минутный тренд.
class TelemetryTrendUpdater : public concurrency::OSThread {
public:
    TelemetryTrendUpdater();
    void arm(); // Вызывать один раз в ReplyBotModule::setup()

protected:
    int32_t runOnce() override;

private:
    bool armed;
    void checkAndUpdate();
};

// Глобальный экземпляр
extern TelemetryTrendUpdater telemetryTrendUpdater;
