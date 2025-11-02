# 🌦 WeatherStation 2.0

## 1️⃣ Motivation / Background
**WeatherStation 2.0** is the successor of my first homemade weather station.  
The original version aimed for wireless sensor communication but wasn’t stable enough. 😅  
It began as a **school project**, but this version is designed as a **modular, scalable, and reliable system** with a more professional architecture.

🎯 **Goal:**  
Build a multi-node weather monitoring network with  
- optional cloud integration,  
- local display (TFT/LCD/7-segment),  
- and modular sensor interfaces —  
to observe environmental data in different rooms or locations. 🏠

---

## 2️⃣ Project Overview
- **Core Idea:**  
  A modular indoor weather station built around the **ESP32**, measuring **temperature, humidity, and light** in real time.  
- **Display System:**  
  A color **TFT GUI** as main interface, with **LCD (16x2)** or **7-segment** fallback for minimal configurations.  
- **System Topology:**  
  ![Fritzing Overview](docs/images/fritzing_overview.jpg)  
  *(Fritzing layout showing node structure and sensor wiring)*

---

## 3️⃣ Display & GUI Overview
| Display Type | Library / Driver | Highlights | Notes |
|--------------|-----------------|-------------|--------|
| **TFT 2.4" / 3.2"** | `MCUFRIEND_kbv` | Full-color GUI with real-time updates (<1s), customizable layout & theme | Main interface, supports dual-node mode |
| **LCD 16x2 (I2C)** | `LiquidCrystal_I2C` | Simple text-based readout (Temp/Humidity) | Low-power fallback |
| **7-segment (TM1637)** | `TM1637Display` | Compact numeric output for key metrics | Ultra-low-power, minimal design |

**Example Layout (TFT):**


┌───────────────────────────────┐
│ WeatherStation │
├───────────────────────────────┤
│ Temp: 22.5°C │
├───────────────────────────────┤
│ Humidity: 55% │
├───────────────────────────────┤
│ Light: 800 lux │
├───────────────────────────────┤
│ Updated: 12:34 │
└───────────────────────────────┘


---

## 4️⃣ Hardware Overview
| Component | Model / Type | Function | Notes |
|-----------|---------------|-----------|--------|
| **Main Controller** | ESP32 DevKit v1 / ESP32-WROOM-32 | Core processor, Wi-Fi & Bluetooth | Handles sensors, display & wireless communication |
| **Temp/Humidity Sensor** | DHT22 / BME280 | Measures temperature (±0.5°C) & humidity (±2%) | Plug-and-play modular |
| **Light Sensor** | BH1750 | Ambient light intensity measurement | Adjustable update rate |
| **Wireless Module** | NRF24L01+ | Stable communication between remote nodes (~100 m) | Default wireless backbone |
| **Backup Link** | 433 MHz Transceiver | Long-range fallback (~1 km) | Optional |
| **Displays** | TFT / LCD / 7-seg | Shows real-time environment data | GUI section for layout |
| **Input** | 10kΩ Potentiometer | Contrast control or menu navigation | Multi-display compatible |
| **Optional Sensors** | PMS5003, Camera Module | Air quality & time-lapse imaging | Expandable modular support |

📷 *Example schematic / assembly:*  
![Assembly Example](docs/images/step_by_step.jpg)  
*(Documented build process with Fritzing diagrams)*

---

## 5️⃣ Firmware / Code Structure
📁 **Firmware:** [`/firmware/main/WeatherStation.ino`](firmware/main/WeatherStation.ino)  
🧩 **Config files:** JSON/YAML located in `/firmware/config/`

- Modular codebase (sensors, display, wireless, config separated)  
- Designed for **multi-node scalability**  
- Plug-in style: add new sensors without touching the main loop  

---

## 6️⃣ Measurements & Calculations
- 📘 Reference: [`docs/WeatherStation_Test.pdf`](docs/WeatherStation_Test.pdf)  
- All measurement principles and formulas are documented.  
- Sample output and test results in `/examples/`.

---

## 7️⃣ Networking & Collaboration
- **Wireless Protocols:** NRF24L01 (default) / 433 MHz (fallback)  
- **Max Nodes:** Tested up to 15+ nodes in single network  
- **Contributing:** Open issues and PRs are welcome 🤝  
- **Troubleshooting:** see `/docs/troubleshooting.md`

---

## 8️⃣ License
MIT License – see [LICENSE](LICENSE) 📜

---

## 💡 Highlights / Key Improvements
- Clean modular structure (`firmware/`, `hardware/`, `docs/`, `examples/`)  
- Reworked GUI mockup for clarity and consistency  
- Visual documentation with Fritzing and photo references  
- Emoji-based sectioning for readability 🌦🛠️📊  
- Git-ready folder hierarchy for multi-node development  
- Scalable firmware architecture for IoT expansion 🚀  

---

### 🧰 Future Plans
- MQTT / Web dashboard integration 🌐  
- SD-card data logging  
- Local time-sync (NTP)  
- OTA updates for remote nodes  
