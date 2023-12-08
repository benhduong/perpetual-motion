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

  setupWatchdog();

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
  // put your main code here, to run repeatedly:
  magnetOn();
  updateBallCount();
  petWatchdog();
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