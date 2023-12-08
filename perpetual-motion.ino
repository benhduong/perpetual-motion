#define magnet 7
#define sensor 9

volatile bool ballSensed = false;
int ballCount = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(magnet, OUTPUT);
  pinMode(sensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensor), checkForBall, LOW);

  // init serial monitor
  Serial.begin(9600);
  while(!Serial);
  Serial.println("system ready");
}

void loop() {
  // put your main code here, to run repeatedly:
  magnetOn();
  updateBallCount();
}

void checkForBall() {
  if (!ballSensed) {
    ballSensed = true;
  }
}

void magnetOn() {
  if (ballSensed) {
    // turn magnet on
    digitalWrite(magnet, HIGH);
    Serial.println("HIGH");
    delay(500);

    // turn magnet off
    digitalWrite(magnet, LOW);
    Serial.println("LOW");
    delay(500);
  }
}

void updateBallCount() {
  if (ballSensed) {
    // update variables and print ball count
    ballSensed = false;
    ballCount += 1;
    Serial.print("ball ");
    Serial.println(ballCount);
  }
}