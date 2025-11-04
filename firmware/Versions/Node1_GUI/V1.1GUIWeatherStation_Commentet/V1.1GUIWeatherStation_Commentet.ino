/* 
  Wetterstation-UI v2.0 für Arduino mit TFT-Display (MCUFRIEND_kbv)
  Autor: [Dein Name] (ersetze mit deinem Namen)
  Datum: [Aktuelles Datum] (ersetze mit dem aktuellen Datum)
  GitHub-Repository: [Dein Repository-Link] (ersetze mit deinem Link)
  
  Dieses Projekt implementiert eine nutzerfreundliche Graphical User Interface (GUI) zur Anzeige von Wetterdaten 
  (Temperatur, Luftfeuchtigkeit, Luftdruck) auf einem TFT-Display. Die UI ist skalierbar, kompatibel mit 
  verschiedenen Displaymodellen und umfasst eine Zeitstempelanzeige für Update-Transparenz.
*/

// ----------------------------------------------------------------------------
// --- 1. Bibliotheken einbinden (Abhängigkeiten) ---
// ----------------------------------------------------------------------------
#include <Adafruit_GFX.h>    // Basisklasse für Grafikoperationen (Linien, Text, Formen)
#include <MCUFRIEND_kbv.h>   // Display-Treiber für das MCUFRIEND_kbv-TFT-Modell (SPI-Schnittstelle)
#include <string.h>          // Für C-String-Funktionen wie strlen(), optional aber für Konsistenz

// ----------------------------------------------------------------------------
// --- 2. Farbdefinitionen (RGB565-Format) ---
// ----------------------------------------------------------------------------
/* 
  TFT-Displays verwenden oft das 16-Bit RGB565-Format (5 Bit Rot, 6 Bit Grün, 5 Bit Blau).
  Jede Farbe wird hier über einen Hex-Code definiert, der direkt vom Display interpretiert wird.
*/
#define BLACK       0x0000    // Vollständig schwarz (Hintergrund des gesamten Screens)
#define WHITE       0xFFFF    // Vollständig weiß (Ränder, Zeitstempel)
#define TITLE_BLUE  0x001F    // Intensivblau (Titel "Wetterstation" – hervorzuheben)
#define NAME_GRAY   0x4444    // Hellgrau (Metrik-Namen wie "Temperatur: " – subtile Schrift)
#define VALUE_RED   0xF800    // Rot (Werte wie 22.5°C – klare Lesbarkeit)
#define BG_LIGHT    0xD3D3    // Helliggrau (Hintergrund des UI-Bereichs – Kontrast zum schwarzen Screen)
#define BORDER_WHITE 0xFFFF   // Weiß (äußerer Rahmen des UI-Bereichs – optische Abgrenzung)

// ----------------------------------------------------------------------------
// --- 3. Textgröße-Konstanten (Pixelberechnungen) ---
// ----------------------------------------------------------------------------
/* 
  Adafruit_GFX bietet keine eingebaute textWidth()-Methode, daher müssen wir manuell die Breite von Texten 
  berechnen. Hier definieren wir die Pixelbreite pro Zeichen für unterschiedliche Schriftgrößen (TextSize).
  Diese Werte sind spezifisch für die Standard-Schriftart von Adafruit_GFX. Anpassen, falls eine andere Schriftart verwendet wird.
*/
#define TEXT_SIZE_3_WIDTH_PER_CHAR 14  // Breite pro Zeichen bei TextSize(3) (Große Schrift für Titel)
#define TEXT_SIZE_3_HEIGHT          24 // Höhen eines Textes bei TextSize(3) (Pixel)
#define TEXT_SIZE_2_WIDTH_PER_CHAR  12  // Breite pro Zeichen bei TextSize(2) (Normale Schrift für Metriken)
#define TEXT_SIZE_2_HEIGHT          20 // Höhen eines Textes bei TextSize(2) (Pixel)

