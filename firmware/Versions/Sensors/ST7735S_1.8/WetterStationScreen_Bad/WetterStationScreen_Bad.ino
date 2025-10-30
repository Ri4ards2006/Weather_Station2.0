#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS   16
#define TFT_DC   17
#define TFT_RST  18
#define TFT_SDA  19
#define TFT_SCL  20

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_SDA, TFT_SCL, TFT_RST);

// Beispielwerte
float temperature = 23.4;
float humidity = 48.9;
float pressure = 1012.6;
String timeNow = "14:37";

// Layout-Parameter
int scrW, scrH;
int headerH;
int footerH;
int contentY, contentH;

// Eigene Farbe definieren (Dunkelgrau)
uint16_t DARKGREY = 0;  // wird in setup() berechnet

void setup() {
  Serial.begin(115200);
  tft.initR(INITR_REDTAB);
  tft.setRotation(2);
  tft.invertDisplay(false);

  scrW = tft.width();
  scrH = tft.height();

  headerH = max(16, scrH / 10);
  footerH = max(18, scrH / 8);
  contentY = headerH;
  contentH = scrH - headerH - footerH;

  DARKGREY = tft.color565(40, 40, 40); // RGB -> dunkles Grau

  tft.fillScreen(ST77XX_BLACK);
  drawHeader();
  drawFooter("Status: OK");
  drawWeatherData();
}

void loop() {
  delay(2000);
  float newTemp = temperature + (random(-5,6) * 0.1);
  float newHum  = humidity + (random(-3,4) * 0.1);
  float newPres = pressure + (random(-2,3) * 0.1);
  updateWeatherData(newTemp, newHum, newPres, timeNow);
}

// ---------- Zeichenfunktionen ----------

void drawHeader() {
  tft.fillRect(0, 0, scrW, headerH, ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(1);
  tft.setCursor(6, (headerH - 8) / 2);
  tft.print("Wetterstation v1.0");
}

void drawFooter(const char* status) {
  tft.fillRect(0, scrH - footerH, scrW, footerH, DARKGREY);
  tft.setTextColor(ST77XX_WHITE, DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(6, scrH - footerH + 4);
  tft.print(status);

  tft.drawRect(scrW - 34, scrH - footerH + 4, 28, footerH - 8, ST77XX_WHITE);
  tft.setCursor(scrW - 32, scrH - footerH + 6);
  tft.print("LoRa");
}

void drawWeatherData() {
  tft.fillRect(0, contentY, scrW, contentH, ST77XX_BLACK);

  int rows = 4;
  int rowH = contentH / rows;

  // Temperatur
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_YELLOW);
  int tx = 6;
  int ty = contentY + 2;
  tft.setCursor(tx, ty);
  tft.print("Temp:");
  String tStr = String(temperature, 1) + " C";
  tft.setCursor(scrW - (tStr.length() * 6 * 2) - 6, ty);
  tft.print(tStr);

  // Luftfeuchte
  tft.setTextColor(ST77XX_CYAN);
  ty = contentY + rowH + 2;
  tft.setCursor(tx, ty);
  tft.print("Hum:");
  String hStr = String(humidity, 1) + " %";
  tft.setCursor(scrW - (hStr.length() * 6 * 2) - 6, ty);
  tft.print(hStr);

  // Luftdruck
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_MAGENTA);
  ty = contentY + rowH * 2 + 6;
  tft.setCursor(tx, ty);
  tft.print("Druck:");
  String pStr = String(pressure, 1) + " hPa";
  tft.setCursor(scrW - (pStr.length() * 6) - 6, ty);
  tft.print(pStr);

  // Uhrzeit
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN);
  ty = contentY + rowH * 3 + 2;
  tft.setCursor(tx, ty);
  tft.print("Zeit:");
  tft.setCursor(scrW - (timeNow.length() * 6 * 2) - 6, ty);
  tft.print(timeNow);
}

void updateWeatherData(float newTemp, float newHum, float newPres, String newTime) {
  tft.fillRect(0, contentY, scrW, contentH, ST77XX_BLACK);

  temperature = newTemp;
  humidity = newHum;
  pressure = newPres;
  timeNow = newTime;

  drawWeatherData();
  drawFooter("Status: OK");
}
