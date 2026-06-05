#include <Wire.h>
#include <Arduino.h>
#include <U8g2lib.h>

#include <avr/pgmspace.h>

U8G2_SSD1306_64X32_1F_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

const unsigned char turbo_bitmap [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x3f, 0xc0, 0x00, 0x00, 0x20, 0x20, 0x03, 0x1b, 0x50, 
	0x30, 0x00, 0x28, 0x50, 0x08, 0x00, 0x60, 0x10, 0x28, 0xfe, 0x90, 0x60, 0x04, 0x81, 0xa3, 0x20, 
	0x84, 0x2d, 0x42, 0x20, 0x8a, 0x64, 0x46, 0x63, 0x4a, 0xc2, 0x44, 0x4e, 0x42, 0x93, 0x89, 0x18, 
	0x46, 0x34, 0x8b, 0x01, 0xc4, 0x24, 0x02, 0x01, 0x82, 0xc9, 0x44, 0x00, 0x0a, 0x9b, 0xc5, 0x00, 
	0x8c, 0x10, 0x81, 0x00, 0x14, 0x67, 0x22, 0x00, 0x10, 0xcc, 0x30, 0x00, 0x28, 0x00, 0x48, 0x00, 
	0xe0, 0x00, 0x0c, 0x00, 0x80, 0x01, 0x13, 0x00, 0x00, 0x7f, 0x02, 0x00, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// EmuCan
#include "EMUcan.h"
EMUcan emucan(0x600); // Base ID 600 --> See EMU settings

#include <mcp2515.h>
struct can_frame canMsg;
MCP2515 mcp2515(10); // CS Pin 10

// Timing constants
unsigned long previousMillis = 0;
const long interval = 50; // Update interval in ms
const int splashScreenDelay = 5000; // Splash screen display time in ms
const int setupDelay = 1000; // Setup delay in ms

// RPM constants
const uint16_t MIN_STABLE_RPM = 700; // Minimum RPM to consider engine running
const float RPM_HYSTERESIS_FACTOR = 0.8; // Factor for RPM hysteresis

// Boost/MAP constants
const float MIN_BOOST_BAR = 0.4; // Minimum boost to record in bar
const uint16_t MIN_BOOST_MAP = (uint16_t)((MIN_BOOST_BAR + 1.0) * 100); // Convert to MAP units (110)
const uint16_t LOW_MAP_THRESHOLD = 50; // Low MAP threshold to trigger display
const uint16_t MAX_VALID_MAP = 400; // Maximum valid MAP value

// Display constants
const int blinkOnTime = 500; // Display on time in ms
const int blinkOffTime = 200; // Display off time in ms
const int barDisplayTime = 500; // "bar" text display time
const int boostTextTime = 400; // "boost" text display time
const int displayCycles = 3; // Number of display cycles
const int blinksPerCycle = 3; // Number of blinks per cycle

// Flag to indicate if the engine is running and stable
bool engineStable = false;
bool displaySavedResult = false;
bool hasRecordedValidBoost = false; // Flag to track if we've seen a valid boost peak

// Boost tracking variables
uint16_t savedMapValue = 0;
float savedMapFloat = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  u8g2.begin();

  // Display splash screen
  u8g2.clearBuffer();
  u8g2.drawXBMP(18, 0, 32, 32, turbo_bitmap);
  u8g2.sendBuffer();
  delay(splashScreenDelay);
  
  // Show "boost" text
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso20_tr);
  u8g2.setCursor(-2, 28);
  u8g2.print("boost");
  u8g2.sendBuffer();

  Serial.print("EMUCAN_LIB_VERSION: ");
  Serial.println(EMUCAN_LIB_VERSION);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  delay(setupDelay);
}