// ----------------------------------------------------------------------------
// --- 4. Display-Pindefinitionen ---
// ----------------------------------------------------------------------------
/* 
  Der TFT-Display (MCUFRIEND_kbv) kommuniziert über den SPI-Bus. Hier definieren wir die Pins der Arduino,
  die für die Display-Kontrolle verantwortlich sind.
  
  Pins müssen je nach Hardware angepasst werden! Beispiel: Für einen 2.4"-Display mit SPI.
*/
#define TFT_CS    53   // Chip Select (CS): Steuert, ob der Display der SPI-Kommunikation beitritt (LOW = aktiv)
#define TFT_RS     9   // Register Select (RS): Distinguiert zwischen Befehl (LOW) und Daten (HIGH)
#define TFT_RST    8   // Reset (RST): Initialisiert den Display (LOW = Reset, HIGH = Normalbetrieb)
#define BACKLIGHT 13  // Backlight-Pin: Steuert die Hintergrundbeleuchtung (HIGH = ein, LOW = aus – falls Display es erfordert)

// ----------------------------------------------------------------------------
// --- 5. Globale Variablen ---
// ----------------------------------------------------------------------------
/* 
  Globale Variablen werden außerhalb von Funktionen definiert, damit sie überall im Code zugreifbar sind.
  Sie speichern Zustandsinformationen (z. B. aktuelle Zeit, Display-Größe) und initialisieren das Display-Objekt.
*/
MCUFRIEND_kbv tft(&SPI, TFT_CS, TFT_RS, TFT_RST);  // Display-Objekt: Erstellt eine Instanz mit SPI und Pin-Definitionen
int screenWidth, screenHeight;   // Dynamische Display-Größe (erhalten via tft.width()/height() nach Initialisierung)
int displayLeft = 20;               // Linker Rand des UI-Bereichs (Pixel – Abstand vom Display-Edge links)
int displayTop = 30;                // Oberer Rand des UI-Bereichs (Pixel – Abstand vom Display-Edge oben)
int displayWidth;                    // Breite des UI-Bereichs (berechnet als screenWidth - 2*displayLeft)
int displayHeight = 200;            // Höhen des UI-Bereichs (Pixel – kann angepasst werden für mehr/weniger Inhalte)
String currentTime = "00:00";       // Aktueller Zeitstempel (Format: Minuten:Sekunden – zeigt Update-Zeit)
unsigned long lastTimeUpdate = 0;   // Letzte Zeit, zu der der Zeitstempel aktualisiert wurde (ms – für Rate-Steuerung)

// ----------------------------------------------------------------------------
// --- 6. Function Prototypes (Compiler-Voranmeldung) ---
// ----------------------------------------------------------------------------
/* 
  Der Arduino-Compiler benötigt Vorankündigungen (Prototypen) von Funktionen, die später im Code definiert werden.
  Ohne Prototypen könnte es zu Fehlern wie "Funktion nicht gefunden" kommen.
*/
void updateWeatherUI(float temp, float humidity, float pressure);  // UI aktualisieren mit Werten
void updateWeatherUI();                                           // UI aktualisieren ohne Werte (Standard)
void drawDisplayArea();                                           // Zeichnet den UI-Bereich mit Rahmen
void drawMetric(int y, String name, float value, String unit, int decimals);  // Zeichnet eine Metrik (Name + Wert)
void drawSeparator(int y);                                        // Zeichnet eine horizontale Trennlinie

