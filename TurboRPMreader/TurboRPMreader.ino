#include <Wire.h>
#include <Adafruit_MCP4725.h>

const int inputPin = 8;
const int refV = 4124;
Adafruit_MCP4725 dac;

unsigned long lastTime = 0;
unsigned long period = 0;

const float BLADES = 14.0;
const float DIVISOR = 8.0;

void setup() {
  pinMode(inputPin, INPUT);
  Serial.begin(115200);
  dac.begin(0x60);
}

void loop() {
  while (!(PINB & (1 << (inputPin - 8))));
  unsigned long startTime = micros();
  while ((PINB & (1 << (inputPin - 8))));
  period = micros() - startTime;

  float frequency = 1000000.0 / (period * 2.0);

  // RPM from blade count and divisor
  float rpm_new = frequency * (DIVISOR * 60.0) / BLADES;

  // Old estimate (kept for comparison)
  float rpm_old = (10000000.0 / (period * 2.0)) * 60.0 / 16.0;

  // TSS Voltage Out = (RPM - 2555) / 37568
  float tss_voltage = (rpm_new - 2555.0) / 37568.0;
  tss_voltage = constrain(tss_voltage, 0.0, 5.0);

  uint16_t dacValue = (tss_voltage / 5.0) * refV;

  if (millis() - lastTime > 10) {
    dac.setVoltage(dacValue, false);

    Serial.print("Freq: ");      Serial.print(frequency);
    Serial.print(" Hz | RPM_new: "); Serial.print(rpm_new);
    Serial.print(" | RPM_old: ");    Serial.print(rpm_old);
    Serial.print(" | TSS_Vout: ");   Serial.print(tss_voltage, 4);
    Serial.print(" V | DAC: ");      Serial.println(dacValue);

    lastTime = millis();
  }
}




// RPM = 37568 * (TSS Voltage Out) + 2555
// 100000 rpm * 14 blades/rev = 1400000 blade pass counts per minute
// 1400000 / 60 sec/min = 23333 blade pass counts per second (Hz)
// 23333 / 8 count divisor = 2917 Hz output frequency





