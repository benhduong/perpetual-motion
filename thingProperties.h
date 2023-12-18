#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
const char SSID[]     = "SECRET_SSID";             // Network SSID (name)
const char PASS[]     = "SECRET_OPTIONAL_PASS";    // Network password (use for WPA, or use as key for WEP)

void onMusicEnabledChange();
void onPerpetualMotionMachineChange();

CloudSwitch musicEnabled;
CloudSwitch perpetualMotionMachine;
CloudSwitch mockIsOn;


void initProperties() {
  ArduinoCloud.addProperty(musicEnabled, READWRITE, ON_CHANGE, onMusicEnabledChange);
  ArduinoCloud.addProperty(perpetualMotionMachine, READWRITE, ON_CHANGE, onPerpetualMotionMachineChange);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

typedef enum {
  sWAIT_FOR_BALL = 1,
  sELECTROMAGNET_ON = 2,
  sSYSTEM_OFF = 3,
} state;

struct test {
  state in;
  state out;
  long mils;
  CloudSwitch isOn;
  bool ballSensed;
};