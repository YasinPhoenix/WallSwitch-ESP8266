#include "Persistence.h"
#include <EEPROM.h>
#include <string.h>

static uint8_t computeChecksum(const PersistentState &s) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&s);
    uint16_t sum = 0;
    // Every byte except the checksum field itself, which is always last.
    for (size_t i = 0; i < sizeof(PersistentState) - 1; i++) {
        sum += bytes[i];
    }
    return static_cast<uint8_t>(sum & 0xFF);
}

void Persistence::begin() {
    EEPROM.begin(sizeof(PersistentState));
}

void Persistence::applyDefaults(PersistentState &out) {
    memset(&out, 0, sizeof(out));
    out.magic = PERSIST_MAGIC;
    out.version = PERSIST_VERSION;
    out.relayStates = 0; // all relays off
    for (uint8_t i = 0; i < 4; i++) {
        out.colorOn[i]  = DEFAULT_COLOR_ON;
        out.colorOff[i] = DEFAULT_COLOR_OFF;
    }
    // apSsid/apPassword left blank (zeroed above -> empty strings). The
    // caller (main .ino, on first boot only) fills in a generated default
    // SSID before the first save, since that needs ESP.getChipId().
    out.checksum = computeChecksum(out);
}

bool Persistence::load(PersistentState &out) {
    EEPROM.get(0, out);
    if (out.magic != PERSIST_MAGIC) return false;
    if (out.version != PERSIST_VERSION) return false;
    if (computeChecksum(out) != out.checksum) return false;
    // Defensive: force null termination regardless, even though a matching
    // checksum should make this redundant - never trust a flash read enough
    // to skip a bounds guard on a buffer we're about to strlen()/copy.
    out.apSsid[AP_SSID_MAX_LEN] = '\0';
    out.apPassword[AP_PASS_MAX_LEN] = '\0';
    return true;
}

void Persistence::save(const PersistentState &in) {
    PersistentState copy = in;
    copy.magic = PERSIST_MAGIC;
    copy.version = PERSIST_VERSION;
    copy.checksum = computeChecksum(copy);
    EEPROM.put(0, copy);
    EEPROM.commit();
}