// ----------------------------------------------------------------------------
// --- 7. Setup-Funktion (Hardware-Initialisierung) ---
// ----------------------------------------------------------------------------
void setup() {
  // 7.1 Serielle Kommunikation starten (Baudrate 9600 – Standard für Debugging)
  Serial.begin(9600);
  Serial.println("Wetterstation UI Initialisierung...");

  // 7.2 Backlight einrichten (Display sichtbar machen)
  pinMode(BACKLIGHT, OUTPUT);          // Pin als Output konfigurieren (LED-Steuerung)
  digitalWrite(BACKLIGHT, HIGH);      // Backlight einschalten. ACHTUNG: Manche Displays brauchen LOW hier! Prüfen via seriellen Monitor.

  // 7.3 SPI-Bus initialisieren (Grundlage für Display-Kommunikation)
  SPI.begin();                         // Startet den SPI-Controller der Arduino. Wichtig für Datenübertragung zum Display.
  Serial.println("SPI-Bus initialisiert.");

  // 7.4 Display-ID erkennen (Kompatibilität mit Clones)
  uint16_t tftID = tft.readID();       // Liest die eindeutige ID des Displays (z. B. 0x9486 für Original, 0xD3D3 für Clones)
  Serial.print("TFT ID: 0x"); Serial.println(tftID, HEX);  // Debug-Ausgabe der ID (hilft bei Troubleshooting)
  
  // 7.4.1 Fallback für bekannte Clone-IDs (z. B. 0xD3D3 steht für ein häufiges Clone-Modell)
  if (tftID == 0xD3D3) {  
    tftID = 0x9486; 
    Serial.println("Fallback-ID (0x9486) verwendet, da ein Clone-Display detektiert wurde.");
  }

  // 7.5 Display initialisieren (Konfiguration der Display-Hardware)
  tft.begin(tftID);                    // Initialisiert das Display mit der ermittelten ID. Ohne dies zeigt das Display nichts.
  tft.setRotation(1);                 // Rotiert das Display um 90° (Landscape-Modus). 1 = 90°, 2 = 180°, 3 = 270°. Hier: Breite horizontal.
  Serial.println("TFT initialisiert.");

  // 7.6 Dynamische Display-Größe ermitteln (wichtig für skalierbares Layout)
  /* 
    Nach der Initialisierung können wir die tatsächliche Auflösung des Displays erfragen.
    screenWidth: Breite des Displays in Pixel (z. B. 240 für 2.4"-Display)
    screenHeight: Höhe des Displays in Pixel (z. B. 320 für 2.4"-Display)
    displayWidth: UI-Breite (screenWidth - 2*displayLeft) – entfernt linke und rechte Ränder
  */
  screenWidth = tft.width();          // Ermittelt die tatsächliche Breite
  screenHeight = tft.height();        // Ermittelt die tatsächliche Höhe
  displayWidth = screenWidth - 2 * displayLeft;  // UI-Breite berechnen (ohne Rand)

  // 7.7 Start-UI anzeigen (Initialanzeige mit Standardwerten)
  updateWeatherUI();                   // Ruft updateWeatherUI() ohne Argumente auf (verwendet Standardwerte 0.0)
  delay(5000);                         // 5 Sekunden Pause, um die Init-Anzeige zu betrachten
}

