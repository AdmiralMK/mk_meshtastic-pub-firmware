#include "EventPing.h"
#include "mesh/MeshTypes.h"
#include <cstring>
#include <cstdio>
#include <cctype>

bool eventPing_matches(const char *msg)
{
    // Только /ping — /hello и /test обрабатывают свои handlers
    return strncmp(msg, "/ping", 5) == 0 && (msg[5] == '\0' || isspace((unsigned char)msg[5]));
}

void eventPing_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize)
{
    int hopsAway = mp.hop_start - mp.hop_limit;
    int rssi = mp.rx_rssi;
    if (rssi > 0)
        rssi -= 200;
    float snr = mp.rx_snr;
    snprintf(reply, replySize, "🎙️ Mic Check : %d Hops | RSSI %d | SNR %.1f", hopsAway, rssi, snr);
}