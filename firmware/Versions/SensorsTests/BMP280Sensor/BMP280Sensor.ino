#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // I2C Sensorobjekt

void setup() {
  Serial.begin(9600);
  Serial.println(F("Starte BMP280 Test (Enhanced Safe Mode)"));

  // Mega I2C Pins automatisch: SDA=20, SCL=21
  Wire.begin(); 

  // Sensor initialisieren (Adresse 0x76 oder 0x77)
  bool sensorFound = bmp.begin(0x76); 
  if (!sensorFound) {
    Serial.println(F("❌ BMP280 nicht gefunden!"));
    Serial.println(F("  Prüfe: SDA=20, SCL=21, CSB=5V, SDO=GND, VCC/GND"));
  } else {
    Serial.println(F("✅ BMP280 erfolgreich verbunden."));
  }
}

void loop() {
  // Prüfen, ob Sensor gefunden wurde
  if (bmp.sensorID() == 0) {
    Serial.println(F("Sensor nicht aktiv, Werte werden nicht gelesen."));
  } else {
    float tempC = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F;
    float altitude = bmp.readAltitude(1013.25);

    Serial.print(F("Temperatur: "));
    Serial.print(tempC);
    Serial.println(" °C");

    Serial.print(F("Luftdruck: "));
    Serial.print(pressure);
    Serial.println(" hPa");

    Serial.print(F("Höhe: "));
    Serial.print(altitude);
    Serial.println(" m");
  }

  Serial.println("-------------------------");
  delay(2000); // 2 Sekunden warten
}
