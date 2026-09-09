#include "WebServer.h"
#include "WebUI.h"
#include "Config.h"
#include <string.h>
#include <stdlib.h>

// The ESP8266WebServer API hands back arguments as Arduino String - that is
// the one unavoidable String touch-point in this project (see Phase 4
// audit). Every String here is read once via .c_str()/.length() and copied
// into a fixed buffer immediately; none is stored, concatenated, or grown.

static bool parseUint(const String &s, long &out) {
    if (s.length() == 0) return false;
    char *end = nullptr;
    long v = strtol(s.c_str(), &end, 10);
    if (end == s.c_str() || *end != '\0') return false;
    out = v;
    return true;
}

void WebServerModule::begin(StateManager *state, ShiftRegister *sr, WiFiAP *ap) {
    _state = state;
    _sr = sr;
    _ap = ap;

    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/state", HTTP_GET, [this]() { handleState(); });
    _server.on("/relay", HTTP_POST, [this]() { handlePostRelay(); });
    _server.on("/rgb", HTTP_POST, [this]() { handlePostRgb(); });
    _server.on("/wifi", HTTP_GET, [this]() { handleWifiPage(); });
    _server.on("/wifi/state", HTTP_GET, [this]() { handleWifiState(); });
    _server.on("/wifi", HTTP_POST, [this]() { handlePostWifi(); });
    _server.begin();
}

void WebServerModule::handleRoot() {
    _server.send_P(200, "text/html", MAIN_PAGE);
}

void WebServerModule::handleState() {
    char buf[128];
    size_t pos = 0;
    pos += snprintf(buf + pos, sizeof(buf) - pos, "{\"r\":[");
    for (uint8_t i = 0; i < SWITCH_COUNT; i++)
        pos += snprintf(buf + pos, sizeof(buf) - pos, "%s%d", i ? "," : "", _state->get(i).relayOn ? 1 : 0);
    pos += snprintf(buf + pos, sizeof(buf) - pos, "],\"on\":[");
    for (uint8_t i = 0; i < SWITCH_COUNT; i++)
        pos += snprintf(buf + pos, sizeof(buf) - pos, "%s%d", i ? "," : "", _state->get(i).colorOn);
    pos += snprintf(buf + pos, sizeof(buf) - pos, "],\"off\":[");
    for (uint8_t i = 0; i < SWITCH_COUNT; i++)
        pos += snprintf(buf + pos, sizeof(buf) - pos, "%s%d", i ? "," : "", _state->get(i).colorOff);
    snprintf(buf + pos, sizeof(buf) - pos, "]}");
    _server.send(200, "application/json", buf);
}

void WebServerModule::handlePostRelay() {
    long idx, state;
    if (!parseUint(_server.arg("i"), idx) || !parseUint(_server.arg("s"), state)) return sendBadRequest();
    if (idx < 0 || idx >= SWITCH_COUNT || (state != 0 && state != 1)) return sendBadRequest();

    bool want = (state == 1);
    if (_state->get((uint8_t)idx).relayOn != want) {
        _state->toggleRelay((uint8_t)idx, *_sr);
    }
    sendJsonOk();
}

void WebServerModule::handlePostRgb() {
    long idx, which, color;
    if (!parseUint(_server.arg("i"), idx) || !parseUint(_server.arg("w"), which) || !parseUint(_server.arg("c"), color))
        return sendBadRequest();
    if (idx < 0 || idx >= SWITCH_COUNT || (which != 0 && which != 1) || color < 0 || color >= COLOR_COUNT)
        return sendBadRequest();

    _state->setColor((uint8_t)idx, which == 1, (uint8_t)color, *_sr);
    sendJsonOk();
}

void WebServerModule::handleWifiPage() {
    _server.send_P(200, "text/html", WIFI_PAGE);
}

void WebServerModule::handleWifiState() {
    char ssid[AP_SSID_MAX_LEN + 1];
    _ap->getSsid(ssid, sizeof(ssid));
    char buf[48];
    snprintf(buf, sizeof(buf), "{\"ssid\":\"%s\"}", ssid);
    _server.send(200, "application/json", buf);
}

void WebServerModule::handlePostWifi() {
    String ssidArg = _server.arg("ssid");
    String passArg = _server.hasArg("open") ? String() : _server.arg("password");

    if (ssidArg.length() > AP_SSID_MAX_LEN || passArg.length() > AP_PASS_MAX_LEN) return sendBadRequest();

    char ssidBuf[AP_SSID_MAX_LEN + 1];
    char passBuf[AP_PASS_MAX_LEN + 1];
    strncpy(ssidBuf, ssidArg.c_str(), sizeof(ssidBuf) - 1); ssidBuf[sizeof(ssidBuf) - 1] = '\0';
    strncpy(passBuf, passArg.c_str(), sizeof(passBuf) - 1); passBuf[sizeof(passBuf) - 1] = '\0';

    if (!_ap->setCredentials(ssidBuf, passBuf, (uint8_t)passArg.length())) return sendBadRequest();

    _server.send_P(200, "text/html", RESTART_NOTICE_PAGE);
}

void WebServerModule::sendJsonOk() {
    _server.send(200, "application/json", "{\"ok\":1}");
}

void WebServerModule::sendBadRequest() {
    _server.send(400, "application/json", "{\"ok\":0}");
}
