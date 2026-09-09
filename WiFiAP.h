#ifndef WIFI_AP_H
#define WIFI_AP_H

#include <Arduino.h>
#include "Config.h"
#include "Persistence.h"

// The device is an access point only - it never joins another network.
// Users connect their phone directly to it to reach the control page.
// begin() starts the AP with whatever SSID/password is in the resolved
// PersistentState. A later setCredentials() call (from the /wifi settings
// page) validates, persists, and schedules a restart - softAP() is not
// "hot swapped" while a client is still connected under the old identity.
class WiFiAP {
public:
    void begin(const PersistentState &p);

    void getSsid(char *out, size_t outLen) const; // for the settings page - never exposes the password
    bool setCredentials(const char *ssid, const char *password, uint8_t passwordLen);

    void tick(uint32_t nowMs); // call every loop() - performs the scheduled restart, if any

    static bool validateSsid(const char *ssid);
    static bool validatePassword(const char *password, uint8_t len);

private:
    char _ssid[AP_SSID_MAX_LEN + 1];
    bool _restartPending = false;
    uint32_t _restartAtMs = 0;
};

#endif // WIFI_AP_H
