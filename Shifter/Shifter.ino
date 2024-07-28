#include <Wire.h>
#include <Adafruit_MCP4725.h>

const int refV = 4124;
const int I2C_ADDR = 0x60;
Adafruit_MCP4725 dac;

enum GearPosition {
  P, R, N, D, S, M
};

GearPosition currentPosition = P;
GearPosition targetPosition = P;

const int numPositions = 6;
const int positionPins[numPositions] = {2, 3, 4, 5, 6, 7};

unsigned long lastPositionChangeTime = 0;
const unsigned long positionChangeDelay = 200;

void setup()
{
  for (int i = 2; i <= 9; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  Serial.begin(9600);
  Wire.begin();
  dac.begin(I2C_ADDR);
  delay(100);
  setVoltageForPosition(P);
}

GearPosition getLeverPosition() {
  for (int i = 0; i < numPositions; i++) {
    if (digitalRead(positionPins[i]) == LOW) {
      return static_cast<GearPosition>(i);
    }
  }
  return currentPosition;
}

void setVoltageForPosition(GearPosition position) {
  float voltage = 0.0;
  const char* positionName = "";

  switch (position) {
    case P: voltage = 0.5; positionName = "P"; break;
    case R: voltage = 0.9; positionName = "R"; break;
    case N: voltage = 1.3; positionName = "N"; break;
    case D: voltage = 1.7; positionName = "D"; break;
    case S: voltage = 2.1; positionName = "S"; break;
    case M: voltage = 2.5; positionName = "M"; break;
  }

  dac.setVoltage(voltage * refV / 5.0, false);
  Serial.print(positionName);
  Serial.println(" selected");
}

void handleManualMode() {
  static bool upshiftPressed = false;
  static bool downshiftPressed = false;

  if (digitalRead(8) == LOW && !upshiftPressed) {
    dac.setVoltage(2.9 * refV / 5.0, false);
    upshiftPressed = true;
    Serial.println("Upshift pressed");
  } else if (digitalRead(8) == HIGH && upshiftPressed) {
    upshiftPressed = false;
  }

  if (digitalRead(9) == LOW && !downshiftPressed) {
    dac.setVoltage(3.3 * refV / 5.0, false);
    downshiftPressed = true;
    Serial.println("Downshift pressed");
  } else if (digitalRead(9) == HIGH && downshiftPressed) {
    downshiftPressed = false;
  }
}

void loop()
{
  GearPosition leverPosition = getLeverPosition();
  
  if (leverPosition != targetPosition) {
    targetPosition = leverPosition;
    lastPositionChangeTime = millis();
  }

  if (targetPosition != currentPosition && (millis() - lastPositionChangeTime >= positionChangeDelay)) {
    currentPosition = targetPosition;
    setVoltageForPosition(currentPosition);
  }
  
  if (currentPosition == M) {
    handleManualMode();
  }
}