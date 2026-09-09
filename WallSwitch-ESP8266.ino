#include "Config.h"
#include "ShiftRegister.h"
#include "State.h"
#include "Persistence.h"
#include "InputManager.h"
#include "WiFiAP.h"
#include "WebServer.h"

ShiftRegister shiftRegister;
StateManager stateManager;
InputManager inputManager;
WiFiAP wifiAP;
WebServerModule webServer;

void setup() {
    // 1. GPIO pin modes only. No shift-register write yet, no relay/LED
    //    transition happens until the real starting state is known.
    shiftRegister.begin();
    inputManager.begin();

    // 2. Resolve persistent state once, here, before anything touches the
    //    physical outputs or starts the access point. A corrupt or
    //    first-ever EEPROM gets safe defaults plus a generated AP name,
    //    saved once, so StateManager and WiFiAP never have to race each
    //    other to establish the first valid record.
    Persistence::begin();
    PersistentState state;
    if (!Persistence::load(state)) {
        Persistence::applyDefaults(state);
        char chipSuffix[7];
        snprintf(chipSuffix, sizeof(chipSuffix), "%06X", (unsigned)(ESP.getChipId() & 0xFFFFFF));
        snprintf(state.apSsid, sizeof(state.apSsid), "Switch-%s", chipSuffix);
        Persistence::save(state);
    }

    // 3. Apply the resolved state and perform the ONE startup physical
    //    write - see ShiftRegister::begin()/StateManager::begin() for why
    //    there is no separate "zero the registers" step before this.
    stateManager.begin(state, shiftRegister);

    // 4. Bring up the access point and the web server.
    wifiAP.begin(state);
    webServer.begin(&stateManager, &shiftRegister, &wifiAP);
}

void loop() {
    uint32_t now = millis();

    uint8_t pressedIndex;
    if (inputManager.poll(pressedIndex)) {
        stateManager.toggleRelay(pressedIndex, shiftRegister);
    }

    // Hold every touch at once for FACTORY_RESET_HOLD_MS to forget the AP
    // SSID/password (and relay/color prefs) and reboot to defaults.
    if (inputManager.checkFactoryReset(now)) {
        Persistence::invalidate();
        ESP.restart();
    }

    webServer.handleClient();
    stateManager.tick(now);
    wifiAP.tick(now);
}
