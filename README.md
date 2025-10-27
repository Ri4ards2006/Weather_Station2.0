🌦 WeatherStation 2.0
A modular, ESP32-based indoor weather station with enhanced scalability and wireless integration

WeatherStation 2.0 is the next-generation indoor environmental monitoring system. Built around the ESP32 microcontroller, it measures and displays real-time temperature, humidity, and light data while supporting modular sensor expansion, wireless communication, and optional cloud integration. This version addresses key limitations of WeatherStation 1.0 (e.g., unstable wireless module compatibility and poor scalability) with a redesigned architecture focused on reliability and future-proofing.

📌 Problem & Goal
Problem: The first WeatherStation version struggled with consistent wireless sensor communication (e.g., NRF24L01/433 MHz modules) and lacked flexibility to add new sensors without extensive code changes.
Goal: Develop a scalable, modular system that:

Reliably supports local and remote (wireless) sensors.
Easily integrates new environmental metrics (e.g., light, CO₂, particulate matter).
Offers clear, user-friendly local displays (TFT/LCD/7-segment) and optional remote dashboards.
🚀 Key Features
Real-Time Environmental Monitoring: Measures temperature (°C), humidity (%), and light intensity (lux) with sub-second update rates (configurable).
Local Display Flexibility:
TFT Color Display: Uses MCUFRIEND_kbv (SPI) for vibrant, GUI-based metrics (customizable layout, colors).
Fallback Displays: Supports legacy LCD 16x2 or 4-digit 7-segment displays for low-power setups.
ESP32 Central Controller: Leverages ESP32’s dual-core CPU, Wi-Fi, and Bluetooth for:
Wireless sensor coordination (via NRF24L01/433 MHz).
Cloud connectivity (MQTT, HTTP) for remote data storage/visualization.
Modular Sensor Design:
Sensors are decoupled via an abstraction layer (easy to add/remove without rewriting core code).
Example sensors: DHT22, BME280 (temp/humidity), BH1750 (light), HC-SR04 (optional occupancy detection).
Phase-Based Development:
Phase 1 (Core): Local data measurement and display (TFT/LCD/7-segment).
Phase 2 (Wireless): Add NRF24L01/433 MHz modules for remote sensor communication (e.g., outdoor temp/humidity).
Phase 3 (Cloud): Optional web dashboard (via Node-RED, Blynk) or cloud integration (AWS IoT, Adafruit IO).
🛠 Hardware Components
Component
Model/Type
Purpose
Main Controller
ESP32 DevKit v1 / ESP32-WROOM-32
Central processing, Wi-Fi, Bluetooth, and sensor management.
Environmental Sensors
DHT22 / BME280
High-accuracy temperature (±0.5°C) and humidity (±2%) measurements.
BH1750
Light intensity sensing (0-65535 lux) for brightness tracking.
Wireless Modules
NRF24L01+ 2.4 GHz
Stable communication with remote sensors (range: ~100m).
433 MHz Transceiver
Low-power fallback for long-range (up to 1km) sensor communication.
Local Displays
MCUFRIEND_kbv TFT (2.4"/3.2")
Primary display with color GUI (customizable layout, anti-aliasing).
LCD 16x2 (I2C)
Low-cost fallback display for basic metric visualization.
4-digit 7-segment (SPI)
Ultra-low-power display for critical values (e.g., temp alert).
User Input
Potentiometer (10kΩ)
Adjust display contrast (TFT/LCD) or navigate menu options (TFT UI).
Optional Add-Ons
Raspberry Pi Camera Module
Capture time-lapse images or visual logs (e.g., window condensation).
PMS5003 Particulate Sensor
Extend to air quality monitoring (PM2.5/PM10).

📂 Repository Structure

