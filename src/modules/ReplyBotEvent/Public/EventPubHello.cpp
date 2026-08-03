#include "EventPubHello.h"
#include <cstring>
#include <cstdio>
#include <strings.h>

bool eventPubHello_matches(const char *msg) {
    return strncasecmp(msg, "hello", 5) == 0;
}

void eventPubHello_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize) {
    int hopsAway = mp.hop_start - mp.hop_limit;
    int rssi = mp.rx_rssi;
    if (rssi > 0) rssi -= 200;
    float snr = mp.rx_snr;
    snprintf(reply, replySize, "👋 Hello! : %d Hops | RSSI %d | SNR %.1f", hopsAway, rssi, snr);
}