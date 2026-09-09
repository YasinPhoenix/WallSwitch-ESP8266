#include "WiFiAP.h"
#include <ESP8266WiFi.h>
#include <string.h>

void WiFiAP::begin(const PersistentState &p) {
    strncpy(_ssid, p.apSsid, sizeof(_ssid) - 1);
    _ssid[sizeof(_ssid) - 1] = '\0';

    WiFi.mode(WIFI_AP);
    if (p.apPassword[0] == '\0') {
        WiFi.softAP(_ssid); // open network
    } else {
        WiFi.softAP(_ssid, p.apPassword);
    }
}

void WiFiAP::getSsid(char *out, size_t outLen) const {
    strncpy(out, _ssid, outLen - 1);
    out[outLen - 1] = '\0';
}

bool WiFiAP::validateSsid(const char *ssid) {
    size_t len = strlen(ssid);
    if (len < 1 || len > AP_SSID_MAX_LEN) return false;
    // Reject characters that would break the hand-rolled JSON in
    // WebServer::handleWifiState() - simpler and more robust than trying to
    // escape them at read time.
    for (size_t i = 0; i < len; i++) {
        char c = ssid[i];
        if (c == '"' || c == '\\' || (unsigned char)c < 0x20) return false;
    }
    return true;
}

bool WiFiAP::validatePassword(const char *password, uint8_t len) {
    (void)password; // only length is constrained, per spec (empty, or >= 8 chars)
    return len == 0 || (len >= AP_PASS_MIN_LEN && len <= AP_PASS_MAX_LEN);
}

bool WiFiAP::setCredentials(const char *ssid, const char *password, uint8_t passwordLen) {
    if (!validateSsid(ssid)) return false;
    if (!validatePassword(password, passwordLen)) return false;

    PersistentState p;
    if (!Persistence::load(p)) {
        Persistence::applyDefaults(p);
    }
    strncpy(p.apSsid, ssid, sizeof(p.apSsid) - 1);
    p.apSsid[sizeof(p.apSsid) - 1] = '\0';
    strncpy(p.apPassword, password, sizeof(p.apPassword) - 1);
    p.apPassword[sizeof(p.apPassword) - 1] = '\0';
    Persistence::save(p);

    _restartPending = true;
    _restartAtMs = millis() + WIFI_RESTART_DELAY_MS;
    return true;
}

void WiFiAP::tick(uint32_t nowMs) {
    if (_restartPending && nowMs >= _restartAtMs) {
        ESP.restart();
    }
}
