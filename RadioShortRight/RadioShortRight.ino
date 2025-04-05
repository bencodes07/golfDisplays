#include <Wire.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <avr/pgmspace.h>

// Use a more memory-efficient display configuration
U8G2_SSD1306_64X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

// EmuCan
#include "EMUcan.h"
EMUcan emucan(0x600); // Base ID 600 --> See EMU settings

#include <mcp2515.h>
struct can_frame canMsg;
MCP2515 mcp2515(10); // CS Pin 10

// Timing constants
unsigned long prevMillis = 0;
unsigned long lastReceivedTime = 0;
unsigned long toggleDisplayTime = 0;

// Button constants
#define MODE_BTN_PIN 7     // Button to toggle display on/off
#define OUTPUT_PIN 6
#define DEBOUNCE_DELAY 50

// Display states
enum DisplayState {
  DISPLAY_OFF,
  DISPLAY_LD,
  DISPLAY_LC
};

// Last values
float lastAnalogIn6 = -1.0;
DisplayState displayState = DISPLAY_LD; // Start with LD display on
bool lcValue = false;      // LC state (false = LC2, true = LC1)
bool previousLcValue = false; // Store previous LC value to detect changes

// Button states
int modeBtnState = HIGH;
int lastModeBtnState = HIGH;
unsigned long lastModeDbnTime = 0;

// LD threshold values adjusted for sensor offset
const float ldThresholds[] = {1.05, 1.58, 2.05, 2.65, 3.15, 3.75, 4.35, 4.9};

void setup(void) {
  Serial.begin(9600);
  Serial.println("Initializing...");
  
  // Initialize display
  u8g2.begin();
  u8g2.setDrawColor(1);
  u8g2.setFlipMode(1);
  
  // Initialize CAN
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  
  // Setup pins - explicit INPUT_PULLUP
  pinMode(MODE_BTN_PIN, INPUT_PULLUP);
  pinMode(OUTPUT_PIN, OUTPUT);
  
  // Initial pin state check and debug
  Serial.print("Initial MODE pin state: ");
  Serial.println(digitalRead(MODE_BTN_PIN));
  
  Serial.println("Setup complete");
}

void loop() {
  // Process mode button for display on/off
  handleModeButton();
  
  // Check CAN messages
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    emucan.checkEMUcan(canMsg.can_id, canMsg.can_dlc, canMsg.data);
    if (emucan.EMUcan_Status() == EMUcan_RECEIVED_WITHIN_LAST_SECOND) {
      lastReceivedTime = millis();
      
      // Get current analog input 6 value
      float currentAnalogIn6 = emucan.emu_data.analogIn6;
      
      // Check for LC value change from ECU (CAN switch 4 status)
      bool currentLcValue = (emucan.emu_data.outflags2 & EMUcan::F_CANSW4) > 0;
      
      // If LC value changed, update display and output
      if (currentLcValue != previousLcValue) {
        lcValue = currentLcValue;
        previousLcValue = currentLcValue;
        digitalWrite(OUTPUT_PIN, !lcValue);
        
        Serial.print("LC Changed from ECU: ");
        Serial.println(lcValue ? "LC1" : "LC2");
        
        // Show LC state for 3 seconds
        if (displayState != DISPLAY_OFF) {
          displayState = DISPLAY_LC;
          toggleDisplayTime = millis();
          updateDisplay();
        }
      }
      
      // Update if analog value changed or first read
      if (lastAnalogIn6 == -1.0 || abs(currentAnalogIn6 - lastAnalogIn6) >= 0.03) {
        lastAnalogIn6 = currentAnalogIn6;
        Serial.print("Analog Input 6: ");
        Serial.println(currentAnalogIn6);
        
        // Print the LD level for debugging
        byte ldNum = getLdNumber(currentAnalogIn6);
        Serial.print("LD Level: ");
        Serial.println(ldNum);
        
        if (displayState != DISPLAY_OFF) {
          updateDisplay();
        }
      }
    }
  }
  
  // Check if LC display time is up
  if (displayState == DISPLAY_LC && (millis() - toggleDisplayTime >= 3000)) {
    Serial.println("LC display timeout - returning to LD display");
    displayState = DISPLAY_LD;
    updateDisplay();
  }
  
  // Regular update at interval
  unsigned long now = millis();
  if (now - prevMillis >= 100) {
    prevMillis = now;
    if (displayState != DISPLAY_OFF) {
      updateDisplay();
    }
  }
}

void handleModeButton() {
  // Read mode button (display on/off)
  int modeReading = digitalRead(MODE_BTN_PIN);
  
  // Handle debouncing
  if (modeReading != lastModeBtnState) {
    lastModeDbnTime = millis();
  }
  
  if ((millis() - lastModeDbnTime) > DEBOUNCE_DELAY) {
    if (modeReading != modeBtnState) {
      modeBtnState = modeReading;
      
      // Button is pressed (LOW)
      if (modeBtnState == LOW) {
        // Toggle display state
        if (displayState == DISPLAY_OFF) {
          // Turn display on
          displayState = DISPLAY_LD;
          Serial.println("Display turned ON");
          updateDisplay();
        } else {
          // Turn display off
          displayState = DISPLAY_OFF;
          Serial.println("Display turned OFF");
          clearDisplay();
        }
      }
    }
  }
  lastModeBtnState = modeReading;
}

// Get LD number based on voltage with deadzone
byte getLdNumber(float voltage) {
  // Check each threshold with specific ranges
  if (voltage < ldThresholds[0]) return 1;
  if (voltage < ldThresholds[1]) return 2;
  if (voltage < ldThresholds[2]) return 3;
  if (voltage < ldThresholds[3]) return 4;
  if (voltage < ldThresholds[4]) return 5;
  if (voltage < ldThresholds[5]) return 6;
  if (voltage < ldThresholds[6]) return 7;
  
  // Default to LD8 for anything higher
  return 8;
}

void clearDisplay() {
  u8g2.firstPage();
  do {
    // Empty draw - blank screen
  } while (u8g2.nextPage());
}

void updateDisplay() {
  u8g2.firstPage();
  do {
    // Check if EMU communication is active
    if (millis() - lastReceivedTime < 5000) {
      if (displayState == DISPLAY_LC) {
        // Display LC1 or LC2 based on ECU value
        u8g2.setFont(u8g2_font_profont22_mf);
        u8g2.drawStr(0, 26, lcValue ? "LC1" : "LC2");
      } else {
        // Display LD status based on analog value
        u8g2.setFont(u8g2_font_profont22_mf);
        
        // Get LD number based on voltage thresholds
        byte ldNum = getLdNumber(lastAnalogIn6);
        
        // Display LD status
        char buffer[4] = "LD";
        buffer[2] = '0' + ldNum;
        buffer[3] = 0;
        u8g2.drawStr(0, 26, buffer);
      }
    } else {
      // No EMU communication
      u8g2.setFont(u8g2_font_profont17_mf);
      u8g2.drawStr(0, 26, "NO EMU");
    }
  } while (u8g2.nextPage());
}