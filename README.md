# 🌦 WeatherStation 2.0

A modular ESP32-based indoor weather station that measures and displays environmental data.  
This is the second version — the first one worked but ran into issues with wireless modules and scalability.

---

## 🚀 Features
- Real-time indoor temperature, humidity, and light measurements
- Local display support: LCD / 7-segment display
- ESP32 as the central controller (Wi-Fi + optional RF modules)
- Modular design for easy sensor expansion
- Designed in phases:
  1. Phase 1: Display sensor data locally
  2. Phase 2: Add wireless modules for remote sensors
  3. Phase 3: Optional web dashboard or cloud integration

---

## 🧰 Hardware Components
| Component                         | Description                        |
|----------------------------------|------------------------------------|
| ESP32 DevKit v1                   | Main microcontroller               |
| DHT22 / BME280                    | Temperature & humidity sensors     |
| HC-SR04                            | Ultrasonic distance sensor         |
| LCD 16x2 / 4-digit 7-segment      | Local display output               |
| RF Module (NRF24L01 / 433 MHz)    | Wireless sensor communication      |
| Potentiometers                     | Display contrast / menu control    |
| Optional Camera Module             | Time-lapse / image capture         |

---

## 🧩 Repository Structure
 HEAD


 20da323 (Add project folder structure for Weatherstation2.0)
