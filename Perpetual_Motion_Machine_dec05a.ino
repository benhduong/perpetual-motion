#include "thingProperties.h"
#include "pitches.h"

#include <Servo.h>

Servo servo;

#define magnet 7
#define sensor 8
#define buzzer 5
#define servoPin 10

bool ballSensed = false;
int ballCount = 0;

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

void setup() {
  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  pinMode(magnet, OUTPUT);
  analogWrite(magnet, 255);
  pinMode(sensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensor), magnetOn, FALLING);

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

void loop() {
  ArduinoCloud.update();
  // Your code here
  if (perpetualMotionMachine) {
    updateBallCount();
    
    if (melody && ballSensed) {
      // Add your code here to act upon Melody change
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
      ballSensed = false;
      if (ballCount == size) {
        ballCount = 0;
      }
    }
    
    petWatchdog();
  }
}

void magnetOn() {
  // turn magnet on
  analogWrite(magnet, 0);
  delayMicroseconds(16 * 1000);

  // turn magnet off
  analogWrite(magnet, 255);
  Serial.println("FIRED");
  ballSensed = true;
}

void updateBallCount() {
  if (ballSensed) {
    // update variables and print ball count
    ballCount += 1;
    Serial.print("ball ");
    Serial.println(ballCount);
  }
}

/*
  Since PerpetualMotionMachine is READ_WRITE variable, onPerpetualMotionMachineChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onPerpetualMotionMachineChange()  {
  if (perpetualMotionMachine) {
    attachInterrupt(digitalPinToInterrupt(sensor), magnetOn, FALLING);
    servo.write(80);
  } else {
    servo.write(180);
    delay(1000);
    detachInterrupt(digitalPinToInterrupt(sensor));
  }
}
