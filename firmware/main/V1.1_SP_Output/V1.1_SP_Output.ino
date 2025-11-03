#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <RTClib.h>

// --- Sensoren definieren ---
Adafruit_BMP280 bmp;       // BMP280: Temperatur + Druck
RTC_DS3231 rtc;            // Echtzeituhr-Modul (z. B. DS3231)

// --- Ultraschallsensor-Pins ---
#define TRIG_PIN 52
#define ECHO_PIN 53

// --- Zeitvariablen ---
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 2000;  // alle 2 Sekunden neue Messung

void setup() {
  Serial.begin(115200);     // Monitor
  Serial1.begin(9600);      // Funkmodul (TX1=18, RX1=19)
  Wire.begin();             // I2C starten (SDA=20, SCL=21)

  Serial.println(F("🌦 Wetterstation startet..."));

  // --- BMP280 initialisieren ---
  if (!bmp.begin(0x76)) {   // Adresse kann 0x76 oder 0x77 sein
    Serial.println(F("❌ BMP280 nicht gefunden!"));
    while (1);
  } else {
    Serial.println(F("✅ BMP280 verbunden"));
  }

  // --- RTC initialisieren ---
  if (!rtc.begin()) {
    Serial.println(F("❌ RTC-Modul nicht gefunden!"));
  } else {
    Serial.println(F("✅ RTC verbunden"));
  }

  // --- Ultraschallsensor vorbereiten ---
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println(F("Setup abgeschlossen.\n"));
}

void loop() {
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();
    printSensorData();
  }
}

// ---------- Messfunktionen ----------

void printSensorData() {
  // --- BMP280 auslesen ---
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;  // in hPa

  // --- Ultraschall messen ---
  float distance = measureDistanceCM();

  // --- Zeit abrufen ---
  DateTime now = rtc.now();
  char timeString[20];
  sprintf(timeString, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  // --- Ausgabe auf Serial-Monitor ---
  Serial.println(F("----------- Messdaten -----------"));
  Serial.print(F("🌡 Temperatur: ")); Serial.print(temperature, 1); Serial.println(F(" °C"));
  Serial.print(F("🌬 Luftdruck:  ")); Serial.print(pressure, 1); Serial.println(F(" hPa"));
  Serial.print(F("📏 Entfernung: ")); Serial.print(distance, 1); Serial.println(F(" cm"));
  Serial.print(F("⏰ Zeit:        ")); Serial.println(timeString);
  Serial.println(F("--------------------------------\n"));

  // --- Optional: Werte ans Funkmodul senden ---
  Serial1.print("TEMP:"); Serial1.println(temperature);
  Serial1.print("PRESS:"); Serial1.println(pressure);
  Serial1.print("DIST:"); Serial1.println(distance);
}

// ---------- Hilfsfunktion: HC-SR04 ----------
float measureDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // Timeout nach 25ms (~4m)
  float distance = duration * 0.0343 / 2;         // cm
  return distance;
}
