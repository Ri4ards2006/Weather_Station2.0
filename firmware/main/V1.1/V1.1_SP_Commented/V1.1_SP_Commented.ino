// ==============================================
// WeatherStation 2.0 - Master Sensor Data Logger (With BMP280, RTC, and Ultrasonic)
// ==============================================
// Zweck: Dieses Sketch liest Daten von I2C-Sensoren (BMP280 für Temperatur/Luftdruck), einem RTC-Modul (DS3231 für Zeit), 
//        und einem Ultraschall-Sensor (Entfernung) ein. Er behandelt serielle Befehle (z. B. Uhrzeit setzen) und gibt 
//        alle Messwerte alle 2 Sekunden über den seriellen Monitor aus.
//        Ideal für Wetterstationen oder Umweltmonitoring-Systeme, die zeitbehaftete Sensordaten benötigen.
// Hardware-Anforderungen:
//   - Arduino mit I2C (Wire), Serial, und GPIO-Pins (hier: TRIG=52, ECHO=53)
//   - BMP280-Sensor (I2C-Adresse 0x76 standardmäßig)
//   - DS3231 RTC-Modul (I2C-Adresse 0x68 standardmäßig)
//   - Ultraschall-Sensor (TRIG/ECHO Pins)
//   - 3.3V oder 5V Stromversorgung (abhängig von Komponenten)
// ==============================================

// ---------------------------
// 1. Bibliotheken einbinden
// ---------------------------
// Wire.h: Benötigt für I2C-Kommunikation (Master-Rolle) mit BMP280 und RTC-Modul.
#include <Wire.h>        

// Adafruit_BMP280.h: Spezielle Bibliothek für den BMP280-Sensor (Temperatur/Luftdruck). 
//                    Implementiert I2C-Funktionsaufrufe und Datenkonvertierung.
#include <Adafruit_BMP280.h> 

// RTClib.h: Bibliothek für den DS3231 Real-Time-Clock (RTC). Managiert Zeitstempel und Kalibrationsfunktionen.
#include <RTClib.h>     

// ==============================================

// ---------------------------
// 2. Sensor- und Pin-Definitionen
// ---------------------------
// BMP280-Sensor-Objekt (I2C). Dieses Objekt verwaltet alle Kommunikation mit dem Druck-/Temperatursensor.
Adafruit_BMP280 bmp;       

// RTC-DS3231-Objekt (I2C). Dieses Objekt verwaltet die Uhrzeit und Kalibrationsfunktionen des Real-Time-Clocks.
RTC_DS3231 rtc;            

// Ultraschall-Sensor-Pins: Auslöser (TRIG) und Echo (ECHO)-Pin definieren.
// TRIG-Pin: Sendet ein Trigger-Signal, um Entfernungsmessung zu starten (Output).
// ECHO-Pin: Empfängt das Echo-Signal vom Objekt (Input).
#define TRIG_PIN 52        
#define ECHO_PIN 53        

// Zeitmanagement-Variablen:
// 'lastUpdate': Zeitstempel (ms seit Start) der letzten Vollständigen Sensormessung. 
//               Verfolgt, wann der nächste Messzyklus fällig ist.
unsigned long lastUpdate = 0;  

// 'updateInterval': Intervall (ms) zwischen Messzyklen. Hier 2000ms = 2 Sekunden. 
//                   Anpassen, falls Sensoren langsamer reagieren oder häufiger gemessen werden sollen.
const unsigned long updateInterval = 2000;  
// ==============================================

