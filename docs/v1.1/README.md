# 🌦 WeatherStation 2.0 - Master (v1.1)

## Overview
This Arduino project implements a **master node** that reads data from a BMP280 sensor, an ultrasonic distance sensor, and a DS3231 RTC. The collected data is displayed on the serial monitor and optionally sent via `Serial1` to other devices.

---

## Features
- **BMP280**: Measures temperature and atmospheric pressure
- **RTC DS3231**: Real-time clock, checks for power loss, can be set via serial command (`SET:YYYY-MM-DD HH:MM:SS`)
- **Ultrasonic sensor**: Measures distance in centimeters
- **Serial debug output**: Displays sensor readings clearly in the console
- **Optional Serial1 output**: For radio modules or other external devices

---

## Hardware
- **BMP280**: I2C, address 0x76
- **RTC DS3231**: I2C, SDA/SCL pins
- **Ultrasonic sensor**: TRIG=52, ECHO=53
- Recommended: Arduino Mega or Uno

---

## Libraries
- **Wire.h** – I2C communication
- **Adafruit_BMP280.h** – BMP280 sensor library
- **RTClib.h** – RTC DS3231 library

---

## Pinout
| Component       | Arduino Pin |
|-----------------|------------|
| BMP280 SDA      | SDA        |
| BMP280 SCL      | SCL        |
| RTC SDA         | SDA        |
| RTC SCL         | SCL        |
| Ultrasonic TRIG | 52         |
| Ultrasonic ECHO | 53         |

---

## Usage
1. Connect the Arduino and upload the sketch
2. Open the Serial Monitor at 115200 baud
3. Data will be displayed every 2 seconds
4. Optionally, set the RTC time via Serial:

Example: SET:2025-11-03 12:34:56


---

## How it works
- The `loop()` checks for RTC commands via Serial input
- Every 2 seconds, the BMP280 and ultrasonic readings are collected and printed
- Example output:

🌡 Temperature: 23.5 °C
🌬 Pressure: 1013.2 hPa
📏 Distance: 55.2 cm
⏰ Time: 12:34:56
Unix Time: 1730541296

- Ultrasonic uses `pulseIn` with a 25ms timeout; values <0 indicate no reading
- BMP280 initialization is retried if it was not detected initially

---

## Notes
- BMP280 and RTC share the I2C bus
- RTC power loss triggers a warning
- Serial1 output can forward sensor data to additional modules

