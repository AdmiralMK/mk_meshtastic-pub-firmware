#include "EventMesh.h"
#include "NodeDB.h"
#include "RTC.h"
#include "RadioLibInterface.h"
#include "airtime.h"
#include "configuration.h"
#include <cstring>
#include <cstdio>
#include <cctype>

bool eventMesh_matches(const char *msg)
{
    return strncmp(msg, "/mesh", 5) == 0 && (msg[5] == '\0' || isspace((unsigned char)msg[5]));
}

void eventMesh_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize)
{
    // --- Airtime ---
    float chUtil = 0.0f;
    float airUtil = 0.0f;
    if (airTime) {
        chUtil = airTime->channelUtilizationPercent();
        airUtil = airTime->utilizationTXPercent();
    }
    float chNoise = chUtil; // прежняя семантика «Channel noise»

    // --- Radio stats / noise floor ---
    int32_t noiseFloorDbm = -120;
    uint32_t txGood = 0, rxGood = 0, rxBad = 0, txRelay = 0;
    uint16_t txDrop = 0;
    if (RadioLibInterface::instance) {
        RadioLibInterface::instance->updateNoiseFloor();
        noiseFloorDbm = RadioLibInterface::instance->getAverageNoiseFloor();
        txGood = RadioLibInterface::instance->txGood;
        rxGood = RadioLibInterface::instance->rxGood;
        rxBad = RadioLibInterface::instance->rxBad;
        txRelay = RadioLibInterface::instance->txRelay;
        txDrop = RadioLibInterface::instance->txDrop;
    }

    // --- Скан NodeDB ---
    uint32_t total = nodeDB->getNumMeshNodes();
    uint32_t ourNode = nodeDB->getNodeNum();
    uint32_t active1h = 0;
    uint32_t neighbors05h = 0;
    uint32_t routers = 0;
    uint32_t favorites = 0;
    uint32_t ignored = 0;
    uint32_t hops0 = 0, hops1 = 0, hops2 = 0, hops3p = 0;
    float bestSnr = -999.0f;
    bool haveSnr = false;

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

        const bool isNeighbor = (n->has_hops_away && n->hops_away == 0);
        if (isNeighbor && haveTime && n->last_heard > 0 && (nowSec - n->last_heard) < 1800) {
            neighbors05h++;
            if (n->snr > bestSnr) {
                bestSnr = n->snr;
                haveSnr = true;
            }
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
            else if (n->hops_away == 2)
                hops2++;
            else
                hops3p++;
        }
    }

    snprintf(reply, replySize,
             "🌐 **MESH:**\n"
             "Channel Utilization %.1f %%\n"
             "Air Utilization %.1f %%\n"
             "Channel noise: %.1f %%\n"
             "Noise floor: %ld dBm\n"
             "Best SNR: %s%.1f\n"
             "TX: %lu (relay %lu) | RX: %lu\n"
             "rxBad: %lu | txDrop: %u",
             chUtil, 
             airUtil, 
             chNoise, 
             (long)noiseFloorDbm,
             haveSnr ? "" : "N/A", haveSnr ? bestSnr : 0.0f,
             (unsigned long)txGood, (unsigned long)txRelay, (unsigned long)rxGood,
             (unsigned long)rxBad, (unsigned)txDrop
            );
}