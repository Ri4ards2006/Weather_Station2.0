// ==============================================
// WeatherStation 2.0 v1.2 - Master Sensor & Alarm System
// ==============================================
// Purpose: This Arduino sketch integrates multiple sensors (BMP280 for temperature/pressure, 
//          ultrasonic for distance) with an RTC (real-time clock) to log and display environmental data. 
//          It also includes an alarm system (traffic lights + buzzer) triggered by specific conditions 
//          (e.g., close distance or high temperature) and a button for manual alarm testing.
//          Designed for weather monitoring or environmental safety systems with time-stamped data.

// Hardware Requirements:
//   - Arduino (supports I2C, Serial, and GPIO pins)
//   - BMP280 I2C Sensor (temperature/pressure, address 0x76)
//   - DS3231 RTC Module (I2C, address 0x68)
//   - Ultrasonic Distance Sensor (TRIG=52, ECHO=53)
//   - Traffic Light LEDs (Green=50, Yellow=48, Red=46)
//   - Buzzer (44)
//   - Push Button (26, connected with pull-up resistor or INPUT_PULLUP)
// ==============================================

// ---------------------------
// 1. Required Library Inclusions
// ---------------------------
// Wire.h: Enables I2C (TWI) communication for BMP280 and RTC modules.
#include <Wire.h>        

// Adafruit_BMP280.h: Specialized library for the BMP280 sensor. Handles I2C commands, data reading, 
//                    and conversion to temperature (°C) and pressure (hPa).
#include <Adafruit_BMP280.h> 

// RTClib.h: Library for the DS3231 RTC module. Manages timekeeping, date validation, and manual time adjustment.
#include <RTClib.h>     
// ==============================================

// ---------------------------
// 2. Sensor & Peripheral Definitions
// ---------------------------
// BMP280 Sensor Object: Manages communication with the temperature/pressure sensor.
Adafruit_BMP280 bmp;       

// DS3231 RTC Object: Manages communication with the real-time clock module.
RTC_DS3231 rtc;            
// ==============================================

// ---------------------------
// 3. Pin Definitions (Ultrasonic Sensor)
// ---------------------------
// Ultrasonic Trigger Pin (TRIG): Sends a short pulse to start distance measurement (OUTPUT).
#define TRIG_PIN 43     
// Ultrasonic Echo Pin (ECHO): Receives the reflected pulse from the measured object (INPUT).
#define ECHO_PIN 42   
// ==============================================

// ---------------------------
// 4. Pin Definitions (Traffic Light + Buzzer)
// ---------------------------
// Traffic Light LEDs (OUTPUT Pins):
// - Green LED: Signals normal conditions (LOW = off, HIGH = on)
#define LED_GREEN  51     
// - Yellow LED: Signals warning conditions (LOW = off, HIGH = on)
#define LED_YELLOW 50     
// - Red LED: Signals critical conditions (LOW = off, HIGH = on)
#define LED_RED    49     

// Buzzer Pin (OUTPUT): Sounds alarms when triggered (LOW = off, HIGH = on via tone())
#define BUZZER     53      
// ==============================================

// ---------------------------
// 5. Pin Definition (Test Button)
// ---------------------------
// Test Button Pin (INPUT_PULLUP): Manual button to trigger a test alarm. 
//                                INPUT_PULLUP → Pin is HIGH when not pressed, LOW when pressed.
#define BUTTON_PIN 26       
// ==============================================

// ---------------------------
// 6. Time Management Variables
// ---------------------------
// 'lastUpdate': Timestamp (ms since startup) of the last full sensor data cycle. 
//               Tracks when the next data collection/update is needed.
unsigned long lastUpdate = 0;  

// 'updateInterval': Interval (ms) between sensor data cycles. Set to 2000ms (2 seconds). 
//                   Adjust based on sensor responsiveness or data logging needs.
const unsigned long updateInterval = 2000;  
// ==============================================