// ---------------------------
// 3. Setup-Funktion (Einmal bei Start)
// ---------------------------
// Initialisiert serielle Kommunikation, Sensoren, und hardware-abhängige Einstellungen.
void setup() {
  // Serieller Monitor initialisieren (Standarduart 115200 Baud). Für Debugging und User-Feedback.
  Serial.begin(115200);  
  // Serial1 initialisieren (z. B. für externe Anzeigen oder Geräte). 9600 Baud (standardmäßig für manche Modulen).
  Serial1.begin(9600);   
  
  // I2C-Master initialisieren (Wire-Library). Dieser Arduino fungiert als I2C-Master (steuert Sklaven).
  Wire.begin();         
  
  // Startbildschirm-Nachrichten ausgeben (mit Farbcodes für Lesbarkeit)
  Serial.println(F("🌦 Wetterstation startet..."));  // F() reduziert Flash-Speicherverbrauch (Strings)
  
  // ---------------------------
  // BMP280-Sensor Initialisierung
  // ---------------------------
  // Prüft, ob der BMP280-Sensor über I2C verbunden ist. Standard-Adresse: 0x76.
  if (!bmp.begin(0x76)) {  
    Serial.println(F("❌ BMP280 nicht gefunden!"));  // Fehlermeldung, falls Sensor nicht antwortet
    // Hinweis: Ohne 'infinite loop' hier, damit RTC-Debugging weiterhin funktioniert, falls BMP fehlt.
  } else {  
    Serial.println(F("✅ BMP280 verbunden"));        // Bestätigung, falls Sensor verbunden
  }
  
  // ---------------------------
  // RTC-DS3231 Initialisierung
  // ---------------------------
  // Prüft, ob das RTC-Modul über I2C verbunden ist. Standard-Adresse: 0x68.
  if (!rtc.begin()) {  
    Serial.println(F("❌ RTC-Modul nicht gefunden! Prüfe SDA/SCL und VCC/GND")); // Fehlermeldung mit Tipps
    // Hinweis: Überprüfen Sie die I2C-Kabel (SDA/SCL), Stromversorgung (VCC/GND), oder Adressen (falls geändert).
  } else {  
    Serial.println(F("✅ RTC verbunden"));           // Bestätigung, falls RTC verbunden
  }
  
  // ---------------------------
  // RTC-Power-Check
  // ---------------------------
  // Prüft, ob das RTC seit dem letzten Start Strom verloren hat (z. B. leere Batterie).
  if (rtc.lostPower()) {  
    Serial.println(F("⚠️ RTC meldet Stromverlust (lostPower()). Zeit ggf. nicht gesetzt.")); 
    Serial.println(F("   -> Entweder Batterie installieren/prüfen oder Zeit per 'SET:' setzen.")); 
    // Option: RTC auf Kompilierzeit setzen (nur temporär, deaktivieren nach Einsatz)
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
    // Serial.println(F("RTC auf Kompilierzeit gesetzt. Kommentiere rtc.adjust() danach aus.")); 
    // Hinweis: Die Kompilierzeit (von Arduino IDE) wird hier verwendet. Nur für Testzwecke!
  }
  
  // ---------------------------
  // Ultraschall-Sensor Pins
  // ---------------------------
  // TRIG-Pin als Output (sendet Trigger-Signale)
  pinMode(TRIG_PIN, OUTPUT);  
  // ECHO-Pin als Input (empfängt Echo-Signale)
  pinMode(ECHO_PIN, INPUT);   
  
  // Setup-Fertig-Meldung
  Serial.println(F("Setup abgeschlossen."));  
  Serial.println(F("Befehle: Sende 'SET:YYYY-MM-DD HH:MM:SS' um die Uhr zu setzen.")); 
  Serial.println();  // Leerzeile für bessere Lesbarkeit
}
// ==============================================

