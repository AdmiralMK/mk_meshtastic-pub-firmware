#pragma once
#include "mesh/MeshTypes.h"

// Категория команды
enum class CommandCategory {
    ADMIN,      // Административная (только ADMBOT)
    SYSTEM,     // Служебная (со слэшем, ответ в DM/ADMBOT)
    PUBLIC      // Тестовая (без слэша, ответ в тот же канал)
};

// Структура обычной команды (служебная или публичная)
struct ReplyBotCommand {
    const char* command;                    // Имя команды
    CommandCategory category;               // Категория
    bool (*matches)(const char *msg);       // Проверка совпадения
    void (*handle)(const meshtastic_MeshPacket &mp, char *reply, size_t replySize);
};

// Структура админской команды
struct ReplyBotAdminCommand {
    const char* command;                    // Имя команды
    bool (*matches)(const char *msg);       // Проверка совпадения
    void (*handle)(const meshtastic_MeshPacket &mp, const char *args, char *reply, size_t replySize);
};

// Реестры
const ReplyBotCommand* getSystemRegistry();
size_t getSystemRegistrySize();

const ReplyBotCommand* getPublicRegistry();
size_t getPublicRegistrySize();

const ReplyBotAdminCommand* getAdminRegistry();
size_t getAdminRegistrySize();