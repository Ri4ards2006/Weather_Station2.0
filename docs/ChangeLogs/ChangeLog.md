
### Changelog  
All notable changes to the **WeatherStation 2.0 Arduino Master Node** project will be documented here. Versioning follows semantic versioning (`MAJOR.MINOR.PATCH`), where:  
- `MAJOR`: Breaking changes (e.g., hardware overhauls).  
- `MINOR`: New features or enhancements without breaking existing functionality.  
- `PATCH`: Bug fixes or minor optimizations.  

---

## v1.2.0 - [2024-10-01]  
**🚀 Major Update: Automated Alarm System & Interactive Controls**  

### New Features  
1. **Alarm System Implementation**  
   - Added **traffic light LEDs** (Red, Yellow, Green) for visual condition alerts:  
     - 🔴 *Critical State*: Activated when distance 40°C.  
     - 🟡 *Warning State*: Activated when distance <20cm (but ≥10cm).  
     - 🟢 *Normal State*: Default (Green LED on) when no conditions are breached.  
   - Integrated **buzzer** (active-high) for auditory feedback during critical alarms (1000Hz tone, 500ms duration).  

2. **Test Button Integration**  
   - Added tactile push button (pin 26) with internal pull-up resistor (`INPUT_PULLUP`).  
   - Manual alarm testing: Pressing the button triggers a test alarm (logs "🔔 Button gedrückt: Test-Alarm!", activates Red LED, sounds buzzer, resets to Green after 500ms).  

3. **Enhanced Serial Feedback**  
   - Added alarm activation messages (e.g., "🔔 Alarm aktiv: Entfernung zu gering!") to Serial Monitor.  
   - Improved setup logs with clearer status codes (e.g., "✅ BMP280 verbunden", "⚠️ RTC meldet Stromverlust").  

### Enhancements  
- **Hardware Documentation**:  
  - Expanded pinout table to include traffic light LEDs (pins 46, 48, 50), buzzer (pin 44), and test button (pin 26).  
  - Added wiring diagram reference (`docs/images/wiring_master_v1.2.png`) for precise hardware assembly.  

- **Firmware Structure**:  
  - Separated alarm logic into dedicated `triggerAlarm()` function, improving code readability.  
  - Clarified sensor data collection flow in `printSensorData()` (explicit thresholds and state transitions).  

- **Error Handling**:  
  - Added checks for LED/buzzer/button functionality during initialization (defaults to Green LED, all others off).  
  - Improved ultrasonic sensor error messaging: "Entfernung: keine Messung (Timeout)" instead of generic errors.  

### Bug Fixes  
- **BMP280 Initialization Redundancy**: Fixed redundant `begin()` call in `printSensorData()`, ensuring stable sensor communication.  
- **LED Pin Labeling**: Corrected pin comments for traffic lights (previously referenced incorrect pin numbers).  

### Documentation Updates  
- Added "Sensor & Alarm Logic" section to README, detailing triggers and responses.  
- Updated "Troubleshooting Tips" with alarm-specific issues (e.g., "LED/Buzzer not responding → check pin connections").  
- Included future roadmap for v1.2 (TFT integration, cloud MQTT support, OTA updates).  

---

## v1.1.0 - [2024-05-20]  
**🔍 Core Sensor Data Logging & RTC Integration**  

### New Features  
1. **Multi-Sensor Support**  
   - **BMP280 (I2C)**: Measures temperature (°C) and atmospheric pressure (hPa).  
   - **HC-SR04 Ultrasonic**: Distance measurement (cm) with 25ms timeout handling.  
   - **DS3231 RTC (I2C)**: Real-time clock for accurate timestamps (supports battery backup via CR2032).  

2. **Serial Data Output**  
   - Periodic (every 2s) sensor data logging to Serial Monitor (115200 baud) with formatted symbols (🌡, 🌬, 📊).  
   - Optional `Serial1` output for raw data forwarding (e.g., "TEMP:25.3", "PRESS:1012.8").  

3. **RTC Time Synchronization**  
   - Added `parseAndSetRTC()` function to handle "SET:YYYY-MM-DD HH:MM:SS" serial commands for manual time setting.  
   - Logs success/error messages for RTC adjustments (e.g., "✅ RTC gesetzt via SET-Befehl").  

### Enhancements  
- **I2C Bus Management**:  
  - Explicit error checking for BMP280 and RTC initialization (`setup()` logs "❌ BMP280 nicht gefunden!" if missing).  
  - Automatic calculation of slave count via `sizeof(slaveAddrs)` (though v1.1 focused on single master, not slaves).  

- **Hardware Setup**:  
  - Defined I2C pins (SDA/SCL) and ultrasonic TRIG/ECHO pins (52/53) in `setup()`.  
  - Added basic pin mode configuration (OUTPUT for sensors, INPUT for button).  

### Limitations (Addressed in v1.2)  
- No visual/auditory feedback for critical conditions (data logs only).  
- Alarm testing required environmental changes (no manual button).  
- Alarm logic not automated (manual threshold checks needed).  

---

## v1.0.0 - [2024-01-15] (Original School Project)  
**Prototype: Basic Wireless Sensor Network**  

### Core Functionality  
- Single-node weather station prototype.  
- **DHT11 Sensor**: Basic temperature (°C) measurement (low precision, ±2°C).  
- **Wireless Communication**: Unstable NRF24L01+ integration (high latency, packet loss).  
- **Serial Output**: Raw data logs without formatting or timestamps (e.g., "Temp: 22").  

### Known Issues  
- Wireless module (NRF24L01+) unreliable for multi-node setups.  
- No RTC → Time tracked via `millis()`, leading to drift and no date context.  
- No error handling for sensor disconnections or wiring issues.  
- No fallback displays (only serial output).  

### Deprecated Features  
- Wireless communication (replaced in v1.1 with focus on stability; v1.2 removes it entirely for master node).  
- DHT11 (upgraded to BMP280 in v1.1 for higher precision).  

---

## v0.9.0 - [2023-11-01] (Pre-School Project)  
**Early Exploration: Analog Sensors & Basic Display**  

### Features  
- **Analog Light Sensor**: Simple photoresistor for light intensity (raw analog readings).  
- **7-Segment Display**: Numeric output for temperature (limited to integer values).  
- **No I2C**: All sensors connected via analog/digital pins (no RTC, no BMP280).  

### Limitations  
- No modular design (hardcoded sensor support).  
- No serial debugging → Data validation via physical display only.  
- No backup power → System resets frequently, losing data context.  

---

## How to Read This Changelog  
- Use `[YYYY-MM-DD]` dates to track release timelines.  
- Prioritize changes in `v1.2.0` for current users, as it’s the latest stable version.  
- For historical context, refer to older versions (e.g., v1.0.0’s unstable wireless).  

--- 

This changelog provides a clear timeline of improvements, making it easy for users to understand what’s new in v1.2 and how the project has evolved. It highlights technical advancements, user-centric features, and resolved limitations—perfect for your GitHub repository’s "what’s changed" overview! 📊✨
