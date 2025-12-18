#include <Arduino.h>

static const uint8_t adcPin = 4; 
static const uint8_t lowLightLedPin = 14;
static const uint8_t highLightLedPin = 13;
static const uint8_t switchBitsResolutionButtonPin = 12;
static const uint8_t switchDbButtonPin = 11;

static const uint8_t adcResolutionBits[] =  {4, 8, 10, 12};
static const double adcResolutionMaxVal[] =  {16, 256, 1024, 4096};
static const adc_attenuation_t attenuations[] =  {ADC_0db, ADC_2_5db, ADC_6db, ADC_11db};
static const uint16_t attenuationsMax[] =  {1100, 1500, 2200, 3300};

static uint8_t currentResolutionIndex = 2;
static uint8_t currentAttenuationIndex = 0;
static uint8_t resSize = 4;
static uint8_t attSize = 4;

static const float lowLightThreshold = 0.25;
static const float highLightThreshold = 0.75;

void setRes();
void setAtt();

void IRAM_ATTR resSwitchPressed() {
  Serial.println("Resolution button pressed");
  currentResolutionIndex = (currentResolutionIndex + 1) % resSize;
  setRes();
}

void IRAM_ATTR attSwitchPressed() {
  Serial.println("Attenuation button pressed");
  currentAttenuationIndex = (currentAttenuationIndex + 1) % attSize;
  setAtt();
}


void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial to initialize
  Serial.println("ADC Setup");

  setRes();
  setAtt();
  analogReadResolution(adcResolutionBits[currentResolutionIndex]);
  analogSetPinAttenuation(adcPin, attenuations[currentAttenuationIndex]);

  pinMode(lowLightLedPin, OUTPUT);
  pinMode(highLightLedPin, OUTPUT);
  pinMode(switchBitsResolutionButtonPin, INPUT_PULLUP);
  pinMode(switchDbButtonPin, INPUT_PULLUP);

  attachInterrupt(switchBitsResolutionButtonPin, resSwitchPressed, FALLING);
  attachInterrupt(switchDbButtonPin, attSwitchPressed, FALLING);
}


void loop() {
  int adcValue = analogRead(adcPin);
  int millivoltsRead = analogReadMilliVolts(adcPin);
  int maxVal = adcResolutionMaxVal[currentResolutionIndex];
  int maxVolt = attenuationsMax[currentAttenuationIndex];
  int millivoltsCalc = (adcValue / maxVal) * maxVolt;

  Serial.print("ADC Value: ");
  Serial.print(adcValue);
  Serial.print(" | millivoltsRead: ");
  Serial.print(millivoltsRead);
  Serial.print(" mV");
  Serial.print(" | millivoltsCalc: ");
  Serial.print(millivoltsCalc);
  Serial.print(" mV. | ");
  Serial.print(abs(millivoltsRead - millivoltsCalc));
  Serial.println(" mV.");

  if(adcValue <= maxVal * lowLightThreshold) {
    digitalWrite(lowLightLedPin, LOW);
  } else {
    digitalWrite(lowLightLedPin, HIGH);
  }

  if(adcValue >= maxVal * highLightThreshold) {
    digitalWrite(highLightLedPin, LOW);
  } else {
    digitalWrite(highLightLedPin, HIGH);
  }

   delay(100);
}

void setRes() {
  Serial.print("Current ADC Resolution: ");
  Serial.print(adcResolutionBits[currentResolutionIndex]);
  Serial.println(" bits.");
  analogReadResolution(adcResolutionBits[currentResolutionIndex]);
}

void setAtt() {
  Serial.print("Current ADC Attenuation: ");
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



