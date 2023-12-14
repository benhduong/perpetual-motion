#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char SSID[]     = SECRET_SSID;             // Network SSID (name)
const char PASS[]     = SECRET_OPTIONAL_PASS;    // Network password (use for WPA, or use as key for WEP)

void onMelodyChange();
void onPerpetualMotionMachineChange();

CloudSwitch melody;
CloudSwitch perpetualMotionMachine;

void initProperties() {
  ArduinoCloud.addProperty(melody, READWRITE, ON_CHANGE, onMelodyChange);
  ArduinoCloud.addProperty(perpetualMotionMachine, READWRITE, ON_CHANGE, onPerpetualMotionMachineChange);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
