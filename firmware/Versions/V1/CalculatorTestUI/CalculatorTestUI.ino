#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

// ---- Farbdefinitionen (RGB565-Codes) ----
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define LIGHTGRAY 0xD3D3 // Helliggrau für Buttons

// ---- TFT-Pindefinitionen (passen Sie an Ihre Hardware an!) ----
#define TFT_CS   53   // Chip Select (CS) Pin (SPI-Bus)
#define TFT_RS    9   // Register Select (RS) Pin
#define TFT_RST   8   // Reset (RST) Pin
#define BACKLIGHT 13 // Backlight-Pin (3.3V)

// ---- TFT-Display-Objekt mit expliziten Pins ----
MCUFRIEND_kbv tft(&SPI, TFT_CS, TFT_RS, TFT_RST); 

// ---- UI-Konstanten (passt an Display-Größe an) ----
#define SCREEN_WIDTH  tft.width()   // Display-Breite (typisch 240)
#define SCREEN_HEIGHT tft.height()  // Display-Höhe (typisch 320)
#define DISPLAY_TOP   30            // Display-Bereich: Oben
#define DISPLAY_LEFT  20            // Display-Bereich: Links
#define DISPLAY_WIDTH (SCREEN_WIDTH - 40) // Display-Bereich: Breite (links/rechts Rand)
#define DISPLAY_HEIGHT 60           // Display-Bereich: Höhe (oben/unter Rand)
#define BUTTON_SIZE   60            // Button-Größe (Breite/Höhe)
#define BUTTON_SPACING 10          // Abstand zwischen Buttons

// Schriftgroesse-Schätzungen (anpassen, falls nötig)
#define TEXT_SIZE_3_WIDTH_PER_CHAR 14  // Breite pro Zeichen bei TextSize(3) (Pixel)
#define TEXT_SIZE_3_HEIGHT 24         // Höhen-Schriftgröße 3 (Pixel)
#define TEXT_SIZE_2_WIDTH_PER_CHAR 12  // Breite pro Zeichen bei TextSize(2) (Pixel)
#define TEXT_SIZE_2_HEIGHT 20         // Höhen-Schriftgröße 2 (Pixel)

void setup() {
  Serial.begin(9600);
  Serial.println("Calculator UI Initialisierung...");

  // Backlight einstellen (wichtig für Sichtbarkeit!)
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH); // Backlight einschalten (manche Displays brauchen LOW!)
  Serial.println("Backlight aktiviert.");

  // SPI-Bus initialisieren
  SPI.begin();
  Serial.println("SPI-Bus initialisiert.");

  // Display-ID lesen (Kompatibilität)
  uint16_t tftID = tft.readID();
  Serial.print("TFT ID: 0x"); Serial.println(tftID, HEX);
  if (tftID == 0xD3D3) { // Clone-Fallback
    tftID = 0x9486; 
    Serial.println("Fallback-ID verwendet.");
  }

  // Display initialisieren
  tft.begin(tftID);
  tft.setRotation(1); // Rotation (1 = 90° Landscape)
  Serial.println("TFT initialisiert.");

  // UI-Test:Calculator UI anzeigen
  drawCalculatorUI();
  delay(5000); // 5 Sekunden ansehen
}

void loop() {
  // Simulierte Benutzereingabe (0-9, dann Reset)
  static int inputCounter = 0;
  inputCounter = (inputCounter + 1) % 10; 
  String currentInput = "Eingabe: " + String(inputCounter);

  // UI aktualisieren
  drawCalculatorUI();
  updateDisplayArea(currentInput); // Text im Display-Bereich anzeigen
  delay(1000); // Jede Sekunde aktualisieren
}

// UI für den Taschenrechner zeichnen
void drawCalculatorUI() {
  tft.fillScreen(BLACK); // Hintergrund schwarz

  // Display-Bereich (Weiß mit schwarzem Rand)
  tft.drawRect(DISPLAY_LEFT, DISPLAY_TOP, DISPLAY_WIDTH, DISPLAY_HEIGHT, WHITE);
  tft.fillRect(DISPLAY_LEFT + 2, DISPLAY_TOP + 2, DISPLAY_WIDTH - 4, DISPLAY_HEIGHT - 4, WHITE); // Innenweiss

  // Buttons für Zahlen und Operationen zeichnen
  drawNumberButtons();   // Zahlen 1-9, 0
  drawOperationButtons(); // Operationen (+, -, =)
}