void loop()
{
  // Call the EMUcan lib with every received frame:
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    emucan.checkEMUcan(canMsg.can_id, canMsg.can_dlc, canMsg.data);
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    if (emucan.EMUcan_Status() == EMUcan_RECEIVED_WITHIN_LAST_SECOND) {
      // Check if engine is running and stable based on RPM
      uint16_t rpm = emucan.emu_data.RPM;
      
      // Engine stability check
      if (!engineStable && rpm >= MIN_STABLE_RPM) {
        engineStable = true;
        Serial.println("Engine stabilized - RPM: " + String(rpm));
      } else if (engineStable && rpm < MIN_STABLE_RPM * RPM_HYSTERESIS_FACTOR) {
        engineStable = false;
        Serial.println("Engine no longer stable - RPM: " + String(rpm));
        // Reset boost tracking when engine drops below stable RPM
        resetBoostTracking();
      }
      
      uint16_t mapValue = emucan.emu_data.MAP;
      float mapBar = (float)mapValue/100.0 - 1.0;
      
      // Only process MAP values when engine is stable
      if (engineStable) {
        // Check if boost is higher than our minimum threshold (0.1 bar)
        if (mapValue >= MIN_BOOST_MAP) {
          hasRecordedValidBoost = true;
          
          // Record peak boost if current value is higher than saved value
          if (mapValue > savedMapValue) {
            savedMapValue = mapValue;
            savedMapFloat = savedMapValue;
            Serial.print("New max MAP: ");
            Serial.print(mapBar);
            Serial.println(" bar");
          }
        }
        
        // Detect when boost drops significantly (when letting go of throttle)
        if (hasRecordedValidBoost && mapValue < LOW_MAP_THRESHOLD && savedMapValue >= MIN_BOOST_MAP) {
          displaySavedResult = true; // Set the flag to display the saved result
        }
      }

      // Reset max value if it's invalid
      if (savedMapValue >= MAX_VALID_MAP) {
        Serial.println("Resetting invalid MAP value");
        resetBoostTracking();
      }

      // Display the saved boost result (only if it's above our 0.1 bar threshold)
      if (displaySavedResult && savedMapValue >= MIN_BOOST_MAP) {
        displayMaxBoost();
        resetBoostTracking();
      } 
      else {
        // Display current gear when not showing boost
        displayGear();
      }

    } else {
      Serial.println("No communication from EMU");
    }
    
    if (emucan.decodeCel()) {
      Serial.println("WARNING Engine CEL active");
    }
  }
}

// Reset all boost tracking variables
void resetBoostTracking() {
  savedMapValue = 0;
  savedMapFloat = 0;
  hasRecordedValidBoost = false;
  displaySavedResult = false;
}

// Display the maximum boost value with flashing animation
void displayMaxBoost() {
  float boostBar = savedMapFloat / 100 - 1;
  
  for (int i = 0; i < displayCycles; i++) {
    // Blink the boost value
    for (int j = 0; j < blinksPerCycle; j++) {
      // Show boost value
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_logisoso26_tr);
      u8g2.setCursor(-2, 28);
      u8g2.print(boostBar);
      u8g2.sendBuffer();
      delay(blinkOnTime);

      // Blank screen
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_logisoso26_tr);
      u8g2.setCursor(-2, 28);
      u8g2.print("");
      u8g2.sendBuffer();
      delay(blinkOffTime);
    }

    // Show boost value again
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso26_tr);
    u8g2.setCursor(-2, 28);
    u8g2.print(boostBar);
    u8g2.sendBuffer();
    delay(barDisplayTime);

    // Show "bar" text
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr);
    u8g2.setCursor(-2, 28);
    u8g2.print("  bar");
    u8g2.sendBuffer();
    delay(blinkOffTime);

    // Show "boost" text
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso20_tr);
    u8g2.setCursor(-2, 28);
    u8g2.print("boost");
    u8g2.sendBuffer();
    delay(boostTextTime);
  }
}

// Display current gear information
void displayGear() {
  int8_t currGear = emucan.emu_data.gear;
  int8_t currDSG = emucan.emu_data.DSGmode;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso26_tr);
  u8g2.setCursor(18, 28);
  u8g2.print(translateDSG(currDSG) + translateGear(currGear));
  u8g2.sendBuffer();
}

String translateGear(int8_t gear) {
  if(gear == -1 || gear == 0) {
    return "";
  } else {
    return String(gear);
  }
}

String translateDSG(int8_t dsg) {
  switch(dsg) {
    case 2:
      return "P";
      break; 
    case 3:
      return "R";
      break;
    case 4:
      return "N";
      break;
    case 5:
      return "D";
      break;
    case 6:
      return "S";
      break;
    case 7:
      return "M";
      break;
    default:
      return "M";
  }
}