// ----------------------------------------------------------------------------
// --- 8. Loop-Funktion (Haupt-Schleife) ---
// ----------------------------------------------------------------------------
void loop() {
  // 8.1 Wetterdaten generieren (Simulationsmodus oder echte Sensoren)
  bool simulateData = true;            // Flag: true = Simulieren, false = Echte Sensoren verwenden
  float temp, humidity, pressure;      // Variablen für die Wetterdaten (Temperatur in °C, Feuchtigkeit in %, Druck in hPa)

  if (simulateData) {
    // 8.1.1 Simulierte Daten (plausibel für Test ohne Sensoren)
    /* 
      random(15, 35): Generiert eine Ganzzahl zwischen 15 und 34 (einschließlich). Basistemperatur.
      (float)random(0, 10)/10.0: Generiert eine Ziffer zwischen 0.0 und 0.9 (z. B. 0.5) – fügt eine Dezimalstelle hinzu.
      Ergebnis: temp zwischen 15.0°C und 34.9°C.
    */
    temp = random(15, 35) + (float)random(0, 10)/10.0; 
    humidity = random(30, 90);         // Feuchtigkeit: 30-89% (typische Werte für Indoor)
    pressure = random(980, 1030);      // Luftdruck: 980-1029 hPa (typisch für mittlere Höhen)
  } else {
    // 8.1.2 Echte Sensoren (Beispiel; pins und Library müssen definiert sein!)
    /* 
      Für echte Sensoren:
      1. Installiere die passende Bibliothek (z. B. DHT für Temperatur/Feuchtigkeit, BMP280 für Druck).
      2. Definiere Sensor-Pins oben im Code (z. B. #define DHT_PIN 2).
      3. Initialisiere den Sensor im setup() (z. B. dht.begin()).
      4. Ersetze die folgenden Testwerte durch Sensor-Read-Funktionen.
    */
    // Beispiel-DHT-Aufrufe (nur wenn Library und Pins definiert sind):
    // temp = dht.readTemperature();    // °C (Falls DHT11/DHT22 verwendet)
    // humidity = dht.readHumidity();   // % (Falls DHT11/DHT22 verwendet)
    // Beispiel-BMP-Aufruf (nur wenn Library und Pins definiert sind):
    // pressure = bmp.readPressure() / 100.0;  // Umrechnen von Pa zu hPa (BMP280 liefert Pa)
    temp = 22.5; humidity = 55; pressure = 1013;  // Testwerte (Standard, wenn Sensoren nicht angeschlossen)
  }

  // 8.2 UI aktualisieren (zeigt die neuesten Daten an)
  updateWeatherUI(temp, humidity, pressure);  // Ruft die UI-Aktualisierungs-Funktion mit den generierten/liesenen Werten auf

  // 8.3 Zeitstempel aktualisieren (alle 1 Sekunde)
  unsigned long currentMillis = millis();     // Millisekunden seit Start (TickCount)
  if (currentMillis - lastTimeUpdate >= 1000) {  // Prüft, ob 1 Sekunde vergangen ist
    lastTimeUpdate = currentMillis;          // Speichert die aktuelle Zeit als letztes Update
    int totalSec = currentMillis / 1000;      // Gesamte Sekunden seit Start (currentMillis / 1000)
    int min = totalSec / 60;                 // Minuten: totalSec dividiert durch 60
    int sec = totalSec % 60;                 // Sekunden: totalSec modulo 60 (Rest nach Division)
    
    /* 
      Formatierung von Minuten/Sekunden mit führender Null (z. B. 5 Min. → "05", 3 Sek. → "03")
      (min < 10 ? "0" : ""): Ternärer Operator. Fügt "0" hinzu, wenn Minuten < 10, sonst "".
      Gleiches für sekunden.
    */
    currentTime = (min < 10 ? "0" : "") + String(min) + ":" + 
                  (sec < 10 ? "0" : "") + String(sec); 
  }

  // 8.4 Warte 1 Sekunde (Aktualisierungsrate: 1Hz)
  delay(1000);  // Ohne delay() würde die Schleife so schnell wie möglich laufen – zu hohe Rate kann das Display überlasten.
}

