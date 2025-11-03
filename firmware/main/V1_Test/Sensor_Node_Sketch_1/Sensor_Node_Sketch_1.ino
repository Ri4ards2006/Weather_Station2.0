// === Master: mega_master_display.ino ===
// Dieses Sketch verwaltet einen Masterknoten, der über I2C-Schnittstelle Daten von Sklaven-Sensoren sammelt und diese auf einem TFT-Display anzeigt.

// Bibliotheken einbinden
#include <Wire.h>        // Für I2C-Kommunikation (Master/Sklaven-Protokoll)
#include <SPI.h>         // Für serielle Peripherieinterface (möglicherweise für TFT)
#include <Adafruit_GFX.h>// Abstrakte Grafik-Bibliothek von Adafruit
#include <Adafruit_ST7735.h>// Spezielle Bibliothek für ST7735 TFT-Display (z.B. 1.8" oder 2.4" Displays)

// TFT-Display-Pindefinitionen (Masterknoten)
#define TFT_CS   10  // Chip Select (CS) für TFT: steuert den Display-Selektionspin
#define TFT_DC    9  // Data/Command (DC) für TFT: trennt Kommandos von Daten
#define TFT_RST   8  // Reset-Pin (RST) für TFT: initialisiert den Display

// Initialisiere Adafruit_ST7735-Displayobjekt mit den definierten Pins
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Sklaven-Adressen (I2C-Adressen der verbundenen Sensoren)
// Formат: { 0xAdresse /*Beschreibung*/, ... } // Füge weitere Adressen hinzu, falls nötig
const uint8_t slaveAddrs[] = { 0x10 /*sensor1*/, 0x11 /*sensor2*/ }; 
const uint8_t SLAVE_COUNT = sizeof(slaveAddrs) / sizeof(slaveAddrs[0]);// Anzahl der Sklaven (automatisch berechnet)

// Letzte Poll-Zeit (für Zeitabstand zwischen Polls)
unsigned long lastPoll = 0;
// Zeitintervall zwischen vollständigen Poll-Cycles (1500ms = 1.5 Sekunden)
const unsigned long POLL_INTERVAL = 1500; 
// Verzögerung zwischen Einzelabfrage für jeden Sklaven (100ms)
const unsigned long PER_SLAVE_DELAY = 100; 

// Struktur zur Speicherung von Sklaven-Daten
struct SlaveData {
  uint8_t id;          // Sklaven-ID (I2C-Adresse)
  float temp;          // Aufgezeichnete Temperatur (°C)
  float pressure;      // Aufgezeichnete Luftdruck (hPa)
  float altitude;      // Aufgezeichnete Höhe (m) [aktuellerlich nicht angezeigt]
  float distance;      // Aufgezeichnete Distanz (cm)
  int ampel;           // Zustandsindikator (z.B. für Ampelanzeige: 0=Rot, 1=Rot-Gelb, 2=Grün, 3=Gelb)
  bool valid;          // Gültigkeitsflag (true = Daten sind aktuell, false = ungültig)
  unsigned long lastSeen; // Zeitstempel der letzten gültigen Datenempfang (millis())
};

// Array zur Speicherung von bis zu 8 Sklaven-Daten (passend zur Größe von slaveAddrs)
SlaveData slaves[8]; 

// Initialisierungsfunktion (wird einmal bei Start aufgerufen)
void setup() {
  Serial.begin(115200); // serieller Monitor für Debugging (115200 Baud)
  
  // I2C-Master initialisieren (Wire_library)
  Wire.begin(); 
  
  // TFT-Display initialisieren
  tft.initR(INITR_REDTAB); // spezielle Initialisierung für ROT-Tab-Schema (Farbpallete)
  tft.fillScreen(ST77XX_BLACK); // Display vollständig schwarz füllen
  tft.setRotation(1); // Display-Rotation einstellen (1 = 90°, portrait-Modus)
  tft.setTextWrap(false); // Text Wrap deaktivieren (keine Zeilenumbrüche bei langem Text)
  
  // Sklaven-Daten initialisieren (alle als ungültig markieren)
  for (uint8_t i=0; i<8; i++) { 
    slaves[i].valid = false; 
    slaves[i].id = 0; 
  }
  
  // Willkommensnachricht auf Display anzeigen
  tft.setTextColor(ST77XX_WHITE); // Textfarbe: Weiß
  tft.setTextSize(1); // Textgröße: Standard
  tft.setCursor(0,0); // Cursor an Position (0,0) setzen
  tft.println("WeatherStation 2.0 - Master"); // Nachricht ausgeben
  delay(800); // 800ms warten, um die Nachricht zu sehen
  
  // Display zurücksetzen (schwarz)
  tft.fillScreen(ST77XX_BLACK);
}

