#include "arduino_secrets.h"
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

/*
  Since Melody is READ_WRITE variable, onMelodyChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMelodyChange()  {
  
}