// Zahlen-Buttons (1-9, 0) zeichnen
void drawNumberButtons() {
  int buttonRow = DISPLAY_TOP + DISPLAY_HEIGHT + BUTTON_SPACING; // Buttons starten unter dem Display
  int buttonCol = DISPLAY_LEFT;

  // Zahlen 1-9 (3x3 Grid)
  for (int num = 1; num <= 9; num++) {
    tft.fillRoundRect(buttonCol, buttonRow, BUTTON_SIZE, BUTTON_SIZE, 5, LIGHTGRAY); // Runder Button

    // Text "num" mittig positionieren (Schriftgröße 3)
    tft.setTextSize(3);
    tft.setTextColor(BLACK); // Textschwarz auf hellgrauem Button

    // X-Position: Button-Mitte - Textbreite/2
    int textX = buttonCol + (BUTTON_SIZE / 2) - (TEXT_SIZE_3_WIDTH_PER_CHAR * 1 / 2); 
    // Y-Position: Button-Mitte - TextHöhe/2
    int textY = buttonRow + (BUTTON_SIZE / 2) - (TEXT_SIZE_3_HEIGHT / 2); 

    tft.setCursor(textX, textY);
    tft.print(num);

    // Nächster Button (neue Reihe nach 3 Buttons)
    buttonCol += BUTTON_SIZE + BUTTON_SPACING;
    if (num % 3 == 0) {
      buttonCol = DISPLAY_LEFT;
      buttonRow += BUTTON_SIZE + BUTTON_SPACING;
    }
  }

  // Zahl 0 (großer Button,覆蓋 2 Spalten)
  buttonCol = DISPLAY_LEFT;
  buttonRow += BUTTON_SIZE + BUTTON_SPACING; // Neue Reihe
  tft.fillRoundRect(buttonCol, buttonRow, BUTTON_SIZE * 2 + BUTTON_SPACING, BUTTON_SIZE, 5, LIGHTGRAY);

  // Text "0" mittig positionieren (Schriftgröße 3)
  tft.setTextSize(3);
  tft.setTextColor(BLACK);
  int zeroTextWidth = TEXT_SIZE_3_WIDTH_PER_CHAR * 1; // "0" ist 1 Zeichen
  int zeroTextHeight = TEXT_SIZE_3_HEIGHT;
  int zeroX = buttonCol + (BUTTON_SIZE * 2 + BUTTON_SPACING) / 2 - (zeroTextWidth / 2);
  int zeroY = buttonRow + (BUTTON_SIZE / 2) - (zeroTextHeight / 2);
  tft.setCursor(zeroX, zeroY);
  tft.print("0");
}

// Operationen-Buttons (+, -, =) zeichnen
void drawOperationButtons() {
  // Rechte Spalte für Operationen
  int opsCol = DISPLAY_LEFT + (BUTTON_SIZE * 3 + BUTTON_SPACING * 2); 
  int buttonRow = DISPLAY_TOP + DISPLAY_HEIGHT + BUTTON_SPACING; // Erste Reihe

  // Button "+"
  tft.fillRoundRect(opsCol, buttonRow, BUTTON_SIZE, BUTTON_SIZE, 5, LIGHTGRAY);
  tft.setTextSize(3);
  tft.setTextColor(BLACK);
  int plusTextWidth = TEXT_SIZE_3_WIDTH_PER_CHAR * 1; // "+" ist 1 Zeichen
  int plusTextHeight = TEXT_SIZE_3_HEIGHT;
  int plusX = opsCol + (BUTTON_SIZE / 2) - (plusTextWidth / 2);
  int plusY = buttonRow + (BUTTON_SIZE / 2) - (plusTextHeight / 2);
  tft.setCursor(plusX, plusY);
  tft.print("+");

  // Button "-" (nächste Reihe)
  buttonRow += BUTTON_SIZE + BUTTON_SPACING;
  tft.fillRoundRect(opsCol, buttonRow, BUTTON_SIZE, BUTTON_SIZE, 5, LIGHTGRAY);
  tft.setTextSize(3);
  tft.setTextColor(BLACK);
  int minusTextWidth = TEXT_SIZE_3_WIDTH_PER_CHAR * 1;
  int minusTextHeight = TEXT_SIZE_3_HEIGHT;
  int minusX = opsCol + (BUTTON_SIZE / 2) - (minusTextWidth / 2);
  int minusY = buttonRow + (BUTTON_SIZE / 2) - (minusTextHeight / 2);
  tft.setCursor(minusX, minusY);
  tft.print("-");

  // Button "=" (höhere Button,覆蓋 2 Reihen)
  buttonRow += BUTTON_SIZE + BUTTON_SPACING;
  tft.fillRoundRect(opsCol, buttonRow, BUTTON_SIZE, BUTTON_SIZE * 2 + BUTTON_SPACING, 5, LIGHTGRAY);
  tft.setTextSize(3);
  tft.setTextColor(BLACK);
  int equalsTextWidth = TEXT_SIZE_3_WIDTH_PER_CHAR * 1;
  int equalsTextHeight = TEXT_SIZE_3_HEIGHT;
  int equalsX = opsCol + (BUTTON_SIZE / 2) - (equalsTextWidth / 2);
  int equalsY = buttonRow + (BUTTON_SIZE * 2 + BUTTON_SPACING) / 2 - (equalsTextHeight / 2);
  tft.setCursor(equalsX, equalsY);
  tft.print("=");
}

// Display-Bereich aktualisieren (zeigt aktuellen Eingabewert)
void updateDisplayArea(String text) {
  // Hintergrund des Display-Bereichs löschen (Weiß)
  tft.fillRect(DISPLAY_LEFT + 2, DISPLAY_TOP + 2, DISPLAY_WIDTH - 4, DISPLAY_HEIGHT - 4, WHITE);
  
  // Text schwarze Schrift, Größe 3
  tft.setTextSize(3);
  tft.setTextColor(BLACK);

  // Textbreite schätzen (Länge * Pixel pro Zeichen)
  int textWidth = text.length() * TEXT_SIZE_3_WIDTH_PER_CHAR;
  // Textmittelpunkt (Display-Mitte - Textbreite/2)
  int x = DISPLAY_LEFT + (DISPLAY_WIDTH / 2) - (textWidth / 2) + 2; 

  // Texthöhe (fester Wert) und Mittelpunkt
  int textHeight = TEXT_SIZE_3_HEIGHT;
  int y = DISPLAY_TOP + (DISPLAY_HEIGHT / 2) - (textHeight / 2) + 2; 

  tft.setCursor(x, y);
  tft.print(text);
}