// ---------------------------
// 7. Function Prototypes (Pre-declarations)
// ---------------------------
// measureDistanceCM(): Reads distance from ultrasonic sensor (cm). Returns -1 on timeout.
float measureDistanceCM();   

// printSensorData(): Collects sensor data, formats it, and prints to Serial/Serial1.
void printSensorData();      

// triggerAlarm(): Controls traffic lights and buzzer based on distance, temperature, and pressure.
void triggerAlarm(float distance, float temperature, float pressure);  
// ==============================================

// ---------------------------
// 8. parseAndSetRTC() - Helper Function
// ---------------------------
// Parses a serial string in "SET:YYYY-MM-DD HH:MM:SS" format and sets the RTC time.
// Input: 's' → String with the SET command (e.g., "SET:2024-05-20 14:30:00")
// Output: 'true' if RTC time was updated successfully; 'false' on invalid format/values.
bool parseAndSetRTC(const String &s) {
  // Step 1: Validate command starts with "SET:" prefix
  if (!s.startsWith("SET:")) return false;  
  // Step 2: Extract body (remove "SET:" prefix)
  String body = s.substring(4);              
  // Step 3: Check body length (must be exactly 19 characters for "YYYY-MM-DD HH:MM:SS")
  if (body.length() < 19) return false;       

  // Step 4: Extract and convert individual date/time components
  int Y = body.substring(0,4).toInt();    // Year (e.g., "2024" → 2024)
  int m = body.substring(5,7).toInt();   // Month (e.g., "05" → May)
  int d = body.substring(8,10).toInt();  // Day (e.g., "20" → 20th)
  int H = body.substring(11,13).toInt(); // Hour (e.g., "14" → 2 PM)
  int M = body.substring(14,16).toInt(); // Minute (e.g., "30" → 30 mins)
  int S = body.substring(17,19).toInt(); // Second (e.g., "00" → 0 secs)

  // Step 5: Validate components (basic checks to avoid invalid dates/times)
  // Year: Minimum 2000 (adjust if needed), Month: 1-12, Day: 1-31 (simplified), 
  // Hour: 0-23, Minute/Sekunde: 0-59
  if (Y < 2000 || m < 1 || m > 12 || d < 1 || d > 31 || 
      H < 0 || H > 23 || M < 0 || M > 59 || S < 0 || S > 59) {
    return false; // Invalid values → return false
  }

  // Step 6: Create DateTime object and update RTC
  DateTime dt(Y, m, d, H, M, S);  // Compose date/time
  rtc.adjust(dt);                 // Set RTC to this time
  return true;                    // Success → return true
}
// ==============================================

