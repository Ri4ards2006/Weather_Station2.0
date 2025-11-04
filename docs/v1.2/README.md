
# 🌦 WeatherStation 2.0 - Master (v1.2)  
**Real-Time Environmental Monitoring with Automated Alarm System**  

---

## Overview  
The WeatherStation 2.0 Master (v1.2) is an Arduino-based project that integrates environmental sensing, real-time clock (RTC) functionality, and an automated alarm system. It reads data from a BMP280 sensor (temperature/pressure), an HC-SR04 ultrasonic distance sensor, and a DS3231 RTC module, then displays this data via the serial monitor. Additionally, v1.2 introduces **visual (traffic light)** and **auditory (buzzer)** alerts for critical conditions, plus a test button to manually verify alarm functionality.  

---

## Key Features (v1.2 Enhancements)  
- **Environmental Sensing**:  
  - BMP280: Measures temperature (°C) and atmospheric pressure (hPa).  
  - Ultrasonic Sensor: Detects distance to objects (cm) with timeout handling.  
  - RTC DS3231: Tracks accurate time/date, detects power loss, and supports manual time setting via serial commands.  
- **Alarm System**:  
  - **Traffic Lights**: Visualizes conditions with:  
    - ❌ Red (ID 46): Critical alarm (distance 40°C).  
    - ⚠️ Yellow (ID 48): Warning (distance 40°C.  
- The buzzer sounds a 1000Hz tone for 500ms during critical alarms.  

---

## Example Output (Serial Monitor)  
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

// When distance <10cm (alarm triggered):
🔔 Alarm aktiv: Entfernung zu gering!
----------- Messdaten -----------
🌡 Temperatur: 25.3 °C
🌬 Luftdruck:  1012.8 hPa
📏 Entfernung: 8.9 cm
⏰ Zeit:       09:30:07
--------------------------------

// When Test Button is pressed:
🔔 Button gedrückt: Test-Alarm!
✅ RTC gesetzt via SET-Befehl. (if no SET command, just alarm messages)
```

---

## Notes & Troubleshooting  
- **I2C Conflicts**: BMP280 and RTC share the I2C bus. Use an I2C scanner sketch (e.g., `SimpleI2CScanner`) to confirm addresses if sensors aren’t detected.  
- **RTC Accuracy**: Without a backup battery (CR2032), the RTC may lose time on power loss (`rtc.lostPower()` → log warning). Install a battery for reliable timekeeping.  
- **LED Behavior**: LEDs are active-high (HIGH = on). Green LED starts on by default; Red/Yellow override based on conditions.  
- **Buzzer Adjustment**: Modify `tone()` parameters (frequency/duration) in `triggerAlarm()` if a different alert sound is needed.  
- **Ultrasonic Limits**: Max detection range ~400cm (due to 25ms timeout). Extend the timeout in `measureDistanceCM()` for longer ranges.  
- **Button Wiring**: Ensure the Test Button connects pin 26 to GND. No external resistors needed (Arduino uses `INPUT_PULLUP`).  

---

## Version History  
- **v1.1**: Basic sensor data collection with serial output.  
- **v1.2**: Added alarm system (traffic lights + buzzer), test button, and automated condition checks.  

---

## License & Credits  
- **Code**: MIT License (free to modify, distribute, and adapt).  
- **Libraries**:  
  - Adafruit BMP280 Library: © Adafruit Industries (MIT License).  
  - RTClib: © Adafruit (modified by Jean-Claude Wippler, MIT License).  
- **Hardware**: All components are standard and widely available (e.g., Amazon, eBay, local electronics stores).  

--- 

This project combines precision sensing with user-friendly alerts, making it ideal for weather stations, indoor environmental monitoring, or safety systems requiring real-time condition tracking.
