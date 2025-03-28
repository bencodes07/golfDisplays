#include <Arduino.h>
#include <U8g2lib.h>

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

const bool demoMode = false;

U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCK, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C

void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_logisoso20_tr);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

// 'image-Golf 1', 90x32px
const unsigned char epd_bitmap_image_Golf_1 [] PROGMEM = {
	0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
	0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0xff, 0x6f, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0x80, 0x03, 0x00, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xf0, 0x07, 0x00, 0x03, 0x00, 0x38, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x03, 0x00, 
	0x03, 0x00, 0x70, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x00, 0x03, 0x00, 0xe0, 0x0c, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x03, 0x00, 0xc0, 0x19, 0x00, 0x00, 0x00, 0x00, 
	0x80, 0x3f, 0x00, 0x00, 0x03, 0x00, 0x80, 0x33, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 
	0x03, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0x07, 0x00, 0x00, 0xce, 
	0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 
	0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0x3f, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0xfe, 0x3f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x3f, 0xfc, 0x7f, 0x00, 0xfe, 0x0f, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xf0, 0x7f, 0x00, 
	0xfe, 0x07, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xe0, 0xff, 0x00, 0xff, 0x03, 0xc0, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0x03, 0xc0, 0xff, 0x03, 0xff, 0x03, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x03, 0xc0, 0xff, 0x03, 0xf0, 0x03, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xc0, 0x7f, 0x00, 
	0xf0, 0x03, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xc0, 0x1f, 0x00, 0xe0, 0x03, 0xc0, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0x01, 0xc0, 0x0f, 0x00, 0x80, 0x02, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x03, 0xc0, 0x03, 0x00, 0x00, 0x06, 0x60, 0xff, 0xff, 0xff, 0xff, 0xff, 0x02, 0x60, 0x00, 0x00, 
	0x00, 0x04, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x30, 0x00, 0x00, 0x00, 0x0c, 0x30, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x78, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x78, 0x0e, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00
};

int x_position = -90;
int mode = 1;

void draw_golf() {
  u8g2.setBitmapMode(false /* solid */);
  u8g2.drawXBMP(x_position, 7, 90, 32, epd_bitmap_image_Golf_1);
}

void draw(void) {
  draw_golf();
}

void setup(void) {
  u8g2.begin();
  Serial.begin(9600);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  u8g2_prepare();
  pinMode(7, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, OUTPUT);
}

void displayTemperature(const char* label, uint16_t tempInt, const char* unit, bool divide) {
  char buffer[30]; // Increase buffer size if necessary

  // Set the font for the label and temperature
  u8g2.setFont(u8g2_font_profont22_mf);

  int labelTempLen;
  if (divide) {
    // When dividing, we treat the integer as a floating-point number with one decimal place
    float tempFloat = tempInt / 10.0; // Use 10.0 to move the decimal one place to the left
    int whole = (int)tempFloat;
    int fractional = abs((int)((tempFloat - whole) * 10)); // Get the fractional part, multiplied to become a single digit

    // Use snprintf to format the string, separating the whole and fractional parts for display
    labelTempLen = snprintf(buffer, sizeof(buffer), "%s: %d.%d", label, whole, fractional);
  } else {
    // If not dividing, display as a normal integer
    labelTempLen = snprintf(buffer, sizeof(buffer), "%s: %d", label, tempInt);
  }

  u8g2.drawUTF8(0, 10, buffer); // Adjust X, Y positions as needed

  // Calculate the width in pixels of the label and temperature part
  int pixelWidthPerChar = 12; // This is an estimated average width, adjust as needed
  int labelTempWidthPixels = labelTempLen * pixelWidthPerChar;

  // Set the font for the unit
  u8g2.setFont(u8g2_font_profont17_mf);
  snprintf(buffer, sizeof(buffer), "%s", unit);

  // Calculate the X position for the unit text
  int unitXPos = labelTempWidthPixels;
  u8g2.drawUTF8(unitXPos, 12, buffer); // Adjust the Y position as necessary
}

bool toggleValue = false;

void loop(void) {
  
  u8g2.firstPage();
  while(u8g2.nextPage()) {
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

    if (x_position > 488) { // Once the bitmap moves off the screen & extra time
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
            if(mode == 5) mode = 0;
          }
        }
      }
      
      lastButtonState = reading;

      if (!demoMode) {
        if (millis() - lastReceivedTime < requiredInterval) {
          if (mode == 1) {
            displayTemperature("IAT", emucan.emu_data.IAT, "C", false);
          } else if (mode == 2) {
            displayTemperature("OILT", emucan.emu_data.oilTemperature, "C", false);
          } else if (mode == 3) {
            displayTemperature("OILP", emucan.emu_data.oilPressure * 10, "Bar", true);
          } else if (mode == 4) {
            displayTemperature("FUEL", emucan.emu_data.fuelPressure * 10, "Bar", true);
          }
        } else {
          u8g2.setDrawColor(1);
          char buffer[20];
          snprintf(buffer, sizeof(buffer), "NO EMU");
          
          u8g2.drawUTF8(0, 6, buffer);
        }
      } else {
        if (mode == 1) {
          displayTemperature("IAT", 29, "C", false);
        } else if (mode == 2) {
          displayTemperature("OILT", 114, "C", false);
        } else if (mode == 3) {
          displayTemperature("OIL", 1.5 * 10, "Bar", true);
        } else if (mode == 4) {
          displayTemperature("GAS", 1.3 * 10, "Bar", true);
        }
      }
    } else {
      draw();
      x_position += 2; // Move the bitmap to the right
      delay(30);
    }
  }
}


/* 
* Links
* IAT
* OIL TEMP
* OIL PRESSURE
* FUEL PRESSURE
* 
* Rechts
* WATER TEMP CLT
* EGT1
* EGT2
* MAP
* 
**/
