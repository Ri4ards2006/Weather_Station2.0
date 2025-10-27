#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <string.h> // Für strlen()

// --- Farbdefinitionen (RGB565) ---
#define BLACK       0x0000
#define WHITE       0xFFFF
#define TITLE_BLUE  0x001F    // Titel-Blau
#define NAME_GRAY   0x4444    // Metrik-Namen-Grau
#define VALUE_RED   0xF800    // Werte-Rot
#define BG_LIGHT    0xD3D3    // Display-Hintergrund (hellgrau)
#define BORDER_WHITE 0xFFFF   // Außerrand-Weiß

// --- Textgröße-Konstanten (Pixel) ---
#define TEXT_SIZE_3_WIDTH_PER_CHAR 14  // Breite pro Zeichen bei TextSize(3)
#define TEXT_SIZE_3_HEIGHT          24 // Höhen-Schriftgröße 3
#define TEXT_SIZE_2_WIDTH_PER_CHAR  12  // Breite pro Zeichen bei TextSize(2)
#define TEXT_SIZE_2_HEIGHT          20 // Höhen-Schriftgröße 2

// --- Display-Pins ---
#define TFT_CS    53   // SPI Chip Select
#define TFT_RS     9   // Register Select
#define TFT_RST    8   // Reset
#define BACKLIGHT 13  // Hintergrundbeleuchtung

// --- Globale Variablen ---
MCUFRIEND_kbv tft(&SPI, TFT_CS, TFT_RS, TFT_RST); 
int screenWidth, screenHeight;   // Dynamische Display-Größe
int displayLeft = 20;               // Linker Rand (Pixel)
int displayTop = 30;                // Oberer Rand (Pixel)
int displayWidth;                    // Display-Breite (berechnet)
int displayHeight = 200;            // Display-Höhe
String currentTime = "00:00";       // Zeitstempel
unsigned long lastTimeUpdate = 0;   // Letzte Zeitaktualisierung (ms)

// --- Function Prototypes (erforderlich für Compiler) ---
void updateWeatherUI(float temp, float humidity, float pressure);
void updateWeatherUI(); // Ohne Argumente (Standardwerte)
void drawDisplayArea();
void drawMetric(int y, String name, float value, String unit, int decimals = 0); // Mit decimals-Param
void drawSeparator(int y);

void setup() {
  Serial.begin(9600);
  Serial.println("Wetterstation UI Initialisierung...");
  
  // Backlight einstellen
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);  // Anpassen, falls LOW benötigt
  
  // SPI-Bus initialisieren
  SPI.begin();
  Serial.println("SPI initialisiert.");
  
  // Display-ID erkennen und initialisieren
  uint16_t tftID = tft.readID();
  Serial.print("TFT ID: 0x"); Serial.println(tftID, HEX);
  if (tftID == 0xD3D3) {  // Fallback für Clones
    tftID = 0x9486; 
    Serial.println("Fallback-ID verwendet.");
  }
  tft.begin(tftID);
  tft.setRotation(1);  // 90° Landscape (weite horizontal)
  Serial.println("TFT initialisiert.");

  // Display-Größe dynamisch ermitteln
  screenWidth = tft.width();
  screenHeight = tft.height();
  displayWidth = screenWidth - 2 * displayLeft;  // Rand links/rechts berücksichtigen

  // Start-UI anzeigen
  updateWeatherUI(); 
  delay(5000); // 5 Sekunden Anzeige
}

void loop() {
  // Simulierte Wetterdaten (ersetze mit Sensoren, falls gewünscht)
  bool simulateData = true;  // Auf false setzen für echte Sensoren
  float temp, humidity, pressure;
  
  if (simulateData) {
    temp = random(15, 35) + (float)random(0, 10)/10.0; // 15.0-34.9°C
    humidity = random(30, 90);                         // 30-89%
    pressure = random(980, 1030);                      // 980-1029 hPa
  } else {
    // Beispiel-Sensor-Einbindung (entferne Kommentare):
    // temp = readDHTTemp(); humidity = readDHTHumidity(); pressure = readBMPPressure();
    temp = 22.5; humidity = 55; pressure = 1013;  // Feste Testwerte
  }

  // UI aktualisieren
  updateWeatherUI(temp, humidity, pressure); 
  
  // Zeitstempel aktualisieren (alle 1 Sek.)
  unsigned long currentMillis = millis();
  if (currentMillis - lastTimeUpdate >= 1000) {
    lastTimeUpdate = currentMillis;
    int totalSec = currentMillis / 1000;
    int min = totalSec / 60;
    int sec = totalSec % 60;
    currentTime = (min < 10 ? "0" : "") + String(min) + ":" + 
                  (sec < 10 ? "0" : "") + String(sec);
  }

  delay(1000); // Jede Sekunde aktualisieren
}

