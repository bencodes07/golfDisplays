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

unsigned long previousMillis = 0;
const long interval = 500;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.drawXBMP(18, 0, 32, 32, turbo_bitmap);
  u8g2.sendBuffer();
  delay(5000);
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

  delay(1000);
}

unsigned long displayStartTime = 0;
bool displaySavedResult = false;

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
      uint16_t mapValue = emucan.emu_data.MAP;
      float currMapBar = mapValue;
      uint16_t savedMapValue = -1.0;

      if (mapValue > 180 && mapValue > savedMapValue) // Display when Map reaches 0.8 Bar
      {
        savedMapValue = mapValue; // Save the value when it goes above 1
        displayStartTime = millis(); // Record the time when the value goes above 1
        displaySavedResult = true; // Set the flag to display the saved result
      }

      if (displaySavedResult)
      {
        // Display Maximum MAP Value
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_logisoso26_tr);
        u8g2.setCursor(-2, 28);
        u8g2.print(currMapBar / 100 - 1);
        u8g2.sendBuffer();

        delay(1000);

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_logisoso20_tr);
        u8g2.setCursor(-2, 28);
        u8g2.print("  bar");
        u8g2.sendBuffer();

        delay(400);

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_logisoso20_tr);
        u8g2.setCursor(-2, 28);
        u8g2.print("boost");
        u8g2.sendBuffer();

        delay(400);

        if (millis() - displayStartTime >= 5000) // Display the saved value for 5 seconds
        {
          displaySavedResult = false; // Reset the flag
          savedMapValue = -1;
        }
      } else 
      {
        int8_t currGear = emucan.emu_data.gear;
        int8_t currDSG = emucan.emu_data.DSGmode;

        // Display Current Gear
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_logisoso26_tr);
        u8g2.setCursor(18, 28);
        u8g2.print(translateDSG(currDSG) + translateGear(currGear));
        // DSGmode starts at 2 to 7
        // gear 0 is N --> -1 R --> 1,2,3,4,5,6
        u8g2.sendBuffer();
      }

    } else {
      Serial.println("No communication from EMU");
    }
    if (emucan.emu_data.flags1 & emucan.F_IDLE) {
      Serial.println("Engine Idle active");
    }
    if (emucan.decodeCel()) {
      Serial.println("WARNING Engine CEL active");
    }
  }
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
