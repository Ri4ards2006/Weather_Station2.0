🌦 WeatherStation 2.0
1️⃣ Motivation / Lore

WeatherStation 2.0 is the next iteration of my first version.
The original project attempted wireless sensor communication, but it struggled with stability and modularity. 😅
It was initially made for a school project, but now I aim for a professional, scalable, and reliable system.
The new goal: multiple wireless nodes, optional cloud integration, local TFT/LCD/7-segment display, and modular sensor support — so I can monitor environmental conditions in different rooms at home. 🏠

2️⃣ Project Overview

What is it?
A modular indoor weather station built around the ESP32, measuring temperature, humidity, and light in real time.

Display & GUI: TFT color GUI as main interface, LCD 16x2 or 7-segment as fallback for minimal setups.

Reference diagram:


(Example wiring and node layout using Fritzing)

3️⃣ GUI / Display Overview
Display Type	Library / Driver	Layout Highlights	Notes
TFT 2.4"/3.2"	MCUFRIEND_kbv	Color metrics: Temp, Humidity, Light; sub-second updates; customizable layout & colors	Main GUI; supports dual-node updates
LCD 16x2 (I2C)	LiquidCrystal_I2C	Simple text: Temp/Humidity	Fallback for low-power setups
7-segment 4-digit (SPI)	TM1637	Minimal display for key values (e.g., temperature alert)	Ultra-low-power, compact

Example TFT Layout:

┌───────────────────────────────┐
│       WeatherStation          │
├───────────────────────────────┤
│ Temp:    22.5°C               │
├───────────────────────────────┤
│ Humidity: 55%                 │
├───────────────────────────────┤
│ Light:   800 lux              │
├───────────────────────────────┤
│ Updated: 12:34                │
└───────────────────────────────┘


The layout is modular, so remote node readings can be added dynamically.

GUI supports custom colors, font sizes, and menu navigation via potentiometer or touch (optional).

4️⃣ Hardware Overview
Component	Model / Type	Purpose	Notes
Main Controller	ESP32 DevKit v1 / ESP32-WROOM-32	Core processing, Wi-Fi, Bluetooth	Handles all sensor nodes & display
Temp/Humidity Sensor	DHT22 / BME280	Accurate temperature (±0.5°C) and humidity (±2%)	Modular connection
Light Sensor	BH1750	Measures ambient light intensity	Optional, configurable update rate
Wireless Module	NRF24L01+	Stable communication with remote nodes (~100 m)	Default wireless node protocol
Backup Wireless	433 MHz Transceiver	Long-range, low-power (~1 km)	Optional fallback
Displays	TFT 2.4"/3.2", LCD 16x2, 7-segment	Displays environmental metrics	See GUI section for layout
Input	Potentiometer 10kΩ	Adjust display contrast or navigate menus	Works with TFT and LCD
Optional	Raspberry Pi Camera, PMS5003	Time-lapse, particulate matter measurement	Expandable sensor modules

Schematics / Assembly Step-by-Step:


(Step-by-step assembly documented with images and Fritzing diagrams)

5️⃣ Firmware / Code

Quick link to code: WeatherStation Firmware
 💻

Modular design: sensors, display, wireless communication abstracted.

Supports multiple nodes, easy integration of new sensors without rewriting core logic.

Configuration via JSON/YAML in /firmware/config/.

6️⃣ Measurements / Calculations

PDF Reference: Test & Calculation
 📊

All environmental readings and formulas documented.

Example outputs in /examples/ folder.

7️⃣ Network & Community

Wireless Nodes: NRF24L01 / 433 MHz, scalable up to 15+ nodes.

Contributing: Open Issues & Pull Requests welcome.

Tips / Troubleshooting: /docs/troubleshooting.md.

8️⃣ License

MIT License – see LICENSE
 📜

💡 Highlights / Improvements:

Clean modular folder structure (firmware/, hardware/, docs/, images/, examples/)

Improved GUI table & ASCII mockup for better readability

Emojis to make sections immediately scannable 🌦📌🚀🛠️🎉🤝

Step-by-step hardware visuals + Fritzing diagrams for clarity

Modular approach for firmware & sensor expansion