// Haupt-Schleife (wird unendlich wiederholt)
void loop() {
  unsigned long now = millis(); // aktuelle Zeit seit Start (ms)
  
  // Prüfen, ob seit letztem Poll mehr als POLL_INTERVAL vergangen ist
  if (now - lastPoll >= POLL_INTERVAL) {
    pollAllSlaves(); // Alle Sklaven abfragen
    lastPoll = now; // Letzte Poll-Zeit aktualisieren
    renderAll(); // Daten auf Display rendern
  }
}

// Funktion zum Abfragen aller Sklaven (nacheinander)
void pollAllSlaves() {
  for (uint8_t i=0; i<SLAVE_COUNT; i++) { // Schleife über alle definierten Sklaven
    uint8_t addr = slaveAddrs[i]; // aktuelle Sklaven-Adresse holen
    requestFromSlave(addr); // Daten von Sklaven anfordern
    delay(PER_SLAVE_DELAY); // kleine Verzögerung, um I2C-Bus nicht zu überlasten
  }
}

// Funktion zum Abfragen eines einzelnen Sklaven und Behandlung der Antwort
void requestFromSlave(uint8_t addr) {
  Wire.requestFrom(addr, (uint8_t)64); // Von Sklaven mit Adresse 'addr'最多 64 Bytes anfordern
  
  String reply = ""; // Empfangene Antwort als String speichern
  unsigned long start = millis(); // Startzeit zur Timeout-Überwachung
  
  // Empfangene Daten sammeln (bis zu 200ms Timeout)
  while (Wire.available()) { 
    char c = Wire.read(); //einzelnes Byte lesen
    reply += c; //hinzufügen zum Antwort-String
    
    // Sicherheits-Timeout: Beende Schleife, wenn 200ms vergangen sind
    if (millis() - start > 200) break; 
  }
  
  // Prüfen, ob Daten empfangen wurden
  if (reply.length() > 0) {
    Serial.print("Reply from 0x"); Serial.print(addr, HEX); Serial.print(": "); Serial.println(reply); // Debug-Nachricht
    parseAndStore(reply); // Daten parsen und im SlaveData-Array speichern
  } else {
    Serial.print("No reply from 0x"); Serial.println(addr, HEX); // Debug-Nachricht (keine Antwort)
    
    // Sklaven als ungültig markieren (wenn er in der Liste existiert)
    for (uint8_t j=0; j<8; j++) { 
      if (slaves[j].valid && slaves[j].id == addr) { 
        slaves[j].valid = false; 
        break; 
      }
    }
  }
}

// Funktion zum Parsen der empfangenen String-Daten und Speichern in SlaveData-Struktur
void parseAndStore(const String &s) {
  // Erwarteter Format: "ID,TEMP,PRESS,ALT,DIST,AMP"
  // Beispieldaten: "0x10,23.5,1013.2,150.0,5.5,2"
  
  int idx = 0; // Hilfsvariable (nicht verwendet, vielleicht Überreste)
  int start = 0; // Startposition für Substring
  String parts[8]; // Array zum Aufteilen des Strings (maximal 8 Teile)
  
  // Teile-Array initialisieren (alle leere Strings)
  for (int i=0; i<7; i++) parts[i] = "";
  
  int p = 0; // Anzahl der geteilten Teile
  
  // Trenne den String an Kommas (',')
  for (int i=0; i<s.length(); i++) {
    if (s[i] == ',') {
      parts[p++] = s.substring(start, i); // Teil von start bis i-1 speichern
      start = i+1; // Startposition für nächsten Teil aktualisieren
    }
  }
  // Letzten Teil (nach dem letzten Komma) hinzufügen
  parts[p++] = s.substring(start); 
  
  // Prüfe, ob genügend Teile existieren (mindestens 6: ID, TEMP, PRESS, ALT, DIST, AMP)
  if (p < 6) {
    Serial.println("Parse error: insufficient tokens"); // Debug-Fehler
    return; // Abbruch, falls nicht genug Teile
  }
  
  // Parke Sklaven-ID (Teil 0) aus String in uint8_t (hex oder dezimal)
  String idStr = parts[0]; 
  uint8_t id = (uint8_t) strtol(idStr.c_str(), NULL, 0); // strtol: Konvertiert String zu Zahl (Auto-Basis)
  
  // Finde passenden Slot im SlaveData-Array:
  int slot = -1; 
  // 1. Suchen nach vorhandenem gültigen Sklaven mit gleicher ID
  for (int i=0; i<8; i++) {
    if (slaves[i].valid && slaves[i].id == id) { 
      slot = i; 
      break; 
    }
  }
  // 2. Wenn kein gültiger Slot gefunden: Suche ersten freien Slot
  if (slot == -1) {
    for (int i=0; i<8; i++) {
      if (!slaves[i].valid) { 
        slot = i; 
        break; 
      }
    }
  }
  // 3. Kein freier Slot vorhanden?
  if (slot == -1) { 
    Serial.println("No slot free for slave"); // Debug-Fehler
    return; 
  }
  
  // Sklaven-Daten aktualisieren
  slaves[slot].id = id;          // ID speichern
  slaves[slot].valid = true;     // Gültigkeitsflag setzen
  slaves[slot].lastSeen = millis();// Zeitstempel für letzte Datenaktualisierung
  
  // Parke numerische Werte (Fliesskommazahlen) sicher
  slaves[slot].temp = parseFloatToken(parts[1]);  // Teil 1: Temperatur
  slaves[slot].pressure = parseFloatToken(parts[2]); // Teil 2: Luftdruck
  slaves[slot].altitude = parseFloatToken(parts[3]); // Teil 3: Höhe [nicht angezeigt]
  slaves[slot].distance = parseFloatToken(parts[4]); // Teil 4: Distanz
  slaves[slot].ampel = parts[5].toInt(); // Teil 5: Ampelzustand (Integer)
}

