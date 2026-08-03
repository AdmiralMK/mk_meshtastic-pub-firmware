#include "EventNodes.h"
#include "NodeDB.h"
#include "RTC.h"
#include "configuration.h"
#include <cstring>
#include <cstdio>
#include <cctype>

bool eventNodes_matches(const char *msg)
{
    return strncmp(msg, "/nodes", 6) == 0 && (msg[6] == '\0' || isspace((unsigned char)msg[6]));
}

void eventNodes_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize)
{
    size_t onlineNodes = nodeDB->getNumOnlineMeshNodes();

    uint32_t total = nodeDB->getNumMeshNodes();
    uint32_t ourNode = nodeDB->getNodeNum();
    uint32_t active1h = 0;
    uint32_t neighbors05h = 0;
    uint32_t routers = 0;
    uint32_t favorites = 0;
    uint32_t ignored = 0;
    uint32_t hops0 = 0, hops1 = 0, hops2p = 0;

    const uint32_t nowSec = getValidTime(RTCQualityNTP);
    const bool haveTime = (nowSec > 0);

    for (uint32_t i = 0; i < total; i++) {
        auto *n = nodeDB->getMeshNodeByIndex(i);
        if (!n)
            continue;

        if (nodeInfoLiteIsFavorite(n))
            favorites++;
        if (nodeInfoLiteIsIgnored(n))
            ignored++;

        const bool isSelf = (n->num == ourNode);

        if (haveTime && n->last_heard > 0 && (nowSec - n->last_heard) < 3600)
            active1h++;

        if (isSelf)
            continue;

        if (n->has_hops_away && n->hops_away == 0 && haveTime && n->last_heard > 0 &&
            (nowSec - n->last_heard) < 1800) {
            neighbors05h++;
        }

        if (n->role == meshtastic_Config_DeviceConfig_Role_ROUTER ||
            n->role == meshtastic_Config_DeviceConfig_Role_REPEATER ||
            n->role == meshtastic_Config_DeviceConfig_Role_ROUTER_LATE) {
            routers++;
        }

        if (n->has_hops_away) {
            if (n->hops_away == 0)
                hops0++;
            else if (n->hops_away == 1)
                hops1++;
            else
                hops2p++; // 2 и более
        }
    }

    const uint32_t maxDb = (uint32_t)MAX_NUM_NODES;
    const uint32_t inDb = total;

    snprintf(reply, replySize,
             "📟 **NODES:**\n"
             "Online nodes: %lu\n"
             "Nodes: %lu | %lu active(1h)\n"
             "Neighbors(0.5h): %lu\n"
             "NodeDB: %lu | MaxDB: %lu\n"
             "Favorite: %lu | Ignored: %lu\n"
             "Routers/Repeater: %lu\n"
             "Node hops:\n"
             "- 0: %lu\n"
             "- 1: %lu\n"
             "- 2+: %lu",
             (unsigned long)onlineNodes, 
             (unsigned long)total, (unsigned long)active1h,
             (unsigned long)neighbors05h, 
             (unsigned long)inDb, (unsigned long)maxDb, 
             (unsigned long)favorites, (unsigned long)ignored, 
             (unsigned long)routers, 
             (unsigned long)hops0, 
             (unsigned long)hops1,
             (unsigned long)hops2p);
}