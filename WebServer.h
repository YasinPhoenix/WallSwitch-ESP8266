#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "State.h"
#include "ShiftRegister.h"
#include "WiFiAP.h"

// GET  /            -> main control page (PROGMEM)
// GET  /state        -> {"r":[...],"on":[...],"off":[...]}
// POST /relay         i=<idx>&s=<0|1>            -> {"ok":1}
// POST /rgb           i=<idx>&w=<0|1>&c=<0-7>    -> {"ok":1}   (w: 0=off-color, 1=on-color)
// GET  /wifi          -> AP settings page (PROGMEM)
// GET  /wifi/state    -> {"ssid":"..."}           (password is never sent back)
// POST /wifi           ssid=...&open=on&password=...  -> saves + schedules restart
class WebServerModule {
public:
    void begin(StateManager *state, ShiftRegister *sr, WiFiAP *ap);
    void handleClient() { _server.handleClient(); }

private:
    ESP8266WebServer _server{80};
    StateManager *_state = nullptr;
    ShiftRegister *_sr = nullptr;
    WiFiAP *_ap = nullptr;

    void handleRoot();
    void handleState();
    void handlePostRelay();
    void handlePostRgb();
    void handleWifiPage();
    void handleWifiState();
    void handlePostWifi();

    void sendJsonOk();
    void sendBadRequest();
};

#endif // WEB_SERVER_H
