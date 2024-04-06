#include <Wire.h>
#include <Adafruit_MCP4725.h>

const int refV = 4124;
const int I2C_ADDR = 0x60;
Adafruit_MCP4725 dac;

void setup()
{
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  Serial.begin(9600);
  Wire.begin();
  dac.begin(I2C_ADDR);
  delay(100);
  dac.setVoltage(0.5 * refV / 5.0, false);
}

int buttonPStatus = 1;
int buttonRStatus = 1;
int buttonNStatus = 1;
int buttonDStatus = 1;
int buttonSStatus = 1;
int buttonMStatus = 1;
int buttonPlusStatus = 1;
int buttonMinusStatus = 1;

void loop()
{
  int pin2Value = digitalRead(2);
  int pin3Value = digitalRead(3);
  int pin4Value = digitalRead(4);
  int pin5Value = digitalRead(5);
  int pin6Value = digitalRead(6);
  int pin7Value = digitalRead(7);
  int pin8Value = digitalRead(8);
  int pin9Value = digitalRead(9);
  delay(10);

  if (buttonPStatus != pin2Value)
  {
    buttonPStatus = pin2Value;
    dac.setVoltage(0.5 * refV / 5.0, false);
    Serial.println("P pressed");
  }
  if (buttonRStatus != pin3Value)
  {
    buttonRStatus = pin3Value;
    dac.setVoltage(0.9 * refV / 5.0, false);
    Serial.println("R pressed");
  }
  if (buttonNStatus != pin4Value)
  {
    buttonNStatus = pin4Value;
    dac.setVoltage(1.3 * refV / 5.0, false);
    Serial.println("N pressed");
  }
  if (buttonDStatus != pin5Value)
  {
    buttonDStatus = pin5Value;
    dac.setVoltage(1.7 * refV / 5.0, false);
    Serial.println("D pressed");
  }
  if (buttonSStatus != pin6Value)
  {
    buttonSStatus = pin6Value;
    dac.setVoltage(2.1 * refV / 5.0, false);
    Serial.println("S pressed");
  }
  if (buttonMStatus != pin7Value)
  {
    buttonMStatus = pin7Value;
    dac.setVoltage(2.5 * refV / 5.0, false);
    Serial.println("M pressed");
  }

  if (buttonPlusStatus != pin8Value)
  {
    buttonPlusStatus = pin8Value;
    dac.setVoltage(2.9 * refV / 5.0, false);
    Serial.println("Upshift pressed");
  }
  if (buttonMinusStatus != pin9Value)
  {
    buttonMinusStatus = pin9Value;
    dac.setVoltage(3.3 * refV / 5.0, false);
    Serial.println("Downshift pressed");
  }
}