// ---------------------------
// 4. parseAndSetRTC-Funktion
// ---------------------------
// Verarbeitet serielle Eingaben im Format "SET:YYYY-MM-DD HH:MM:SS" und setzt die RTC-Zeit.
// Argument: 's' → String mit dem SET-Befehl (z. B. "SET:2025-11-03 12:34:56")
// Rückgabe: 'true' → Befehl erfolgreich verarbeitet; 'false' → ungültiges Format oder Fehler.
bool parseAndSetRTC(const String &s) {
  // Schritt 1: Überprüfen, ob der String mit "SET:" startet (Befehl-Präfix)
  if (!s.startsWith("SET:")) return false;  // Kein SET-Befehl → Fehler
  
  // Schritt 2: Extrahiere den "Körper" des Strings (ohne "SET:")
  String body = s.substring(4);  // Ab Index 4 (nach "SET:") bis Ende
  
  // Schritt 3: Validieren der Länge. Erwartet: 19 Zeichen (YYYY-MM-DD HH:MM:SS)
  //           Beispiel: "2025-11-03 12:34:56" → 19 Zeichen
  if (body.length() < 19) return false;  // Zu kurz → ungültig
  
  // Schritt 4: Extrahieren und Konvertieren der einzelnen Komponenten (Jahr, Monat, Tag, etc.)
  // Nutzt String.substring() mit Start- und End-Index (beachtet, dass End-Index ausschließlich ist)
  int Y = body.substring(0,4).toInt();    // Jahr (0-3) → z. B. "2025" → 2025
  int m = body.substring(5,7).toInt();   // Monat (5-6) → z. B. "11" → November
  int d = body.substring(8,10).toInt();  // Tag (8-9) → z. B. "03" → 3. Tag
  int H = body.substring(11,13).toInt(); // Stunde (11-12) → z. B. "12" → 12:00
  int M = body.substring(14,16).toInt(); // Minute (14-15) → z. B. "34" → 12:34
  int S = body.substring(17,19).toInt(); // Sekunde (17-18) → z. B. "56" → 12:34:56
  
  // Schritt 5: Validieren der Werte (Geburtstagsproblem vermeiden!)
  // Jahr: Mindestens 2000 (aus Praxis, kann angepasst werden)
  // Monat: 1-12
  // Tag: 1-31 (Prüfung auf korrekte Monatstage fehlt, aber grundlegendes Validieren)
  // Stunde: 0-23
  // Minute/Sekunde: 0-59
  if (Y < 2000 || m < 1 || m > 12 || d < 1 || d > 31 || H < 0 || H > 23 || M < 0 || M > 59 || S < 0 || S > 59) {
    return false;  // Ungültige Werte → Fehler
  }
  
  // Schritt 6: Erstelle DateTime-Objekt mit den validierten Werten
  DateTime dt(Y, m, d, H, M, S);  
  // Schritt 7: Setze RTC-Zeit auf dt
  rtc.adjust(dt);               
  
  return true;  // Befehl erfolgreich
}
// ==============================================

// ---------------------------
// 5. Main Loop (Endlosschleife)
// ---------------------------
// Hauptschleife: Verarbeitet serielle Befehle, führt Messzyklen aus, und gibt Daten aus.
void loop() {
  // ---------------------------
  // Serielle Befehle verarbeiten (SET-Befehl)
  // ---------------------------
  // Prüfen, ob serielle Daten verfügbar sind (z. B. vom Benutzer via Monitor)
  if (Serial.available()) {  
    // Lies den String bis zum Zeilenumbruch ('\n') und entferne Leerzeichen am Anfang/Ende (trim())
    String line = Serial.readStringUntil('\n').trim();  
    // Prüfen, ob der String nicht leer ist (nach trim())
    if (line.length() > 0) {  
      // Versuche, den SET-Befehl zu parsen und RTC zu setzen
      if (parseAndSetRTC(line)) {  
        Serial.println(F("✅ RTC gesetzt via SET-Befehl."));  // Erfolgsmeldung
      } else {  
        // Fehlermeldung mit korrektem Format-Tipp
        Serial.println(F("❌ Ungültiges SET-Format oder Befehl. Verwende: SET:YYYY-MM-DD HH:MM:SS"));  
      }
    }
  }

  // ---------------------------
  // Messzyklus auslösen (alle 2 Sekunden)
  // ---------------------------
  // Prüfen, ob genug Zeit vergangen ist seit der letzten Messung (updateInterval)
  if (millis() - lastUpdate >= updateInterval) {  
    lastUpdate = millis();  // Aktualisiere lastUpdate auf aktuelle Zeit (ms)
    printSensorData();      // Lies Sensoren und gib Daten aus
  }
}
// ==============================================

