#pragma once
#include <cstdint>

class TelemetryHistory {
public:
    static TelemetryHistory& instance();
    
    void load();
    void save();
    
    // Вызывать при получении команды или периодически
    void update(float currentTemp);
    
    float getMin() const { return minTemp; }
    float getMax() const { return maxTemp; }
    float getHistory(uint8_t index) const;
    uint8_t getHistoryCount() const { return historyCount; }
    
    // Возвращает текстовую стрелку тенденции за последние 10 минут
    // ↑ — температура растёт, ↓ — падает, → — стабильна или нет данных
    const char* getTrendArrow(float currentTemp);

private:
    TelemetryHistory() : minTemp(999.0f), maxTemp(-999.0f), historyCount(0), 
                         lastUpdateMs(0), trendTemp(0.0f), trendTimeMs(0), hasTrendData(false) {}
    
    // Часовая история (6 точек)
    float history[6];
    uint8_t historyCount;
    float minTemp;
    float maxTemp;
    uint32_t lastUpdateMs;
    
    // Данные для тренда за 10 минут
    float trendTemp;       // Температура в момент последнего среза
    uint32_t trendTimeMs;  // Время последнего среза (millis)
    bool hasTrendData;     // Есть ли данные для сравнения
};
