#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <Arduino.h>
#include "Config.h"

// Everything that survives a reboot, in one block. One combined
// magic/version/checksum instead of separate ones per field group - the
// whole struct is under 128 bytes regardless of SWITCH_COUNT, so nothing is
// gained by splitting it, and one load/save path is simpler to reason about.
//
// colorOn/colorOff are always sized for 4 switches (not SWITCH_COUNT) so the
// EEPROM layout stays stable even if a unit is later reflashed with a
// different SWITCH_COUNT.
struct PersistentState {
    uint8_t magic;                 // must equal PERSIST_MAGIC
    uint8_t version;                // must equal PERSIST_VERSION
    uint8_t relayStates;            // bit i = relay i, 1 = on
    uint8_t colorOn[4];
    uint8_t colorOff[4];
    char    apSsid[AP_SSID_MAX_LEN + 1];
    char    apPassword[AP_PASS_MAX_LEN + 1];
    uint8_t checksum;               // sum of all preceding bytes, mod 256
};

#define PERSIST_MAGIC   0xA5
#define PERSIST_VERSION 1

namespace Persistence {
    void begin();                              // EEPROM.begin(sizeof(PersistentState))
    bool load(PersistentState &out);            // false if invalid/corrupt -> caller must apply defaults
    void save(const PersistentState &in);       // recomputes checksum, writes, commits
    void applyDefaults(PersistentState &out);   // safe factory defaults (relays off, AP blank)

    // Factory reset: corrupts just the magic byte and commits (cheap, single
    // byte write). The next boot's existing "load() failed" path in the
    // .ino re-applies defaults and generates a fresh AP name - no separate
    // reset logic to keep in sync with normal first-boot behavior.
    void invalidate();
}

#endif // PERSISTENCE_H