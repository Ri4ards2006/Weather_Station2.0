
# 🌦 WeatherStation 2.0 - Arduino Master Node (v1.2)  
**Real-Time Environmental Monitoring with Automated Alarms**  

---

## 1️⃣ Motivation / Background  
**WeatherStation 2.0** started as a school project to build a stable weather monitoring system, but early versions struggled with unreliable wireless communication. Today, this project evolves into a **modular, scalable, and user-centric platform** focused on precise data collection and actionable insights.  

This repository holds the firmware for the **Arduino Master Node v1.2**, the core hub of the system. It aggregates data from sensors (temperature, pressure, distance), tracks time via an RTC, and introduces an **automated alarm system** to alert users of critical conditions—all while maintaining compatibility with serial-based debugging and optional external displays.  

🚀 **Vision**: Create a network of nodes that monitor indoor/outdoor environments, scale to multiple locations, and integrate with smart home/iot ecosystems.  

---

## 2️⃣ Project Overview (v1.2)  
### What It Does  
The Master Node v1.2 is an Arduino-based system that:  
- **Polls Sensors**: Reads temperature (°C), atmospheric pressure (hPa), and object distance (cm) via BMP280 and HC-SR04.  
- **Tracks Time**: Uses a DS3231 RTC for accurate timestamps, with backup battery resilience.  
- **Displays Data**: Outputs formatted sensor readings to the Serial Monitor (115200 baud) every 2 seconds.  
- **Triggers Alarms**: Activates visual (traffic lights) and auditory (buzzer) alerts when thresholds are breached.  
- **Supports Manual Tests**: A physical button lets users validate the alarm system without environmental changes.  

### Key Improvements from v1.1  
v1.1 focused on basic data logging. v1.2 adds:  
- **Alarm System**: Critical (Red) and warning (Yellow) states with buzzer feedback.  
- **Interactive Controls**: Test button for alarm validation and RTC time sync via serial commands.  
- **Automated Logic**: No manual checks needed—alarms trigger automatically based on sensor data.  

---

## 3️⃣ Display & Data Output  
### Primary: Serial Monitor  
Data is displayed in a human-readable format via the Arduino Serial Monitor (115200 baud). Example output:  

```plaintext
🌦 Wetterstation v1.2 startet...
✅ BMP280 verbunden
✅ RTC verbunden
Setup abgeschlossen.
Drücke Button zum Testen des Alarms.

----------- Messdaten -----------
🌡 Temperatur: 25.3 °C
🌬 Luftdruck:  1012.8 hPa
📏 Entfernung: 22.1 cm
⏰ Zeit:       09:30:05
--------------------------------

// During Critical Alarm (distance 40°C):
🔔 Alarm aktiv: Entfernung zu gering!
----------- Messdaten -----------
🌡 Temperatur: 25.3 °C
🌬 Luftdruck:  1012.8 hPa
📏 Entfernung: 8.9 cm
⏰ Zeit:       09:30:07
--------------------------------
```  

### Optional: Serial1 Forwarding  
Raw sensor data (e.g., `TEMP:25.3`, `PRESS:1012.8`) is sent to Serial1 (9600 baud) for external devices like:  
- TFT displays (via `Adafruit_ST7735` or similar libraries).  
- Raspberry Pi gateways for cloud integration.  
- Custom dashboards or data loggers.  

---

## 4️⃣ Hardware Requirements  
### Required Components  
| Component               | Model/Type          | Function                          | Arduino Pin | Notes                                   |
|-------------------------|---------------------|-----------------------------------|-------------|-----------------------------------------|
| Arduino Controller       | Arduino Mega/Uno    | Core microcontroller              | -           | Mega recommended (extra GPIO pins).     |
| BMP280 Sensor            | BMP280 (I2C)        | Temperature/pressure sensing      | SDA, SCL    | I2C address: 0x76 (default).            |
| RTC Module               | DS3231 (I2C)        | Real-time clock (time sync)       | SDA, SCL    | I2C address: 0x68 (default). Battery-backed. |
| Ultrasonic Sensor        | HC-SR04             | Distance measurement (non-contact) | 52 (TRIG), 53 (ECHO) | Max range ~400cm; 25ms timeout.         |
| Traffic Light LEDs       | -                   | Visual alarm indicators            | 46 (Red), 48 (Yellow), 50 (Green) | Active-high (HIGH = ON). Default: Green. |
| Buzzer                   | Active Buzzer       | Auditory alerts                   | 44          | Sounds 1000Hz tone for 500ms during alarms. |
| Test Button              | Tactile Button       | Manual alarm testing              | 26          | Wired to GND (INPUT_PULLUP). LOW = pressed. |  

### Schematic & Wiring  
![Master Node Wiring](docs/images/wiring_master_v1.2.png)  
*(Fritzing diagram showing I2C connections, sensor pins, and LED/buzzer/button wiring)*  

---

## 5️⃣ Sensor & Alarm Logic  
### Supported Sensors  
- **BMP280**: Industrial-grade sensor for temperature (±0.5°C) and pressure (±0.1 hPa).  
- **HC-SR04 Ultrasonic**: Measures distance via sound pulses (0-400cm, 25ms timeout). Returns `-1` on no echo.  
- **DS3231 RTC**: Ultra-accurate clock (±2ppm drift) with battery backup (CR2032) for power loss resilience.  

### Alarm Triggers & Responses  
The system evaluates sensor data every 2 seconds and activates alerts:  

| Condition                          | Visual Alert       | Auditory Alert          |
|------------------------------------|--------------------|-------------------------|
| **Critical**: Distance 40°C | Red LED (46) ON     | Buzzer (44) → 1000Hz/500ms |
| **Warning**: Distance 400cm) or sensor disconnected.  
  - Adjust `pulseIn` timeout (currently 25000µs) in `measureDistanceCM()` for longer ranges.  
- **LED/Buzzer Not Responding**: Check pin connections and ensure LEDs have current-limiting resistors (if required).  

---

## 9️⃣ License & Credits  
- **Code**: MIT License (see [LICENSE](LICENSE)).  
- **Libraries**:  
  - `Wire.h`: Arduino’s built-in I2C library.  
  - `Adafruit_BMP280.h`: © Adafruit Industries (MIT License).  
  - `RTClib.h`: © Adafruit Industries (modified by Jean-Claude Wippler, MIT License).  
- **Hardware**: All components are standard and available from electronics suppliers (e.g., Adafruit, SparkFun).  

---

## 💡 Future Roadmap for v1.2  
- **TFT Display Integration**: Use Serial1 to render data on a 2.4"/3.2" TFT (e.g., ST7735) with custom GUI.  
- **Cloud Connectivity**: Add MQTT/HTTP support to send data to platforms like Adafruit IO or Home Assistant.  
- **OTA Updates**: Enable over-the-air firmware updates (requires ESP32/ESP8266 variant).  
- **Battery Monitoring**: Add a voltage divider to track RTC backup battery levels.  
- **Multi-Node Network**: Extend I2C or add NRF24L01+ for wireless communication with slave nodes.  

---  

This README is designed to welcome new contributors and users, clearly outlining the project’s purpose, features, and next steps. It balances technical depth with readability, making it the perfect first impression for your GitHub repository! 🚀
