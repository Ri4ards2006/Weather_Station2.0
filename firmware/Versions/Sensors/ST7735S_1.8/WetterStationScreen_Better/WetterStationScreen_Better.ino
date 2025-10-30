#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS   16
#define TFT_DC   17
#define TFT_RST  18
#define TFT_SDA  19
#define TFT_SCL  20

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_SDA, TFT_SCL, TFT_RST);

// Beispielwerte (hier später Sensorwerte einsetzen)
float temperature = 23.4;
float humidity = 48.9;
float pressure = 1012.6;
String timeNow = "14:37";

void setup() {
  tft.initR(INITR_REDTAB);
  tft.setRotation(2);
  tft.invertDisplay(false);
  tft.fillScreen(ST77XX_BLACK);

  drawHeader();
  drawWeatherData();
}

void loop() {
  // Simulierter Refresh alle 2 Sekunden
  delay(2000);
  updateWeatherData(temperature + random(-5, 5) * 0.1,
                    humidity + random(-3, 3) * 0.1,
                    pressure + random(-2, 2) * 0.1,
                    timeNow);
}

// ---------- Layout-Funktionen ----------

void drawHeader() {
  tft.fillRect(0, 0, 160, 20, ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 6);
  tft.print("Wetterstation v1.0");
}

void drawWeatherData() {
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  
  tft.setCursor(10, 30);
  tft.print("Temp: ");
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(temperature, 1);
  tft.println(" C");

  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 50);
  tft.print("Luftfeuchte: ");
  tft.setTextColor(ST77XX_CYAN);
  tft.print(humidity, 1);
  tft.println(" %");

  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 70);
  tft.print("Druck: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(pressure, 1);
  tft.println(" hPa");

  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 90);
  tft.print("Zeit: ");
  tft.setTextColor(ST77XX_WHITE);
  tft.print(timeNow);
}

void updateWeatherData(float newTemp, float newHum, float newPres, String newTime) {
  // Lösche nur den Datenbereich (nicht den Header)
  tft.fillRect(0, 25, 160, 100, ST77XX_BLACK);

  temperature = newTemp;
  humidity = newHum;
  pressure = newPres;
  timeNow = newTime;

  drawWeatherData();
}
