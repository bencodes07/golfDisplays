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

  // Button and Toggle Variables
  unsigned long lastDebounceTime = 0;
  unsigned long lastToggleDebounceTime = 0;
  int lastButtonState = HIGH;
  int buttonState = HIGH;
  int lastToggleButtonState = HIGH;
  int toggleButtonState = HIGH;
  const long debounceDelay = 50; // 50 ms debounce time

  // LC value variables - only kept for output pin
  bool toggleValue = true;
  bool previousToggleValue = true;

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
    
    // Initialize CAN 
    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp2515.setNormalMode();
    
    u8g2_prepare();
    pinMode(7, INPUT_PULLUP);  // Mode button
    pinMode(5, INPUT_PULLUP);  // Toggle button
    pinMode(6, OUTPUT);        // Output pin
    
    Serial.println("Setup complete");
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

  void processCanMessages() {
      // Check for new CAN messages (do this as often as possible)
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
  }

  void handleButtons() {
      // Read mode button (display modes)
      int reading = digitalRead(7);
      
      if (reading != lastButtonState) {
          lastDebounceTime = millis();
      }
      
      if ((millis() - lastDebounceTime) > debounceDelay) {
          if (reading != buttonState) {
              buttonState = reading;
              if (buttonState == LOW) {
                  mode++;
                  if (mode == 6) mode = 1;
                  Serial.print("Mode changed to: ");
                  Serial.println(mode);
              }
          }
      }
      
      lastButtonState = reading;
      
      // Read toggle button (kept for manual override if needed)
      int toggleReading = digitalRead(5);
    
      if (toggleReading != lastToggleButtonState) {
          lastToggleDebounceTime = millis();
      }
      
      if ((millis() - lastToggleDebounceTime) > debounceDelay) {
          if (toggleReading != toggleButtonState) {
              int oldToggleButtonState = toggleButtonState;
              toggleButtonState = toggleReading;
              
              if (oldToggleButtonState == HIGH && toggleButtonState == LOW) {
                  toggleValue = !toggleValue;
                  digitalWrite(6, !toggleValue);
                  Serial.print("Manual Button Toggle: ");
                  Serial.println(toggleValue ? "LC1" : "LC2");
              }
          }
      }
      
      lastToggleButtonState = toggleReading;
  }

  void loop(void) {
      // Check for CAN messages frequently - do this outside the display loop!
      processCanMessages();
      
      // Handle button inputs
      handleButtons();
      
      // Update display at regular intervals
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
          
          u8g2.firstPage();
          do {
              if (x_position > 488) { // Once the bitmap moves off the screen & extra time
                  if (!demoMode) {
                      if (millis() - lastReceivedTime < requiredInterval) {
                          // Display based on mode
                          if (mode == 1) {
                              displayTemperature("IAT", emucan.emu_data.IAT, "C", false);
                          } else if (mode == 2) {
                              displayTemperature("OILT", emucan.emu_data.oilTemperature, "C", false);
                          } else if (mode == 3) {
                              displayTemperature("OILP", emucan.emu_data.oilPressure * 10, "Bar", true);
                          } else if (mode == 4) {
                              displayTemperature("FUEL", emucan.emu_data.fuelPressure * 10, "Bar", true);
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
                          displayTemperature("IAT", 29, "C", false);
                      } else if (mode == 2) {
                          displayTemperature("OILT", 114, "C", false);
                      } else if (mode == 3) {
                          displayTemperature("OIL", 15, "Bar", true);
                      } else if (mode == 4) {
                          displayTemperature("GAS", 13, "Bar", true);
                      } else if (mode == 5) {
                          // Mode 5 - Display off (just draw nothing)
                      }
                  }
              } else {
                  draw();
              }
          } while (u8g2.nextPage());
          
          if (x_position <= 488) {
              x_position += 3; // Move the bitmap to the right
              delay(30);
          }
      }
  }