// ---------------------------
// 6. printSensorData-Funktion
// ---------------------------
// Lies Daten von BMP280, Ultraschall-Sensor, und RTC, formatiere sie, und gib sie über Serial/Serial1 aus.
void printSensorData() {
  // Variablen für BMP280-Daten
  float temperature = NAN;  // Temperatur (°C). NAN = "Not a Number" (ungültig)
  float pressure = NAN;      // Luftdruck (hPa). NAN = ungültig
  
  // ---------------------------
  // BMP280-Daten lesen
  // ---------------------------
  // Prüfen, ob BMP280 verbunden ist (begin() vor Setup erfolgreich war)
  if (bmp.begin(0x76)) {  
    // Lies Temperatur (in °C). Die BMP280 liefert Float-Werte.
    temperature = bmp.readTemperature();  
    // Lies Druck (in Pa) und konvertiere zu hPa (hektopascal). 1 hPa = 100 Pa → /100.0F
    pressure = bmp.readPressure() / 100.0F;  
  } else {  
    // BMP280 verbunden? Falls nicht, prüfen wir, ob 'readTemperature()' fehlerhaft ist (NAN-Rückgabe)
    // Hinweis: Dies ist redundant, da 'begin()' bereits in setup() fehlgeschlagen war. Hier für Sicherheit.
    if (bmp.readTemperature() == BMP280_ERROR) {  // BMP280_ERROR = -127.0 (standardmäßige Fehlerwerte)
      temperature = NAN;  // Markiere als ungültig
      pressure = NAN;      // Markiere als ungültig
    }
  }
  
  // ---------------------------
  // Ultraschall-Entfernung lesen
  // ---------------------------
  // Nutzt measureDistanceCM()-Funktion (siehe unten) zur Messung der Entfernung in cm.
  float distance = measureDistanceCM();  

  // ---------------------------
  // RTC-Zeit abfragen
  // ---------------------------
  DateTime now = rtc.now();  // Holte aktuelle Uhrzeit vom RTC
  // Formatiere Zeit in "HH:MM:SS" (z. B. "12:34:56") in einen C-String (array)
  char timeString[20];      // Buffergröße: 20 (um "99:99:99" + Null-Terminator zu fassen)
  snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", 
           now.hour(), now.minute(), now.second());  
  // Konvertiere Zeit in Unix-Time (Sekunden seit 1970-01-01). Hilfreich für Debugging/External Systems.
  unsigned long unixTime = now.unixtime();  

  // ---------------------------
  // Daten ausgeben (Serial Monitor)
  // ---------------------------
  Serial.println(F("----------- Messdaten -----------"));  // Trennlinie
  
  // ---------------------------
  // Temperatur und Luftdruck
  // ---------------------------
  // Prüfen, ob Temperatur gültig ist (NAN? Nein)
  if (!isnan(temperature)) {  
    // Ausgabe formatieren: Symbol "🌡", Wert mit 1 Dezimalstelle, Einheit "°C"
    Serial.print(F("🌡 Temperatur: ")); Serial.print(temperature, 1); Serial.println(F(" °C"));  
    // Ausgabe formatieren: Symbol "🌬", Druckwert mit 1 Dezimalstelle, Einheit "hPa"
    Serial.print(F("🌬 Luftdruck:  ")); Serial.print(pressure, 1); Serial.println(F(" hPa"));  
  } else {  
    // Temperatur ungültig (BMP nicht verbunden/fehlerhaft) → Fehlermeldung
    Serial.println(F("🌡 BMP280: nicht verfügbar"));  
  }

  // ---------------------------
  // Ultraschall-Entfernung
  // ---------------------------
  // Prüfen, ob Entfernung gültig ist (≥0 → gültig; <0 → Timeout/Messfehler)
  if (distance >= 0) {  
    // Ausgabe formatieren: Symbol "📏", Wert mit 1 Dezimalstelle, Einheit "cm"
    Serial.print(F("📏 Entfernung: ")); Serial.print(distance, 1); Serial.println(F(" cm"));  
  } else {  
    // Entfernung ungültig → Fehlermeldung
    Serial.println(F("📏 Entfernung: keine Messung (Timeout)"));  
  }

  // ---------------------------
  // RTC-Zeit
  // ---------------------------
  // Ausgabe formatieren: Symbol "⏰", formatierten timeString
  Serial.print(F("⏰ Zeit:        ")); Serial.println(timeString);  
  // Unix-Time ausgeben (für Entwickler/External Systems)
  Serial.print(F("Unix Time:     ")); Serial.println(unixTime);  

  // Trennlinie unten
  Serial.println(F("--------------------------------\n"));  

  // ---------------------------
  // Optional: Daten an Serial1 senden (z. B. Externer Display-Controller)
  // ---------------------------
  // Temperatur an Serial1 (z. B. für grafische Anzeige auf einem separaten Gerät)
  Serial1.print(F("TEMP:")); Serial1.println(temperature);  
  // Druck an Serial1
  Serial1.print(F("PRESS:")); Serial1.println(pressure);  
  // Entfernung an Serial1
  Serial1.print(F("DIST:")); Serial1.println(distance);  
}
// ==============================================

