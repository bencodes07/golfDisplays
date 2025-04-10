#include <Arduino.h>
#include <U8g2lib.h>

// EmuCan
#include "EMUcan.h"
EMUcan emucan(0x600); // Base ID 600 --> See EMU settings

#include <mcp2515.h>
struct can_frame canMsg;
MCP2515 mcp2515(10); // CS Pin 10

unsigned long previousMillis = 0;
const long interval = 100; // Reduced from 500ms to 100ms for faster updates

// NO EMU Display
unsigned long lastReceivedTime = 0; // Timer to track last received message time
const unsigned long requiredInterval = 5000; // 5 seconds interval

const bool demoMode = false;

// LC value variables - only kept for output pin
bool toggleValue = false;
bool previousToggleValue = false;

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
  
  // Initialize CAN 
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  
  pinMode(7, INPUT_PULLUP);  // Mode button
  pinMode(6, OUTPUT);        // Output pin
  pinMode(5, INPUT_PULLUP);  // Toggle button (kept for compatibility)
  
  u8g2_prepare();
  delay(6300);
  
  Serial.println("Setup complete");
}

void displayTemperature(const char* label, int16_t temp, const char* unit) {
    char buffer[20];

    u8g2.setFont(u8g2_font_profont22_mf);
    int labelTempLen = snprintf(buffer, sizeof(buffer), "%s: %d", label, temp);
    u8g2.drawUTF8(0, 8, buffer);

    int pixelWidthPerChar = 12;
    int labelTempWidthPixels = labelTempLen * pixelWidthPerChar;

    u8g2.setFont(u8g2_font_profont17_mf);
    snprintf(buffer, sizeof(buffer), "%s", unit);

    int unitXPos = labelTempWidthPixels;
    u8g2.drawUTF8(unitXPos, 10, buffer);
  
}

void loop(void) {
    // Check for CAN messages frequently
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
        emucan.checkEMUcan(canMsg.can_id, canMsg.can_dlc, canMsg.data);
        if (emucan.EMUcan_Status() == EMUcan_RECEIVED_WITHIN_LAST_SECOND) {
            lastReceivedTime = millis();
            
            // Check for CAN switch 4 status (outflags2)
            bool currentToggleValue = (emucan.emu_data.outflags2 & EMUcan::F_CANSW4) > 0;
            
            // If LC value changed, update output
            if (currentToggleValue != previousToggleValue) {
                toggleValue = currentToggleValue;
                previousToggleValue = currentToggleValue;
                digitalWrite(6, !toggleValue);
                
                Serial.print("LC Changed from ECU: ");
                Serial.println(toggleValue ? "LC1" : "LC2");
            }
        }
    }
    
    // Update display at regular intervals
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        u8g2.firstPage();
        while (u8g2.nextPage()) {
            // Handle toggle button within the display loop (like in original code)
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

            if (x_position > 168) { // Once the bitmap moves off the screen
                // Handle mode button
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
                            if(mode == 6) mode = 1;
                        }
                    }
                }
                
                lastButtonState = reading;

                if (!demoMode) {
                    if (millis() - lastReceivedTime < requiredInterval) {
                        // Display based on mode
                        if (mode == 1) {
                            displayTemperature("CLT", emucan.emu_data.CLT, "C");
                        } else if (mode == 2) {
                            displayTemperature("EGT1", emucan.emu_data.Egt1, "C");
                        } else if (mode == 3) {
                            displayTemperature("EGT2", emucan.emu_data.Egt2, "C");
                        } else if (mode == 4) {
                            displayTemperature("MAP", emucan.emu_data.MAP, "kPa");
                        } else if (mode == 5) {
                            // Mode 5 - Display off (just draw nothing)
                        }
                    } else {
                        u8g2.setDrawColor(1);
                        u8g2.setFont(u8g2_font_profont22_mf);
                        u8g2.drawUTF8(0, 8, "NO EMU");
                    }
                } else {
                    // Demo mode display
                    if (mode == 1) {
                        displayTemperature("CLT", 102, "C");
                    } else if (mode == 2) {
                        displayTemperature("EGT1", 756, "C");
                    } else if (mode == 3) {
                        displayTemperature("EGT2", 897, "C");
                    } else if (mode == 4) {
                        displayTemperature("MAP", 215, "kPa");
                    } else if (mode == 5) {
                        // Mode 5 - Display off (just draw nothing)
                    }
                }
            } else {
                draw();
            }
        }
        
        if (x_position <= 168) {
            x_position += 3; // Move the bitmap to the right
            delay(30);
        }
    }
}