// Hilfsfunktion zum sicheren Parsen von Float-Werten (behandelt 'nan' oder leere Strings)
float parseFloatToken(const String &tok) {
  // Prüfe auf leeren String oder 'nan'/'NaN'
  if (tok.length() == 0 || tok == "nan" || tok == "NaN") { 
    return NAN; // Kein gültiger Wert -> Rückgabe von NAN (Not a Number)
  }
  return tok.toFloat(); // Konvertiere String zu Float
}

// Funktion zum Rendern aller gültigen Sklaven-Daten auf dem TFT-Display
void renderAll() {
  tft.fillScreen(ST77XX_BLACK); // Display vorneu laden (schwarz)
  
  // Titel-Text einstellen
  tft.setCursor(0,0); // Cursor an (0,0)
  tft.setTextColor(ST77XX_WHITE); // Textfarbe Weiß
  tft.setTextSize(1); // Textgröße Standard
  tft.println("WeatherStation 2.0"); // Titel anzeigen
  tft.println("---------------------------"); // Trennlinie
  
  int y = 24; // Start-Y-Position für Datenzeilen (nach Titel)
  
  // Schleife über alle Sklaven-Daten im Array
  for (int i=0; i<8; i++) {
    if (!slaves[i].valid) continue; // Überspringe ungültige/leere Einträge
    
    // Sklaven-ID anzeigen
    tft.setCursor(0, y); 
    tft.setTextSize(1); 
    tft.setTextColor(ST77XX_CYAN); // Cyan für ID-Text
    tft.print("ID 0x"); 
    tft.print(slaves[i].id, HEX); // ID in hexadezimaler Darstellung ausgeben
    
    // Temperatur, Druck, Distanz anzeigen (Y+10 = nächste Zeile)
    tft.setCursor(0, y+10); 
    tft.setTextColor(ST77XX_WHITE); 
    
    // Temperatur: Falls nicht NAN
    if (!isnan(slaves[i].temp)) {
      tft.print("T: "); 
      tft.print(slaves[i].temp, 1); // 1 Dezimalstelle anzeigen
      tft.print(" C  "); 
    } else {
      tft.print("T: N/A  "); // Keine gültigen Daten: 'N/A' anzeigen
    }
    
    // Druck: Falls nicht NAN
    if (!isnan(slaves[i].pressure)) {
      tft.print("P: "); 
      tft.print(slaves[i].pressure, 1); 
      tft.print(" hPa "); 
    } else {
      tft.print("P: N/A "); 
    }
    
    // Distanz: Falls nicht NAN
    if (!isnan(slaves[i].distance)) {
      tft.print(" D:"); 
      tft.print(slaves[i].distance, 1); 
      tft.print("cm "); 
    } else {
      tft.print(" D:N/A "); 
    }
    
    // Ampelzustand in nächste Zeile (Y+20)
    tft.setCursor(0, y+20); 
    tft.print("State: "); 
    
    // Mappe Ampel-ID zu Zustandsbeschreibung (Switch-Case)
    switch (slaves[i].ampel) {
      case 0: tft.print("ROT"); break;       // 0 -> Rot
      case 1: tft.print("ROT_GELB"); break;  // 1 -> Rot-Gelb
      case 2: tft.print("GRUEN"); break;     // 2 -> Grün
      case 3: tft.print("GELB"); break;      // 3 -> Gelb
      default: tft.print("N/A"); break;      // Unbekannte ID -> N/A
    }
    
    // Nächste Sklaven-Zeile (Y um 36 erhöhen)
    y += 36; 
    // Stoppe, wenn Y über 140 (Display-Höhe) geht, um Überfluss zu vermeiden
    if (y > 140) break; 
  }
}