// ---------------------------
// 7. measureDistanceCM-Funktion
// ---------------------------
// Messung der Entfernung mit dem Ultraschall-Sensor (cm). 
// Prinzip: Sendet ein Trigger-Signal, misst Echo-Dauer, berechnet Entfernung via Schallgeschwindigkeit.
// Rückgabe: Entfernung in cm (≥0 → gültig; -1 → Timeout/Messfehler)
float measureDistanceCM() {
  // Schritt 1: TRIG-Pin auf LOW setzen (bereitet Sensor auf Trigger vor)
  digitalWrite(TRIG_PIN, LOW);  
  delayMicroseconds(3);  // Warte 3µs (Ruhezeit vor Trigger)

  // Schritt 2: Trigger-Signal senden (10µs HIGH-Signal)
  digitalWrite(TRIG_PIN, HIGH);  
  delayMicroseconds(10);  // Trigger-Dauer (10µs, Standard für多数 Ultraschall-Sensoren)
  digitalWrite(TRIG_PIN, LOW);  // TRIG wieder auf LOW

  // Schritt 3: Misst die Dauer, die das Echo-Signal HIGH bleibt (Timeout nach 25ms)
  // pulseIn(pin, value, timeout): Gibt die Dauer (µs) von 'value' (HIGH) auf 'pin' zurück. Timeout in µs.
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);  // 25000µs = 25ms → Timeout falls ECHO >25ms

  // Schritt 4: Fehlerbehandlung (keine Echo-Dauer gemessen)
  if (duration == 0) {  // pulseIn retourne 0 bei Timeout oder kein Echo
    return -1;  // Keine gültige Messung → Rückgabe -1
  }

  // Schritt 5: Entfernung berechnen. Formel: Entfernung = (Echo-Dauer * Schallgeschwindigkeit) / 2
  // Schallgeschwindigkeit im Luft (bei 20°C): ~343 m/s → 343 mm/µs = 0.0343 cm/µs
  // /2, weil das Echo doppelt die Entfernung zurücklegt (Sensor → Objekt → Sensor)
  float distance = (duration * 0.0343) / 2.0;  

  return distance;  // Gib Entfernung in cm zurück
}