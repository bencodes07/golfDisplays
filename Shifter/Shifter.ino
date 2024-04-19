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

bool upshiftPressed = false;
bool downshiftPressed = false;

bool manualSelected = false;

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

  if (buttonMStatus != pin7Value)
  {
    buttonMStatus = pin7Value;
    manualSelected = true;
    dac.setVoltage(2.5 * refV / 5.0, false);
    Serial.println("M pressed");
  } else if (buttonMStatus == pin7Value && manualSelected) {

    if (buttonPlusStatus != pin8Value && !upshiftPressed)
    {
      dac.setVoltage(2.9 * refV / 5.0, false);
      upshiftPressed = true;
      delay(50);
      Serial.println("Upshift pressed");
    } 
    if (buttonPlusStatus == pin8Value && upshiftPressed) {
      buttonMStatus = !pin7Value;
      upshiftPressed = false;
    }
    if (buttonMinusStatus != pin9Value && !downshiftPressed)
    {
      dac.setVoltage(3.3 * refV / 5.0, false);
      downshiftPressed = true;
      delay(50);
      Serial.println("Downshift pressed");
    } 
    if (buttonMinusStatus == pin9Value && downshiftPressed) {
      buttonMStatus = !pin7Value;
      downshiftPressed = false;
    }
  }
  if (buttonPStatus != pin2Value)
  {
    buttonPStatus = pin2Value;
    manualSelected = false;
    dac.setVoltage(0.5 * refV / 5.0, false);
    Serial.println("P pressed");
  }
  if (buttonRStatus != pin3Value)
  {
    buttonRStatus = pin3Value;
    manualSelected = false;
    dac.setVoltage(0.9 * refV / 5.0, false);
    Serial.println("R pressed");
  }
  if (buttonNStatus != pin4Value)
  {
    buttonNStatus = pin4Value;
    manualSelected = false;
    dac.setVoltage(1.3 * refV / 5.0, false);
    Serial.println("N pressed");
  }
  if (buttonDStatus != pin5Value)
  {
    buttonDStatus = pin5Value;
    manualSelected = false;
    dac.setVoltage(1.7 * refV / 5.0, false);
    Serial.println("D pressed");
  }
  if (buttonSStatus != pin6Value)
  {
    buttonSStatus = pin6Value;
    manualSelected = false;
    dac.setVoltage(2.1 * refV / 5.0, false);
    Serial.println("S pressed");
  }
}