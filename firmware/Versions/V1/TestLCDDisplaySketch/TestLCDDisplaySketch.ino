#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

// ---- Farbdefinitionen ----
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

MCUFRIEND_kbv tft;  // TFT-Display-Objekt

void setup() {
  Serial.begin(9600);
  Serial.println("TFT Setup...");

  // Display-ID lesen (für Kompatibilität mit Clones)
  uint16_t tftID = tft.readID();
  Serial.print("TFT ID: 0x"); Serial.println(tftID, HEX);
  if (tftID == 0xD3D3) {  // Fallback für Clone-Displays (z. B. ST7789)
    tftID = 0x9486; 
    Serial.println("Clone Display erkannt – Fallback-ID verwendet.");
  }

  // Display initialisieren (ohne Prüfung, falls begin() keine bool zurückgibt)
  tft.begin(tftID);
  tft.setRotation(1);  // Rotation einstellen (0-3; 1 = 90°)
  
  // Startmeldungen anzeigen (ohne textWidth()-Fehler)
  tft.fillScreen(BLACK);
  displayHelloMessage();  // "Hello Richard!" mit fixer Position
  delay(1500);
  tft.fillScreen(BLUE);
  displayStatusMessage(); // "Arduino Mega LCD OK!" mit fixer Position
  delay(1500);
  tft.fillScreen(RED);
  displayTestMessage();   // "Display Test :)" mit fixer Position
  delay(1500);
}

void displayHelloMessage() {
  tft.setTextSize(3);    // Große Schrift
  tft.setTextColor(GREEN);
  tft.setCursor(30, 50);  // Fixe Position (ursprüngliche Koordinaten)
  tft.println("Hello Richard!");
}

void displayStatusMessage() {
  tft.setTextSize(2);    // Normale Schrift
  tft.setTextColor(WHITE);
  tft.setCursor(40, 100); // Fixe Position (ursprüngliche Koordinaten)
  tft.println("Arduino Mega LCD OK!");
}

void displayTestMessage() {
  tft.setTextSize(2);    // Normale Schrift
  tft.setTextColor(YELLOW);
  tft.setCursor(20, 200); // Fixe Position (ursprüngliche Koordinaten)
  tft.println("Display Test :)");
}

void loop() {
  static uint16_t colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK};
  static int i = 0;  // Farbindex (0-7)

  tft.fillScreen(colors[i]);  // Display mit aktueller Farbe füllen

  // Textfarbe invertieren (Weiß bei dunklem Hintergrund)
  tft.setTextSize(2);
  tft.setTextColor( (colors[i] == BLACK) ? WHITE : BLACK );
  tft.setCursor(40, 100);  // Textposition fixieren (ursprüngliche Koordinaten)

  // Farbe und Index anzeigen (ohne textWidth())
  String currentColorInfo = "Farbe " + String(i) + ": " + getColorName(colors[i]);
  tft.print(currentColorInfo);

  i = (i + 1) % 8;  // Index zyklisch erhöhen
  delay(500);       // Farbwechsel alle 500ms
}

// Hilfsfunktion: Gibt Farb-Name zur Farb-Code zurück
String getColorName(uint16_t colorCode) {
  switch(colorCode) {
    case RED:     return "ROT";
    case GREEN:   return "GRUEN";
    case BLUE:    return "BLAU";
    case YELLOW:  return "GELB";
    case CYAN:    return "CYAN";
    case MAGENTA: return "MAGENT";
    case WHITE:   return "WEIS";
    case BLACK:   return "SCHWARZ";
    default:      return "UNBEKANNT";
  }
}