// ---------------------------
// 9. setup() - Initial Setup (Runs Once at Startup)
// ---------------------------
// Initializes hardware, sensors, and default states.
void setup() {
  // ---------------------------
  // Serial Communication Setup
  // ---------------------------
  // Standard Serial (Serial): 115200 Baud for user feedback/debugging.
  Serial.begin(115200);           
  // Serial1: 9600 Baud (optional) for external devices (e.g., TFT display, Raspberry Pi).
  Serial1.begin(9600);            

  // ---------------------------
  // I2C Master Initialization
  // ---------------------------
  // Enables Arduino as I2C Master to communicate with BMP280 and RTC.
  Wire.begin();                   

  // ---------------------------
  // Welcome Message
  // ---------------------------
  Serial.println(F("🌦 Wetterstation v1.2 startet..."));  // F() → Reduces Flash memory usage (string literals)

  // ---------------------------
  // BMP280 Sensor Check
  // ---------------------------
  // Attempts to initialize BMP280 at I2C address 0x76. 
  // Returns 'false' if sensor not found (e.g., wiring issues, wrong address).
  if (!bmp.begin(0x76)) {         
    Serial.println(F("❌ BMP280 nicht gefunden!")); // Error message
  } else {
    Serial.println(F("✅ BMP280 verbunden"));       // Success message
  }

  // ---------------------------
  // RTC Module Check
  // ---------------------------
  // Attempts to initialize DS3231 RTC. Returns 'false' if module not found (e.g., SDA/SCL wiring issues).
  if (!rtc.begin()) {             
    Serial.println(F("❌ RTC-Modul nicht gefunden! Prüfe SDA/SCL und VCC/GND")); // Error message with troubleshooting tips
  } else {
    Serial.println(F("✅ RTC verbunden"));          // Success message
  }

  // ---------------------------
  // RTC Power Loss Check
  // ---------------------------
  // Checks if RTC lost power (e.g., backup battery depleted). Time may be inaccurate in this case.
  if (rtc.lostPower()) {          
    Serial.println(F("⚠️ RTC meldet Stromverlust (lostPower()). Zeit ggf. nicht gesetzt.")); // Warning message
  }

  // ---------------------------
  // Ultrasonic Sensor Pins
  // ---------------------------
  // Configures TRIG as OUTPUT (to send pulses)
  pinMode(TRIG_PIN, OUTPUT);      
  // Configures ECHO as INPUT (to read reflected pulses)
  pinMode(ECHO_PIN, INPUT);        

  // ---------------------------
  // Traffic Light & Buzzer Pins
  // ---------------------------
  // All LEDs and buzzer set as OUTPUT for control
  pinMode(LED_RED, OUTPUT);    
  pinMode(LED_YELLOW, OUTPUT); 
  pinMode(LED_GREEN, OUTPUT);  
  pinMode(BUZZER, OUTPUT);      

  // ---------------------------
  // Test Button Setup
  // ---------------------------
  // Configures BUTTON_PIN as INPUT_PULLUP (internal pull-up resistor enabled). 
  // Button → LOW when pressed, HIGH when released.
  pinMode(BUTTON_PIN, INPUT_PULLUP);  

  // ---------------------------
  // Initial LED/Buzzer State
  // ---------------------------
  // All LEDs off initially, then green turned on (default "normal" state)
  digitalWrite(LED_RED, LOW);       
  digitalWrite(LED_YELLOW, LOW);    
  digitalWrite(LED_GREEN, HIGH);    // Green LED signals normal conditions
  // Buzzer off initially
  digitalWrite(BUZZER, LOW);        

  // ---------------------------
  // Setup Completion Message
  // ---------------------------
  Serial.println(F("Setup abgeschlossen."));  
  Serial.println(F("Drücke Button zum Testen des Alarms.")); // Informs user about manual alarm test option
  Serial.println();  // Empty line for readability
}
// ==============================================

// ---------------------------
// 10. loop() - Main Program Loop (Runs Infinitely)
// ---------------------------
// Main loop: Handles button input, serial commands, periodic sensor data collection, 
//            and alarm system updates.
void loop() {
  // ---------------------------
  // Manual Alarm Test via Button
  // ---------------------------
  // Checks if button is pressed (digitalRead returns LOW when pressed, HIGH when released)
  if (digitalRead(BUTTON_PIN) == LOW) {  
    Serial.println("🔔 Button gedrückt: Test-Alarm!"); // Feedback message for user
    // Temporarily set LEDs to test alarm state (red on)
    digitalWrite(LED_RED, HIGH);     
    digitalWrite(LED_YELLOW, LOW);   
    digitalWrite(LED_GREEN, LOW);    
    // Sound buzzer at 1000Hz for 500ms (alarm test tone)
    tone(BUZZER, 1000, 500);          
    delay(500);  // Wait for tone to complete
    // Reset LEDs to normal (green on)
    digitalWrite(LED_RED, LOW);      
    digitalWrite(LED_GREEN, HIGH);   
  }

  // ---------------------------
  // Process Serial Commands (RTC Time Set)
  // ---------------------------
  // Checks if serial data is available (e.g., user input via Serial Monitor)
  if (Serial.available()) {          
    String line = Serial.readStringUntil('\n');
    line.trim();

    // Validates line is not empty after trimming
    if (line.length() > 0) {         
      // Attempts to parse and set RTC time using the SET command
      if (parseAndSetRTC(line)) {     
        Serial.println(F("✅ RTC gesetzt via SET-Befehl.")); // Success message
      } else {
        // Error message for invalid SET format/values
        Serial.println(F("❌ Ungültiges SET-Format oder Befehl. Verwende: SET:YYYY-MM-DD HH:MM:SS"));  
      }
    }
  }

  // ---------------------------
  // Periodic Sensor Data Collection
  // ---------------------------
  // Checks if 'updateInterval' (2 seconds) has passed since last data collection
  if (millis() - lastUpdate >= updateInterval) {  
    lastUpdate = millis();  // Updates timestamp to current time (ms)
    printSensorData();      // Triggers data collection and display
  }
}
// ==============================================

