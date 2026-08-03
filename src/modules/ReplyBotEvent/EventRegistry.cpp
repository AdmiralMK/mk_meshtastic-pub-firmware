#include "EventRegistry.h"

// --- ADMIN ---
#include "Admin/EventAdmReboot.h"
#include "Admin/EventAdmSendmsg.h"
#include "Admin/EventAdmCmdsys.h"
#include "Admin/EventAdmCmdtst.h"
#include "Admin/EventAdmBotOff.h"
#include "Admin/EventAdmBotOn.h"
#include "Admin/EventAdmStatus.h"
#include "Admin/EventAdmHere.h"

// --- SYSTEM ---
#include "System/EventNodes.h"
#include "System/EventMesh.h"
#include "System/EventPing.h"
#include "System/EventHello.h"
#include "System/EventTest.h"
#include "System/EventHelp.h"
#include "System/EventHard.h"

// --- PUBLIC ---
#include "Public/EventPubPing.h"
#include "Public/EventPubTest.h"
#include "Public/EventPubHello.h"

// ========== ADMIN REGISTRY ==========
static const ReplyBotAdminCommand adminRegistry[] = {
    { "/adm_reboot",   eventAdmReboot_matches,   eventAdmReboot_handle },
    { "/adm_sendmsg",  eventAdmSendmsg_matches,  eventAdmSendmsg_handle },
    { "/adm_cmdsys",   eventAdmCmdsys_matches,   eventAdmCmdsys_handle },
    { "/adm_cmdtst",   eventAdmCmdtst_matches,   eventAdmCmdtst_handle },
    { "/adm_bot_off",  eventAdmBotOff_matches,   eventAdmBotOff_handle },
    { "/adm_bot_on",   eventAdmBotOn_matches,    eventAdmBotOn_handle },
    { "/adm_status",   eventAdmStatus_matches,   eventAdmStatus_handle },
    { "/adm_here",     eventAdmHere_matches,     eventAdmHere_handle },
};

// ========== SYSTEM REGISTRY ==========
static const ReplyBotCommand systemRegistry[] = {
    { "/nodes",     CommandCategory::SYSTEM, eventNodes_matches,     eventNodes_handle },
    { "/mesh",      CommandCategory::SYSTEM, eventMesh_matches,      eventMesh_handle },
    { "/ping",      CommandCategory::SYSTEM, eventPing_matches,      eventPing_handle },
    { "/hello",     CommandCategory::SYSTEM, eventHello_matches,     eventHello_handle },
    { "/test",      CommandCategory::SYSTEM, eventTest_matches,      eventTest_handle },
    { "/hard",      CommandCategory::SYSTEM, eventHard_matches,      eventHard_handle },
    { "/help",      CommandCategory::SYSTEM, eventHelp_matches,      eventHelp_handle },
    { "/telemetry", CommandCategory::SYSTEM, eventTelemetry_matches, eventTelemetry_handle },
};

// ========== PUBLIC REGISTRY ==========
static const ReplyBotCommand publicRegistry[] = {
    { "ping",  CommandCategory::PUBLIC, eventPubPing_matches,  eventPubPing_handle },
    { "test",  CommandCategory::PUBLIC, eventPubTest_matches,  eventPubTest_handle },
    { "hello", CommandCategory::PUBLIC, eventPubHello_matches, eventPubHello_handle },
};

// ========== ACCESSORS ==========
const ReplyBotAdminCommand* getAdminRegistry() { return adminRegistry; }
size_t getAdminRegistrySize() { return sizeof(adminRegistry) / sizeof(adminRegistry[0]); }

const ReplyBotCommand* getSystemRegistry() { return systemRegistry; }
size_t getSystemRegistrySize() { return sizeof(systemRegistry) / sizeof(systemRegistry[0]); }

const ReplyBotCommand* getPublicRegistry() { return publicRegistry; }
size_t getPublicRegistrySize() { return sizeof(publicRegistry) / sizeof(publicRegistry[0]); }
