#include <Arduino.h>

static const uint8_t adcPin = 4; 
static const uint8_t lowLightLedPin = 14;
static const uint8_t highLightLedPin = 13;
static const uint8_t switchBitsResolutionButtonPin = 16;
static const uint8_t switchDbButtonPin = 18;

static const uint8_t adcResolutionBits[] =  {4, 8, 10, 12};
static const double adcResolutionMaxVal[] =  {16, 256, 1024, 4096};
static const adc_attenuation_t attenuations[] =  {ADC_0db, ADC_2_5db, ADC_6db, ADC_11db};
static const uint16_t attenuationsMax[] =  {1100, 1500, 2200, 3300};

static uint8_t currentResolutionIndex = 2;
static uint8_t currentAttenuationIndex = 0;
static uint8_t resSize = 4;
static uint8_t attSize = 4;

static const float lowLightThreshold = 0.1;
static const float highLightThreshold = 0.9;

static const int loopDelayMs = 100;
static const int logDelayMs = 1000;
static int logTimeCounter = 0;

static const uint32_t debounceMks = 30000; // 30ms

volatile uint32_t lastResBtnPress = 0;
volatile uint32_t lastAttBtnPress = 0;

volatile bool resBtnEvent = false;
volatile bool attBtnEvent = false;

void setRes();
void setAtt();
void tryTurnOnLowLed(int adcValue, int maxVal);
void tryTurnOnHighLed(int adcValue, int maxVal);
void IRAM_ATTR resSwitchPressed();
void IRAM_ATTR attSwitchPressed();
void handleBtnsInLoop();


void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial to initialize
  Serial.println("ADC Setup");
  Serial.print(" | loopDelayMs: ");
  Serial.println(loopDelayMs);

  setRes();
  setAtt();

  pinMode(lowLightLedPin, OUTPUT);
  pinMode(highLightLedPin, OUTPUT);
  pinMode(switchBitsResolutionButtonPin, INPUT_PULLUP);
  pinMode(switchDbButtonPin, INPUT_PULLUP);

  attachInterrupt(switchBitsResolutionButtonPin, resSwitchPressed, FALLING);
  attachInterrupt(switchDbButtonPin, attSwitchPressed, FALLING);
}


void loop() {
  handleBtnsInLoop();

  int adcValue = analogRead(adcPin);
  int millivoltsRead = analogReadMilliVolts(adcPin);
  int maxVal = adcResolutionMaxVal[currentResolutionIndex];
  int maxVolt = attenuationsMax[currentAttenuationIndex];
  int millivoltsCalc = (adcValue / (float)maxVal) * maxVolt;

  if(logTimeCounter > logDelayMs){
      logTimeCounter = 0;
      Serial.print("maxVal : ");
      Serial.print(maxVal);
      Serial.print(" | maxVolt: ");
      Serial.print(maxVolt);
      Serial.print(" mV | ");
      Serial.print("ADC Value: ");
      Serial.print(adcValue);
      Serial.print(" | millivoltsRead: ");
      Serial.print(millivoltsRead);
      Serial.print(" mV");
      Serial.print(" | millivoltsCalc: ");
      Serial.print(millivoltsCalc);
      Serial.print(" mV. | Diifference: ");
      Serial.print(abs(millivoltsRead - millivoltsCalc));
      Serial.println(" mV.");
  } else {
      logTimeCounter += loopDelayMs;
  }

  tryTurnOnLowLed(adcValue, maxVal);
  tryTurnOnHighLed(adcValue, maxVal);

  delay(loopDelayMs);
}

void handleBtnsInLoop() {
  if(resBtnEvent){
    noInterrupts();
    resBtnEvent = false;
    interrupts();

    Serial.println("---- RES button pressed");
    currentResolutionIndex = (currentResolutionIndex + 1) % resSize;
    setRes();
  }

  if(attBtnEvent){
    noInterrupts();
    attBtnEvent = false;
    interrupts();
    
    Serial.println("---- ATT button pressed");
    currentAttenuationIndex = (currentAttenuationIndex + 1) % attSize;
    setAtt();
  }
}

void IRAM_ATTR resSwitchPressed() {
  uint32_t now = micros();
  if(now - lastResBtnPress > debounceMks)
  {
    lastResBtnPress = now;
    resBtnEvent = true;
  }
}

void IRAM_ATTR attSwitchPressed() {
  uint32_t now = micros();
  if(now - lastAttBtnPress > debounceMks)
  {
    lastAttBtnPress = now;
    attBtnEvent = true;
  }
}

void tryTurnOnLowLed(int adcValue, int maxVal){
  if(adcValue <= maxVal * lowLightThreshold) {
    digitalWrite(lowLightLedPin, HIGH);
  } else {
    digitalWrite(lowLightLedPin, LOW);
  }
}

void tryTurnOnHighLed(int adcValue, int maxVal){
  if(adcValue >= maxVal * highLightThreshold) {
    digitalWrite(highLightLedPin, HIGH);
  } else {
    digitalWrite(highLightLedPin, LOW);
  }
}

void setRes() {
  Serial.print("### Current ADC Resolution: ");
  Serial.print(adcResolutionBits[currentResolutionIndex]);
  Serial.println(" bits.");
  analogReadResolution(adcResolutionBits[currentResolutionIndex]);
}

void setAtt() {
  Serial.print("### Current ADC Attenuation: ");
  switch(attenuations[currentAttenuationIndex]) {
    case ADC_0db:
      Serial.println("0 dB (max input voltage approx. 1.1V)");
      break;
    case ADC_2_5db:
      Serial.println("2.5 dB (max input voltage approx. 1.5V)");
      break;
    case ADC_6db:
      Serial.println("6 dB (max input voltage approx. 2.2V)");
      break;
    case ADC_11db:
      Serial.println("11 dB (max input voltage approx. 3.3V)");
      break;
    default:
      Serial.println("Unknown attenuation");
      break;
  }
  analogSetPinAttenuation(adcPin, attenuations[currentAttenuationIndex]);
}