// ---------------------------
// 11. measureDistanceCM() - Ultrasonic Sensor Function
// ---------------------------
// Measures distance using the HC-SR04 (or similar) ultrasonic sensor. 
// Principle: Sends a trigger pulse, measures echo duration, calculates distance via sound speed.
// Returns: Distance in cm (≥0 → valid; -1 → timeout/error).
float measureDistanceCM() {
  // Step 1: Wait for sensor to settle (sets TRIG to LOW)
  digitalWrite(TRIG_PIN, LOW);       
  delayMicroseconds(3);  // Short delay (3µs) before trigger

  // Step 2: Send trigger pulse (10µs HIGH signal)
  digitalWrite(TRIG_PIN, HIGH);      
  delayMicroseconds(10);  // Trigger pulse duration (10µs)
  digitalWrite(TRIG_PIN, LOW);       // Stop trigger pulse

  // Step 3: Measure echo duration (time sensor waits for HIGH signal on ECHO pin)
  // pulseIn(pin, value, timeout): Returns duration (µs) of 'value' (HIGH) on 'pin'. Timeout in µs (25ms here).
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);  

  // Step 4: Handle timeout (no echo received)
  if (duration == 0) {                  
    return -1;  // No valid measurement → return -1
  }

  // Step 5: Calculate distance (cm). Formula: distance = (duration * soundSpeed) / 2
  // Sound speed in air (20°C): ~343 m/s → 0.0343 cm/µs. Divide by 2 (echo travels to object and back).
  return (duration * 0.0343) / 2.0;     
}
// ==============================================