// ----------------------------------------------------------------------------
// --- 9. updateWeatherUI() – UI aktualisieren (zentrierte Metriken) ---
// ----------------------------------------------------------------------------
/* 
  Zentrale Funktion zum Zeichnen des gesamten UI-Inhalts. 
  Parameter:
    - temp: Temperatur in °C (float)
    - humidity: Luftfeuchtigkeit in % (float)
    - pressure: Luftdruck in hPa (float)
  Rückgabe: Keine (void)
  Zweck: Zeichnet Titel, Metriken und Zeitstempel auf dem Display.
*/
void updateWeatherUI(float temp, float humidity, float pressure) {
  // 9.1 Hintergrund löschen (schwarzer Screen)
  tft.fillScreen(BLACK);  // Füllt das gesamte Display mit schwarz, um alten Inhalt zu overwrite.

  // 9.2 UI-Bereich mit Rahmen zeichnen (displayLeft, displayTop, displayWidth, displayHeight)
  drawDisplayArea();      // Ruft die Hilfsfunktion auf, um den UI-Bereich (Box) zu zeichnen.

  // 9.3 Titel "Wetterstation" zeichnen (groß, blau, zentriert)
  tft.setTextSize(3);         // Schriftgröße auf 3 setzen (große Schrift für Titel)
  tft.setTextColor(TITLE_BLUE); // Textfarbe: Intensivblau (hervorzuheben)
  String title = "Wetterstation"; // Titel-Text
  int titleWidth = title.length() * TEXT_SIZE_3_WIDTH_PER_CHAR;  // Breite des Titels (Anzahl Zeichen * Pixel/Zeichen)
  int titleX = displayLeft + (displayWidth / 2) - (titleWidth / 2);  // X-Koordinate: Mitte des UI-Bereichs (zentriert)
  int titleY = displayTop + 20;  // Y-Koordinate: 20 Pixel unter dem oberen Rand des UI-Bereichs
  tft.setCursor(titleX, titleY); // Cursor an Position (titleX, titleY) setzen
  tft.print(title);               // Titel auf Display drucken

  // 9.4 Metrikenbereich vorbereiten (kleine Schrift, Grau/Rot)
  tft.setTextSize(2);          // Schriftgröße auf 2 setzen (normale Schrift für Details)
  tft.setTextColor(NAME_GRAY); // Textfarbe für Namen: Hellgrau (subtil, Hintergrund nicht überdecken)
  int metricY = titleY + TEXT_SIZE_3_HEIGHT + 15;  // Y-Koordinate: Unten vom Titel (Titel-Höhe + 15 Pixel Abstand)

  // 9.5 Temperatur-Metrik zeichnen (1 Dezimalstelle)
  drawMetric(metricY, "Temperatur: ", temp, "°C", 1);  // Ruft Metrik-Funktion auf mit Namen, Wert, Einheit und Dezimalen
  metricY += TEXT_SIZE_2_HEIGHT + 10;  // Nächste Zeile (Höhe der Schriftgröße 2 + 10 Pixel Abstand)

  // 9.6 Trennlinie (Temperatur ↔ Feuchtigkeit)
  drawSeparator(metricY);       // Zeichnet eine horizontale Linie (Grau) zur Optischen Trennung
  metricY += 5;                 // 5 Pixel Abstand nach der Linie

  // 9.7 Feuchtigkeit-Metrik zeichnen (keine Dezimalen)
  drawMetric(metricY, "Feuchtigkeit: ", humidity, "%", 0);  // Metrik ohne Dezimalen (ganze Zahlen)
  metricY += TEXT_SIZE_2_HEIGHT + 10;  // Nächste Zeile

  // 9.8 Trennlinie (Feuchtigkeit ↔ Luftdruck)
  drawSeparator(metricY);
  metricY += 5;                 // Abstand nach Linie

  // 9.9 Luftdruck-Metrik zeichnen (keine Dezimalen)
  drawMetric(metricY, "Luftdruck: ", pressure, " hPa", 0);  // Metrik mit Leerzeichen vor Einheit (Optik)
  metricY += TEXT_SIZE_2_HEIGHT + 10;  // Nächste Zeile (nicht nötig, aber für künftige Metriken vorgesehen)

  // 9.10 Zeitstempel zeichnen (unten, zentriert, weiß)
  tft.setTextSize(2);          // Gleiche Schriftgröße wie Metriken für Konsistenz
  tft.setTextColor(WHITE);     // Textfarbe: Weiß (hervorzuheben im UI-Bereich)
  String timeStr = "Aktualisiert: " + currentTime;  // Zeittext mit Vorinfo
  int timeWidth = timeStr.length() * TEXT_SIZE_2_WIDTH_PER_CHAR;  // Breite des Zeittexts
  int timeX = displayLeft + (displayWidth / 2) - (timeWidth / 2);  // X-Koordinate: Mitte des UI-Bereichs (zentriert)
  int timeY = displayTop + displayHeight - 30;  // Y-Koordinate: 30 Pixel über dem unteren Rand des UI-Bereichs
  tft.setCursor(timeX, timeY);                 // Cursor an Position setzen
  tft.print(timeStr);                           // Zeitstempel auf Display drucken
}

