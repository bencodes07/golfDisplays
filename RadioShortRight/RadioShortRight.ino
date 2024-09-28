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

const bool demoMode = false;

// EmuCan
#include "EMUcan.h"
EMUcan emucan(0x600); // Base ID 600 --> See EMU settings

#include <mcp2515.h>
struct can_frame canMsg;
MCP2515 mcp2515(10); // CS Pin 10

unsigned long previousMillis = 0;
const long interval = 500;

// NO EMU Display
unsigned long lastReceivedTime = 0; // Timer to track last received message time
const unsigned long requiredInterval = 5000; // 5 seconds interval

int mode = 0;

void setup(void) {
  u8g2.begin();
  u8g2.setDrawColor(1);
  u8g2.setFlipMode(1);
  Serial.begin(9600);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  pinMode(7, INPUT_PULLUP);
  pinMode(6, OUTPUT);
  pinMode(5, INPUT_PULLUP);
  delay(12000);
  mode = 1;
}

bool toggleValue = false;

void loop()
{
  u8g2.firstPage();

  do {
    static unsigned long lastToggleDebounceTime = 0;
    static int lastToggleButtonState = HIGH;
    static int toggleButtonState;
    const long toggleDebounceDelay = 50; // 50 ms debounce time

    int toggleReading = digitalRead(5);

    if (toggleReading != lastToggleButtonState) {
      lastToggleDebounceTime = millis();
    }

    if ((millis() - lastToggleDebounceTime) > toggleDebounceDelay) {
      if (toggleReading != toggleButtonState) {
        toggleButtonState = toggleReading;
        if(lastToggleButtonState == 0) {
          toggleValue = !toggleValue;
          digitalWrite(6, !toggleValue);
          Serial.println("Button Toggled");
          Serial.println(digitalRead(6));
        }
      }
    }
    
    lastToggleButtonState = toggleReading;

    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
      emucan.checkEMUcan(canMsg.can_id, canMsg.can_dlc, canMsg.data);
      if (emucan.EMUcan_Status() == EMUcan_RECEIVED_WITHIN_LAST_SECOND) {
        lastReceivedTime = millis();
      }
    }  
    static unsigned long lastDebounceTime = 0;
    static int lastButtonState = HIGH;
    static int buttonState;
    const long debounceDelay = 50; // 50 ms debounce time

    int reading = digitalRead(7);

    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == LOW) {
          mode++;
          if(mode == 2) mode = 0;
        }
      }
    }
    
    lastButtonState = reading;

    if (!demoMode) {
      if (millis() - lastReceivedTime < requiredInterval) {
        if (mode == 1) {
          u8g2.setFont(u8g2_font_profont22_mf);
          // Check the status of CAN switch 4
          bool canSwitch4Status = emucan.emu_data.outflags2 & EMUcan::OUTFLAGS2::F_CANSW4;
          const char* statusText = canSwitch4Status ? "LC1" : "LC2";
          u8g2.drawUTF8(0, 26, statusText);
        }
      } else {
        u8g2.setDrawColor(1);
        u8g2.setFont(u8g2_font_profont17_mf);
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "NO EMU");
        
        u8g2.drawUTF8(0, 26, buffer);
      }
    } else {
      if (mode == 1) {
        char buffer[20];
        u8g2.setFont(u8g2_font_profont22_mf);
        float myFloat = 0.81;
        int whole = (int)myFloat; // Extract whole part
        int fractional = (int)((myFloat - whole) * 100); // Extract two decimal places

        // Manually construct the floating-point representation
        snprintf(buffer, sizeof(buffer), "%d.%02d", whole, fractional);
        u8g2.drawUTF8(0, 26, buffer);
      }
    }
  } while(u8g2.nextPage());
}