// ---------------------------
// 12. printSensorData() - Data Collection & Display Function
// ---------------------------
// Collects data from BMP280, ultrasonic sensor, and RTC, formats it, and prints to Serial/Serial1.
void printSensorData() {
  // BMP280 Data Variables
  float temperature = NAN;  // Temperature (°C). NAN = "Not a Number" (invalid)
  float pressure = NAN;      // Pressure (hPa). NAN = invalid

  // ---------------------------
  // BMP280 Data Read
  // ---------------------------
  // Re-initializes BMP280 (in case it was disconnected/reconnected). 
  // Note: 'begin()' may not be necessary here, but included for robustness.
  if (bmp.begin(0x76)) {     
    // Reads raw temperature data, converts to °C.
    temperature = bmp.readTemperature();  
    // Reads raw pressure data (Pa), converts to hPa (divide by 100).
    pressure = bmp.readPressure() / 100.0F;  
  }

  // ---------------------------
  // Ultrasonic Data Read
  // ---------------------------
  float distance = measureDistanceCM();  // Measures distance (cm)

  // ---------------------------
  // RTC Time Read
  // ---------------------------
  DateTime now = rtc.now();  // Gets current time from RTC
  // Formats time into "HH:MM:SS" string (e.g., "14:30:00") using snprintf()
  char timeString[20];      
  snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", 
           now.hour(), now.minute(), now.second());  

  // ---------------------------
  // Serial Data Output (Debug/View)
  // ---------------------------
  Serial.println(F("----------- Messdaten -----------"));  // Header line

  // ---------------------------
  // Temperature & Pressure
  // ---------------------------
  // Checks if temperature is valid (not NAN)
  if (!isnan(temperature)) {  
    // Prints temperature with 1 decimal place (e.g., "🌡 Temperatur: 23.5 °C")
    Serial.print(F("🌡 Temperatur: ")); Serial.print(temperature, 1); Serial.println(F(" °C"));  
    // Prints pressure with 1 decimal place (e.g., "🌬 Luftdruck:  1013.2 hPa")
    Serial.print(F("🌬 Luftdruck:  ")); Serial.print(pressure, 1); Serial.println(F(" hPa"));  
  } else {
    // Prints error message if BMP280 data is invalid (not connected/failed)
    Serial.println(F("🌡 BMP280: nicht verfügbar"));  
  }

  // ---------------------------
  // Ultrasonic Distance
  // ---------------------------
  // Checks if distance is valid (≥0)
  if (distance >= 0) {        
    // Prints distance with 1 decimal place (e.g., "📏 Entfernung: 50.5 cm")
    Serial.print(F("📏 Entfernung: ")); Serial.print(distance, 1); Serial.println(F(" cm"));  
  } else {
    // Prints timeout/error message if no valid distance measured
    Serial.println(F("📏 Entfernung: keine Messung (Timeout)"));  
  }

  // ---------------------------
  // RTC Time Display
  // ---------------------------
  // Prints formatted time string (e.g., "⏰ Zeit:       14:30:00")
  Serial.print(F("⏰ Zeit:       ")); Serial.println(timeString);  

  // Trennlinie unten (Footer)
  Serial.println(F("--------------------------------\n"));  

  // ---------------------------
  // Optional Data Output to Serial1 (External Devices)
  // ---------------------------
  // Sends raw sensor data to Serial1 (e.g., for a TFT display or gateway)
  Serial1.print(F("TEMP:")); Serial1.println(temperature);  
  Serial1.print(F("PRESS:")); Serial1.println(pressure);    
  Serial1.print(F("DIST:")); Serial1.println(distance);      
}

// ---------------------------
// 13. triggerAlarm() - Alarm Logic Function
// ---------------------------
// Automatically controls traffic lights and buzzer based on sensor data thresholds.
// Inputs: distance (cm), temperature (°C), pressure (hPa)
void triggerAlarm(float distance, float temperature, float pressure) {
  // ---------------------------
  // Default State (Normal Conditions)
  // ---------------------------
  // Resets all LEDs to low, then sets green LED to HIGH (default "safe" state)
  digitalWrite(LED_RED, LOW);       
  digitalWrite(LED_YELLOW, LOW);    
  digitalWrite(LED_GREEN, HIGH);    
  // Buzzer off by default
  digitalWrite(BUZZER, LOW);        

  // ---------------------------
  // Critical Alarm Conditions
  // ---------------------------
  // Checks if either:
  // - Distance is valid (≥0) AND less than 10cm (very close object)
  // - Temperature is valid (not NAN) AND higher than 40°C (extreme heat)
  if ((distance >= 0 && distance < 10) || (!isnan(temperature) && temperature > 40)) {  
    // Critical state → Red LED on, Green off, Buzzer sound
    digitalWrite(LED_RED, HIGH);    
    digitalWrite(LED_GREEN, LOW);   
    tone(BUZZER, 1000, 500);         // Buzzer at 1000Hz for 500ms (alarm tone)
  } 

  // ---------------------------
  // Warning Conditions (Non-Critical)
  // ---------------------------
  // Checks if distance is valid (≥0) AND less than 20cm (close but not critical)
  else if (distance >= 0 && distance < 20) {  
    // Warning state → Yellow LED on, Green off
    digitalWrite(LED_YELLOW, HIGH); 
    digitalWrite(LED_GREEN, LOW);   
  } 

  // ---------------------------
  // Normal Conditions (No Action)
  // ---------------------------
  // If none of the above → Green LED remains on (handled in default state above)
  // else block ensures no redundant code (default state already sets Green HIGH)
  // else {
  //   digitalWrite(LED_GREEN, HIGH); // Redundant, already set in default
  // }
}