// ----------------------------------------------------------------------------
// --- 10. updateWeatherUI() – UI ohne Daten (Standardwerte) ---
// ----------------------------------------------------------------------------
/* 
  Überladene Funktion (ohne Parameter). Ruft die mit Parametern versehene Version auf mit Standardwerten.
  Zweck: Start-Aufruf (bevor Sensoren initialisiert sind) oder Fallback, falls Daten fehlen.
*/
void updateWeatherUI() {
  updateWeatherUI(0.0, 0.0, 0.0);  // Standardwerte: 0.0°C, 0.0%, 0.0hPa
}

// ----------------------------------------------------------------------------
// --- 11. drawDisplayArea() – UI-Bereich mit Rahmen zeichnen ---
// ----------------------------------------------------------------------------
/* 
  Zeichnet den sichtbaren Bereich (Box) des Displays mit gerundeten Ecken und Rändern.
  Zweck: Definiert den visuellen Rahmen für Datenanzeige, verbessert Design und Kontrast.
*/
void drawDisplayArea() {
  // 11.1 Hintergrund des UI-Bereichs (hellgrau, gerundete Ecken)
  /* 
    fillRoundRect(x, y, width, height, radius, color):
    - x/y: Oben links Position des Rechtecks (displayLeft, displayTop)
    - width/height: Größen des UI-Bereichs (displayWidth, displayHeight)
    - radius: Gerundete Ecken (15 Pixel – kann angepasst werden für stärkere/stärkere Rundungen)
    - color: BG_LIGHT (hellgrau)
  */
  tft.fillRoundRect(displayLeft, displayTop, displayWidth, displayHeight, 15, BG_LIGHT);

  // 11.2 Äußerer Rahmen (weiß, dicker)
  /* 
    drawRect(x, y, width, height, color):
    - x/y: 3 Pixel links/oben des UI-Bereichs (displayLeft-3, displayTop-3)
    - width/height: UI-Bereich + 6 Pixel (displayWidth+6, displayHeight+6) – kompensiert den 3 Pixel Rand
    - color: BORDER_WHITE (weiß) – optische Abgrenzung vom Screen-Hintergrund
  */
  tft.drawRect(displayLeft - 3, displayTop - 3, displayWidth + 6, displayHeight + 6, BORDER_WHITE);

  // 11.3 Innerer Rahmen (hellgrau, subtil)
  /* 
    Innerer Rand um den UI-Bereich, um den Inhalt von Hintergrund zu trennen.
    x/y: displayLeft, displayTop (gleiche Position wie UI-Bereich)
    width/height: displayWidth, displayHeight (gleiche Größen)
    color: NAME_GRAY (hellgrau) – passt zur Metrik-Namen-Farbe
  */
  tft.drawRect(displayLeft, displayTop, displayWidth, displayHeight, NAME_GRAY);
}

