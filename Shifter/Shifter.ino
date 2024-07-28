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
const int upshiftPin = 8;
const int downshiftPin = 9;

unsigned long lastPositionChangeTime = 0;
const unsigned long positionChangeDelay = 100; // Adjust this value as needed

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

void handleManualShifting() {
  static bool upshiftPressed = false;
  static bool downshiftPressed = false;

  if (digitalRead(upshiftPin) == LOW && !upshiftPressed) {
    dac.setVoltage(2.9 * refV / 5.0, false);
    upshiftPressed = true;
    Serial.println("Upshift pressed");
  } else if (digitalRead(upshiftPin) == HIGH && upshiftPressed) {
    setVoltageForPosition(M);  // Reset to M position voltage
    upshiftPressed = false;
  }

  if (digitalRead(downshiftPin) == LOW && !downshiftPressed) {
    dac.setVoltage(3.3 * refV / 5.0, false);
    downshiftPressed = true;
    Serial.println("Downshift pressed");
  } else if (digitalRead(downshiftPin) == HIGH && downshiftPressed) {
    setVoltageForPosition(M);  // Reset to M position voltage
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
  
  // Handle manual shifting immediately, without delay
  if (currentPosition == M) {
    handleManualShifting();
  }
}