// --- UI aktualisieren (zentrierte Metriken) ---
void updateWeatherUI(float temp, float humidity, float pressure) {
  tft.fillScreen(BLACK); // Hintergrund komplett schwarz
  drawDisplayArea();     // Display-Box mit Rahmen

  // Titel "Wetterstation" (zentriert, blau)
  tft.setTextSize(3);
  tft.setTextColor(TITLE_BLUE);
  String title = "Wetterstation";
  int titleWidth = title.length() * TEXT_SIZE_3_WIDTH_PER_CHAR; 
  int titleX = displayLeft + (displayWidth / 2) - (titleWidth / 2); 
  int titleY = displayTop + 20;
  tft.setCursor(titleX, titleY);
  tft.print(title);

  // Metriken (Textgröße 2, Grau für Namen, Rot für Werte)
  tft.setTextSize(2);
  int metricY = titleY + TEXT_SIZE_3_HEIGHT + 15; // Abstand unter Titel

  // Temperatur (1 Dezimalstelle)
  drawMetric(metricY, "Temperatur: ", temp, "°C", 1);
  metricY += TEXT_SIZE_2_HEIGHT + 10; // Nächste Zeile

  // Trennlinie
  drawSeparator(metricY);
  metricY += 5; // Abstand nach Linie

  // Feuchtigkeit (keine Dezimalen)
  drawMetric(metricY, "Feuchtigkeit: ", humidity, "%", 0);
  metricY += TEXT_SIZE_2_HEIGHT + 10;

  // Trennlinie
  drawSeparator(metricY);
  metricY += 5;

  // Luftdruck (keine Dezimalen)
  drawMetric(metricY, "Luftdruck: ", pressure, " hPa", 0);
  metricY += TEXT_SIZE_2_HEIGHT + 10;

  // Zeitstempel (unten, weiß)
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  String timeStr = "Aktualisiert: " + currentTime;
  int timeWidth = timeStr.length() * TEXT_SIZE_2_WIDTH_PER_CHAR;
  int timeX = displayLeft + (displayWidth / 2) - (timeWidth / 2); 
  int timeY = displayTop + displayHeight - 30;
  tft.setCursor(timeX, timeY);
  tft.print(timeStr);
}

// --- UI ohne Daten (Standardwerte) ---
void updateWeatherUI() {
  updateWeatherUI(0.0, 0.0, 0.0); 
}

// --- Display-Box mit Rahmen zeichnen ---
void drawDisplayArea() {
  // Runder Hintergrund (hellgrau)
  tft.fillRoundRect(displayLeft, displayTop, displayWidth, displayHeight, 15, BG_LIGHT);
  // Dicker außerrand (Weiß)
  tft.drawRect(displayLeft - 3, displayTop - 3, displayWidth + 6, displayHeight + 6, BORDER_WHITE);
  // Innerer Rahmen (Grau)
  tft.drawRect(displayLeft, displayTop, displayWidth, displayHeight, NAME_GRAY);
}

// --- Metrik (Name + Wert) formatieren ---
void drawMetric(int y, String name, float value, String unit, int decimals) {
  // Name linksbündig platzieren (Grau)
  tft.setTextColor(NAME_GRAY);
  tft.setCursor(displayLeft + 2, y); 
  tft.print(name);

  // Wert formatieren (Rot)
  String valueStr = String(value, decimals) + unit;
  tft.setTextColor(VALUE_RED);

  // Textbreiten berechnen (verwende Konstanten)
  int nameWidth = name.length() * TEXT_SIZE_2_WIDTH_PER_CHAR;
  int valueWidth = valueStr.length() * TEXT_SIZE_2_WIDTH_PER_CHAR;

  // Wenn Name+Wert zu breit: Wert rechtsbündig
  if (nameWidth + valueWidth > (displayWidth - 40)) { 
    int valueX = displayLeft + displayWidth - valueWidth - 20; // Rechter Rand mit Abstand
    tft.setCursor(valueX, y);
    tft.print(valueStr);
    return;
  }

  // Normaler Fall: Wert nach Name (mit Abstand)
  int valueX = displayLeft + 2 + nameWidth + 20; // Name-Breite + Abstand
  tft.setCursor(valueX, y);
  tft.print(valueStr);
}

// --- Horizontale Trennlinie (Grau) ---
void drawSeparator(int y) {
  tft.setTextColor(NAME_GRAY);
  tft.drawLine(displayLeft + 10, y, displayLeft + displayWidth - 10, y, NAME_GRAY);
}