// ----------------------------------------------------------------------------
// --- 12. drawMetric() – Metrik (Name + Wert) formatieren und zeichnen ---
// ----------------------------------------------------------------------------
/* 
  Zeichnet eine einzelne Metrik (z. B. "Temperatur: 22.5°C") mit Name linksbündig und Wert rechtsbündig.
  Parameter:
    - y: Y-Koordinate, wo die Metrik gezeichnet wird (Pixel)
    - name: Metrik-Name (z. B. "Temperatur: " als String)
    - value: Metrik-Wert (float – z. B. 22.5)
    - unit: Einheit (z. B. "°C" als String)
    - decimals: Anzahl Dezimalstellen (0 für ganze Zahlen, 1 für eine Dezimalstelle)
  Zweck: Verhindert Overlap von Name und Wert, optimiert Layout für verschiedene Textlängen.
*/
void drawMetric(int y, String name, float value, String unit, int decimals) {
  // 12.1 Metrik-Name zeichnen (linksbündig, Grau)
  tft.setTextColor(NAME_GRAY);       // Name-Farbe: Hellgrau
  tft.setCursor(displayLeft + 2, y);  // Cursor 2 Pixel rechts vom UI-Linker Rand (Abstand)
  tft.print(name);                    // Namen (z. B. "Temperatur: ") drucken

  // 12.2 Wert formatieren und vorbereiten (Rot)
  String valueStr = String(value, decimals) + unit;  // Wert mit Dezimalen + Einheit (z. B. "22.5°C")
  tft.setTextColor(VALUE_RED);      // Wert-Farbe: Rot (hervorzuheben)

  // 12.3 Textbreiten berechnen (manuell, da Adafruit_GFX keine textWidth()-Methode bietet)
  int nameWidth = name.length() * TEXT_SIZE_2_WIDTH_PER_CHAR;  // Breite des Namens (Zeichen * Pixel/Zeichen)
  int valueWidth = valueStr.length() * TEXT_SIZE_2_WIDTH_PER_CHAR;  // Breite des Werts (Zeichen * Pixel/Zeichen)

  // 12.4 Prüfung: Ist Name+Wert breiter als UI-Bereich? (Anti-Overlap)
  /* 
    displayWidth - 40: UI-Breite minus 40 Pixel (Abstand links/rechts innerhalb des UI-Bereichs)
    Falls Name + Wert breiter wird, platziert der Wert rechtsbündig, um Overlap zu vermeiden.
  */
  if (nameWidth + valueWidth > (displayWidth - 40)) { 
    // 12.4.1 Wert rechtsbündig zeichnen (falls zu breit)
    int valueX = displayLeft + displayWidth - valueWidth - 20;  // X-Koordinate: UI-Rechter Rand - Wert-Breite - 20 Pixel Abstand
    tft.setCursor(valueX, y);  // Cursor an berechnete Position setzen
    tft.print(valueStr);        // Wert drucken
    return;                     // Funktion beenden, da Wert gezeichnet wurde
  }

  // 12.5 Normaler Fall: Wert nach Name zeichnen (mit Abstand)
  /* 
    valueX = UI-Linker Rand + 2 (Abstand) + Name-Breite + 20 (Abstand zwischen Name und Wert)
    Positioniert den Wert 20 Pixel rechts vom Namen.
  */
  int valueX = displayLeft + 2 + nameWidth + 20; 
  tft.setCursor(valueX, y);    // Cursor an Position setzen
  tft.print(valueStr);          // Wert drucken
}

// ----------------------------------------------------------------------------
// --- 13. drawSeparator() – Horizontale Trennlinie zeichnen ---
// ----------------------------------------------------------------------------
/* 
  Zeichnet eine horizontale Linie (Grau) zur Optischen Trennung von Metriken.
  Parameter:
    - y: Y-Koordinate der Linie (Pixel)
  Zweck: Verbessert die UI-Struktur, macht Metriken separierbar.
*/
void drawSeparator(int y) {
  tft.setTextColor(NAME_GRAY);  // Linienfarbe: Hellgrau (passt zum Namen-Design)
  /* 
    drawLine(xStart, y, xEnd, y, color):
    - xStart: displayLeft + 10 (10 Pixel rechts vom UI-Linker Rand)
    - xEnd: displayLeft + displayWidth - 10 (10 Pixel links vom UI-Rechter Rand)
    - y: Y-Koordinate (gleich für Start und End)
    - color: NAME_GRAY (hellgrau)
    Ergebnis: Linie von links zur rechten Seite des UI-Bereichs (ohne Ränder)
  */
  tft.drawLine(displayLeft + 10, y, displayLeft + displayWidth - 10, y, NAME_GRAY);
}

