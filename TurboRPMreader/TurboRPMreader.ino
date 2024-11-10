const int inputPin = 8;  // Digital pin to read from
unsigned long lastTime = 0;
unsigned long period = 0;

void setup() {
  pinMode(inputPin, INPUT);
  Serial.begin(115200);  // High baud rate for faster printing
}

void loop() {
  // Wait for rising edge
  while(!(PINB & (1 << (inputPin - 8)))); 
  unsigned long startTime = micros();
  
  // Wait for falling edge
  while((PINB & (1 << (inputPin - 8))));
  
  period = micros() - startTime;
  
  // Calculate frequency
  float frequency = 10000000.0 / (period * 2);  // Convert to Hz
  float rpm = (frequency * 60) / 14;
  
  // Print every 100ms
  if(millis() - lastTime > 100) {
    Serial.print("RPM: ");
    Serial.print(rpm);
    Serial.println(" rpm");
    lastTime = millis();
  }
}