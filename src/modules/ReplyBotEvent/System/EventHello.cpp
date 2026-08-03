#include "EventHello.h"
#include <cstring>
#include <cstdio>
#include <cctype>

bool eventHello_matches(const char *msg) {
    return strncmp(msg, "/hello", 6) == 0 && (msg[6] == '\0' || isspace((unsigned char)msg[6]));
}

void eventHello_handle(const meshtastic_MeshPacket &mp, char *reply, size_t replySize) {
    int hopsAway = mp.hop_start - mp.hop_limit;
    int rssi = mp.rx_rssi;
    if (rssi > 0) rssi -= 200;
    float snr = mp.rx_snr;
    snprintf(reply, replySize, "👋 Hello! : %d Hops | RSSI %d | SNR %.1f", hopsAway, rssi, snr);
}