/* 
  --- TIPPS FÜR ANPASCHUNG ---
  
  1. Echte Sensoren integrieren:
     - DHT11/DHT22 (Temperatur/Feuchtigkeit):
       a. Installiere die Bibliothek via Library Manager ("DHT sensor library").
       b. Definiere oben: #define DHT_PIN 2; #define DHT_TYPE DHT22; DHT dht(DHT_PIN, DHT_TYPE);
       c. Im setup(): dht.begin(); (Initialisiere den Sensor)
       d. Im loop() (falls simulateData = false):
          temp = dht.readTemperature(); 
          humidity = dht.readHumidity(); 
          if (isnan(temp) || isnan(humidity)) {  // Prüfe auf ungültige Daten (z. B. Sensor fehlt)
            temp = 0.0; humidity = 0.0; 
            Serial.println("Fehler: DHT-Sensor nicht erreichbar!");
          }
     
     - BMP280 (Luftdruck):
       a. Installiere die Bibliothek via Library Manager ("Adafruit BMP280").
       b. Definiere oben: #define BMP_SCK 13; #define BMP_SDA 14; Adafruit_BMP280 bmp;
       c. Im setup(): if (!bmp.begin(BMP_SCK, BMP_SDA)) { Serial.println("BMP280 nicht gefunden!"); }
       d. Im loop() (falls simulateData = false):
          pressure = bmp.readPressure() / 100.0;  // BMP liefert Pa, umrechnen zu hPa
          if (pressure == 0) {  // Prüfe auf Fehler (z. B. Sensor fehlt)
            pressure = 0.0; 
            Serial.println("Fehler: BMP280-Sensor nicht erreichbar!");
          }
  
  2. Farbschema ändern:
     - RGB565-Werte berechnen: Nutze ein Online-Tool (z. B. https://rgbtohex.com/rgb565-converter/).
     - Beispiel: Rot (VALUE_RED) zu Grün ändern → 0x07E0 (Grüntone in RGB565).
     - Achte auf Kontrast: Werte (Rot) sollten gut sichtbar sein auf BG_LIGHT (hellgrau).
  
  3. Layout anpassen:
     - displayHeight erhöhen: Um mehr Metriken anzuzeigen (z. B. "CO2: ...").
     - displayLeft/displayTop ändern: UI-Bereich links/oben verschieben (z. B. displayLeft = 30 für mehr Rand).
     - TEXT_SIZE_*_WIDTH_PER_CHAR adjustieren: Falls eine andere Schriftart verwendet wird (z. B. fette Schrift → 16 Pixel).
  
  4. Textanzeige optimieren:
     - Die manuellen Breitenberechnungen (nameWidth, valueWidth) basieren auf der Standard-Schriftart.
     - Bei Verwendung von benutzerdefinierten Schriftarten (z. B. via setFont()) müssen diese Konstanten angepasst werden.
  
  5. Aktualisierungsrate ändern:
     - Die UI aktualisiert sich standardmäßig alle 1 Sekunde (delay(1000)).
     - Für schnelleres Update: delay(500) → 2Hz (achtung: zu hohe Raten können das Display überlasten).
     - Für langsameres Update: delay(2000) → 0.5Hz (spart Strom bei Batteriebetrieb).
  
  6. Fehlerbehandlung hinzufügen:
     - Füge Prüfungen für ungültige Daten (z. B. NaN) hinzu, um den UI-Bereich nicht mit "fehlerhaften" Werten zu füllen.
     - Beispiel: Im loop(), nach Sensor-Read:
        if (simulateData == false) {
          if (isnan(temp)) { temp = -999.9; }  // Markiere Fehler mit -999.9
          if (isnan(humidity)) { humidity = -1; }
          if (isnan(pressure)) { pressure = -1; }
        }
*/

// ----------------------------------------------------------------------------
// --- ENDE DES CODES ---
// ----------------------------------------------------------------------------

/* 
  Notizen zum Projekt:
  - Dieser Code wurde spezifisch für das MCUFRIEND_kbv-TFT-Display entwickelt. Andere Modelle benötigen ggf. PIN- oder ID-Anpassungen.
  - Die Dynamische Berechnung von displayWidth (screenWidth - 2*displayLeft) gewährleistet Kompatibilität mit Displays verschiedener Auflösungen.
  - Die Zeitstempelanzeige zeigt die aktuelle Update-Zeit seit Start, nicht die reale Systemzeit (ohne RTC-Sensor).
  - Für eine reale Wetterstation empfiehlt sich, simulateData = false zu setzen und echte Sensoren zu integrieren.
*/