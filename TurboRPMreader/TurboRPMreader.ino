#include <Wire.h>
#include <Adafruit_MCP4725.h>

const int inputPin = 8;
const int refV = 4124;  // From your existing code
Adafruit_MCP4725 dac;

unsigned long lastTime = 0;
unsigned long period = 0;

void setup() {
  pinMode(inputPin, INPUT);
  Serial.begin(115200);
  dac.begin(0x60);  // Default I2C address
}

void loop() {
  while(!(PINB & (1 << (inputPin - 8)))); 
  unsigned long startTime = micros();
  
  while((PINB & (1 << (inputPin - 8))));
  
  period = micros() - startTime;
  
  float frequency = 10000000.0 / (period * 2);
  float rpm = (frequency * 60) / 16;
  
  // Convert RPM to voltage (0-5V range)
  // Adjust maxRPM based on expected range
  const float maxRPM = 150000.0;
  float voltage = (rpm / maxRPM) * 5.0;
  
  // Set DAC output
  uint16_t dacValue = (voltage * refV) / 5.0;
  
  
  if(millis() - lastTime > 10) {
    dac.setVoltage(dacValue, false);
    Serial.print("RPM: ");
    Serial.print(rpm);
    Serial.print(" Voltage: ");
    Serial.println(voltage);
    lastTime = millis();
  }
}