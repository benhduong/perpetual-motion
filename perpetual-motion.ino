#include <Pitches.h>
#include "thingProperties.h"

#include <Servo.h>

Servo servo;

// pin numbers
#define magnet 7
#define sensor 8
#define buzzer 5
#define servoPin 10

#define MAGNET_DURATION 16 // duration that magnet should be on in ms

// reflects if the inductive sensor detected a ball (T == detected; F == no detection)
static volatile bool ballSensed = false;

int notes[] = {
  NOTE_B4, NOTE_A4, NOTE_G4, 
  NOTE_B4, NOTE_A4, NOTE_G4, 
  NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4,
  NOTE_A4, NOTE_A4, NOTE_A4, NOTE_A4,
  NOTE_B4, NOTE_A4, NOTE_G4, 
};

int durations[] = {
  2, 2, 2,
  2, 2, 2,
  4, 4, 4, 4,
  4, 4, 4, 4,
  2, 2, 2
};

int ballCount = 0;

void setup() {
  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  pinMode(magnet, OUTPUT);
  toggleMagnet(false); // magnet starts off

  pinMode(sensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensor), sensorChanged, CHANGE);

  servo.attach(servoPin);

  pinMode(buzzer, OUTPUT);
  
  setupWatchdog();

  // init serial monitor
  Serial.begin(9600);
  while(!Serial);
  Serial.println("system ready");
}

// Note: code taken from lab 4
void setupWatchdog() {
  // Clear and enable WDT
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_SetPriority(WDT_IRQn, 0);
  NVIC_EnableIRQ(WDT_IRQn);

  // Configure and enable WDT GCLK:
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(4) | GCLK_GENDIV_ID(5);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->GENCTRL.reg = GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_ID(5) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC(3);
  while(GCLK->STATUS.bit.SYNCBUSY);
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_GEN(5) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID(3);

  // Configure and enable WDT:
  // use WDT->CONFIG.reg, WDT->EWCTRL.reg, WDT->CTRL.reg
  WDT->CONFIG.reg = WDT_CONFIG_PER(9);
  WDT->EWCTRL.reg = WDT_EWCTRL_EWOFFSET(8);
  WDT->CTRL.reg = WDT_CTRL_ENABLE;
  while(WDT->STATUS.bit.SYNCBUSY);
}

void petWatchdog() {
  WDT->CLEAR.reg = 0xA5;
}

// ISR called when sensor changes
void sensorChanged() {
  // NOTE: sensor is HIGH when no ball is detected, LOW when ball is detected
  PinStatus curSignal = digitalRead(sensor);
  if (curSignal == HIGH) {
    // rising edge, ball leaves sensor range
    ballSensed = false;
  } else if (curSignal == LOW) {
    // falling edge, ball enters sensor range
    ballSensed = true;
  } else {
    // should never happen
    Serial.print("unexpected sensor read of ");
    Serial.println(curSignal);
  }
}

void loop() {
  static state CURRENT_STATE = sWAIT_FOR_BALL;
  // update inputs from cloud
  ArduinoCloud.update();
  updateFSM(CURRENT_STATE, millis(), perpetualMotionMachine, ballSensed);   
  petWatchdog();
}

state updateFSM(state curState, long mils, CloudSwitch isOn, bool ballSensed) {
  static long savedClock = mils;
  state nextState;
  switch(curState) {
    case sWAIT_FOR_BALL:
      if (!isOn) {
        // 1-3: turn system off
        nextState = sSYSTEM_OFF;
      }
      if (ballSensed && isOn) {
        // 1-2: ball sensed, turn magnet on
        // turn magnet on
        toggleMagnet(true);
        nextState = sELECTROMAGNET_ON;
        // reset savedClock
        savedClock = mils;
        // play next note
        playNote();
      }
    case sELECTROMAGNET_ON:
      if ((mils - savedClock) >= MAGNET_DURATION) {
        // 2-1: magnet has been on long enough, turn it off
        toggleMagnet(false);
        nextState = sWAIT_FOR_BALL;
      }
    case sSYSTEM_OFF:
      if (isOn) {
        // 3-1: system toggled back on
        ballCount = 0;
        nextState = sWAIT_FOR_BALL;
      }
  }

  return nextState;
}

void toggleMagnet(bool on) {
  if (on) {
    analogWrite(magnet, 0);
    ballSensed = true;
  } else {
    analogWrite(magnet, 255);
  }
}

void playNote() {
  // increment and print ball count
  ballCount += 1;
  Serial.print("ball ");
  Serial.println(ballCount);
  if (musicEnabled) {
    int size = sizeof(durations) / sizeof(int);
    //to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int duration = 1000 / durations[ballCount - 1];
    tone(buzzer, notes[ballCount - 1], duration);

    //to distinguish the notes, set a minimum time between them.
    //the note's duration + 30% seems to work well:
    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);
    
    //stop the tone playing:
    noTone(buzzer);
    // ballSensed = false;
    if (ballCount == size) {
      ballCount = 0;
    }
  }
}

/*
  Since PerpetualMotionMachine is READ_WRITE variable, onPerpetualMotionMachineChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onPerpetualMotionMachineChange()  {
  if (perpetualMotionMachine) {
    attachInterrupt(digitalPinToInterrupt(sensor), sensorChanged, CHANGE);
    servo.write(80);
  } else {
    servo.write(180);
    delay(1000); // delay lets ball complete loop
    detachInterrupt(digitalPinToInterrupt(sensor));
  }
}

// Don't do anything on music enabled change
void onMusicEnabledChange() {

}


test testList[] = {{sWAIT_FOR_BALL, sELECTROMAGNET_ON, 1000, mockIsOn = true, false}};
int numTests = 0;

bool testUpdateFSM(struct test tl[]) {
  for (int i = 0; i < numTests; i++) {
    test currentTest = tl[i];

    if (updateFSM(currentTest.in, currentTest.mils, currentTest.isOn, currentTest.ballSensed) != currentTest.out) {
      return false;
    }
  }
  return true;
}