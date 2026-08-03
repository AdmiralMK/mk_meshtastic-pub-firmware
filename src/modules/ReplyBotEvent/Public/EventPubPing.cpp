#include "EventPubPing.h"
#include <cstring>
#include <cstdio>
#include <strings.h>

bool eventPubPing_matches(const char *msg) {
    return strncasecmp(msg, "ping", 4) == 0;
}

void eventPubPing_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize) {
    int hopsAway = mp.hop_start - mp.hop_limit;
    int rssi = mp.rx_rssi;
    if (rssi > 0) rssi -= 200;
    float snr = mp.rx_snr;
    snprintf(reply, replySize, "🎙️ Mic Check : %d Hops | RSSI %d | SNR %.1f", hopsAway, rssi, snr);
}