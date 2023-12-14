#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "arduino_secrets.h"

const char SSID[]     = SECRET_SSID;             // Network SSID (name)
const char PASS[]     = SECRET_OPTIONAL_PASS;    // Network password (use for WPA, or use as key for WEP)

void onMusicEnabledChange();
void onPerpetualMotionMachineChange();

CloudSwitch musicEnabled;
CloudSwitch perpetualMotionMachine;

void initProperties() {
  ArduinoCloud.addProperty(musicEnabled, READWRITE, ON_CHANGE, onMusicEnabledChange);
  ArduinoCloud.addProperty(perpetualMotionMachine, READWRITE, ON_CHANGE, onPerpetualMotionMachineChange);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
