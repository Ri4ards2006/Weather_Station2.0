#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <RTClib.h>

// --- Sensoren definieren ---
Adafruit_BMP280 bmp;       
RTC_DS3231 rtc;            

// --- Ultraschall ---
#define TRIG_PIN 52
#define ECHO_PIN 53

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 2000;  // ms

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  Wire.begin();

  Serial.println(F("🌦 Wetterstation startet..."));

  // BMP
  if (!bmp.begin(0x76)) {
    Serial.println(F("❌ BMP280 nicht gefunden!"));
    // kein infinite loop hier, wir wollen trotzdem RTC debuggen
  } else {
    Serial.println(F("✅ BMP280 verbunden"));
  }

  // RTC
  if (!rtc.begin()) {
    Serial.println(F("❌ RTC-Modul nicht gefunden! Prüfe SDA/SCL und VCC/GND"));
  } else {
    Serial.println(F("✅ RTC verbunden"));
  }

  // Prüfen, ob RTC Power verloren hat (Batterie leer/fehlt)
  if (rtc.lostPower()) {
    Serial.println(F("⚠️ RTC meldet Stromverlust (lostPower()). Zeit ggf. nicht gesetzt."));
    Serial.println(F("   -> Entweder Batterie installieren/prüfen oder Zeit per 'SET:' setzen."));
    // Optional: Einmalig setzen auf Kompilierzeit (nur aktivieren, wenn du willst):
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // Serial.println(F("RTC auf Kompilierzeit gesetzt. Kommentiere rtc.adjust() danach aus."));
  }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println(F("Setup abgeschlossen."));
  Serial.println(F("Befehle: Sende 'SET:YYYY-MM-DD HH:MM:SS' um die Uhr zu setzen."));
  Serial.println();
}

// Hilfs: parse "YYYY-MM-DD HH:MM:SS"
bool parseAndSetRTC(const String &s) {
  // Erwartetes Format: "SET:2025-11-03 12:34:56"
  if (!s.startsWith("SET:")) return false;
  String body = s.substring(4);
  // sicherstellen, Länge passt
  if (body.length() < 19) return false;

  int Y = body.substring(0,4).toInt();
  int m = body.substring(5,7).toInt();
  int d = body.substring(8,10).toInt();
  int H = body.substring(11,13).toInt();
  int M = body.substring(14,16).toInt();
  int S = body.substring(17,19).toInt();

  if (Y < 2000 || m<1 || m>12 || d<1 || d>31 || H<0 || H>23 || M<0 || M>59 || S<0 || S>59) {
    return false;
  }

  DateTime dt(Y, m, d, H, M, S);
  rtc.adjust(dt);
  return true;
}

void loop() {
  // Serielle Eingaben verarbeiten (für SET:)
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      if (parseAndSetRTC(line)) {
        Serial.println(F("✅ RTC gesetzt via SET-Befehl."));
      } else {
        Serial.println(F("❌ Ungültiges SET-Format oder Befehl. Verwende: SET:YYYY-MM-DD HH:MM:SS"));
      }
    }
  }

  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();
    printSensorData();
  }
}

void printSensorData() {
  // BMP
  float temperature = NAN;
  float pressure = NAN;
  if (bmp.begin(0x76)) { // falls bmp.begin() vorher fehlgeschlagen ist, versuchen wir nicht nochmal endlos
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure() / 100.0F;
  } else if (bmp.readTemperature() != bmp.readTemperature()) {
    // noop - bmp nicht vorhanden
  }

  // Ultraschall
  float distance = measureDistanceCM();

  // RTC Zeit
  DateTime now = rtc.now();
  char timeString[20];
  snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  // Unix time (zum Debuggen, wie Sekunden laufen)
  unsigned long unix = now.unixtime();

  Serial.println(F("----------- Messdaten -----------"));
  if (!isnan(temperature)) {
    Serial.print(F("🌡 Temperatur: ")); Serial.print(temperature,1); Serial.println(F(" °C"));
    Serial.print(F("🌬 Luftdruck:  ")); Serial.print(pressure,1); Serial.println(F(" hPa"));
  } else {
    Serial.println(F("🌡 BMP280: nicht verfügbar"));
  }

  if (distance >= 0) {
    Serial.print(F("📏 Entfernung: ")); Serial.print(distance,1); Serial.println(F(" cm"));
  } else {
    Serial.println(F("📏 Entfernung: keine Messung (Timeout)"));
  }

  Serial.print(F("⏰ Zeit:        ")); Serial.println(timeString);
  Serial.print(F("Unix Time:     ")); Serial.println(unix); // sehr hilfreich zum prüfen ob Sekunden korrekt laufen
  Serial.println(F("--------------------------------\n"));

  // optional an Funk
  Serial1.print(F("TEMP:")); Serial1.println(temperature);
  Serial1.print(F("PRESS:")); Serial1.println(pressure);
  Serial1.print(F("DIST:")); Serial1.println(distance);
}

float measureDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  if (duration == 0) return -1;
  float distance = (duration * 0.0343) / 2.0;
  return distance;
} okokokokok  cool und jz unabhöngig von der lettztenn kommentiere mir bitte auch so krass wie vorher die datei mit klare eklärung und guidance pls 