Collapse
Copy
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
WeatherStation2.0/  
├── firmware/                        # ESP32/Arduino codebase  
│   ├── main/                        # Core application logic  
│   │   ├── WeatherStation.ino       # Main sketch (TFT UI + sensor handling)  
│   │   ├── include/                 # Custom header files (sensor abstractions)  
│   │   └── libraries/               # Third-party libraries (Adafruit_GFX, DHT, etc.)  
│   ├── config/                      # Sensor/display configuration files (JSON/YAML)  
│   └── docs/                        # Code documentation (Doxygen-style comments)  
├── hardware/                        # Circuit designs and BOM  
│   ├── schematics/                  # KiCad/Eagle diagrams for main board and add-ons  
│   ├── BOM.csv                      # Bill of Materials (components, suppliers, costs)  
│   └── 3D_case/                     # Enclosure design (Fusion 360/STL files)  
├── scripts/                         # Helper scripts (data processing, dashboard)  
│   ├── data_logger.py               # Python script to log sensor data to CSV  
│   └── mqtt_publisher.py             # MQTT client for cloud integration (Phase 3)  
├── examples/                        # Sample setups and outputs  
│   ├── tft_ui_screenshot.jpg        # Example GUI on TFT display  
│   └── lcd_output_sample.txt         # Sample LCD text output  
└── docs/                            # Project documentation  
    ├── README.md                     # This file  
    ├── design_notes.md               # Architecture decisions (modularity, wireless)  
    └── troubleshooting.md           # Common issues and fixes (e.g., sensor calibration)  
🛠️ Setup & Installation
Prerequisites
Arduino IDE (with ESP32 board support installed).
Required libraries:
Adafruit GFX Library (GitHub )
MCUFRIEND_kbv Library (GitHub )
DHT Sensor Library (GitHub )
Adafruit BME280 Library (GitHub )
BH1750 Light Sensor Library (custom or third-party).
Steps
Hardware Assembly:
Connect ESP32 to sensors (see hardware/schematics).
Attach display (TFT/LCD/7-segment) using SPI/I2C pins.
Power the system via 5V/3.3V (adjust potentiometer for contrast).
Firmware Upload:
Open firmware/main/WeatherStation.ino in Arduino IDE.
Select the correct ESP32 board (e.g., "ESP32 Arduino").
Upload to the device.
Configuration:
Adjust simulateData flag in code to switch between test mode and real sensors.
Modify displayHeight, displayLeft, etc., in firmware/main/constants.h (if TFT model differs).
🎉 Usage Examples
Local TFT Display:
Run the firmware with simulateData = false. The TFT shows:

Collapse
Copy
1
2
3
4
5
6
7
8
9
10
11
┌───────────────────────────────┐  
│         Wetterstation         │  
├───────────────────────────────┤  
│ Temp:    22.5°C               │  
├───────────────────────────────┤  
│ Humidity:  55%                │  
├───────────────────────────────┤  
│ Light:   800 lux              │  
├───────────────────────────────┤  
│ Aktualisiert: 12:34            │  
└───────────────────────────────┘  
Wireless Remote Sensor (Phase 2):
Pair an ESP32 node (remote) with the main controller using NRF24L01 modules. The main display will show:

Collapse
Copy
1
Remote Temp (Living Room): 24.1°C  
🤝 Contributing
We welcome contributions! To get started:

Report Issues: Use the GitHub Issues tab for bugs or feature requests.
Submit Pull Requests:
Fork the repository.
Create a branch for your feature/fix.
Ensure code follows the existing style (comments, indentation).
Add tests if applicable (e.g., sensor validation scripts).
Documentation: Update docs/design_notes.md or docs/troubleshooting.md for architecture changes or new issues.
📜 License
WeatherStation 2.0 is open-source under the MIT License. See LICENSE for details.

🙏 Acknowledgments
Adafruit Libraries: Critical for TFT display rendering and sensor support.
Bodmer/MCUFRIEND_kbv: Thank you for the SPI display driver library.
Community: Inspired by open-source projects like ESP32 Weather Station and Modular Sensor Frameworks .
Let me know if you need further tweaks (e.g., adding specific sensor specs, links to wiring guides, or emphasizing certain features)! 😊
