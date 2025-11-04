#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS   16
#define TFT_DC   17
#define TFT_RST  18
#define TFT_SDA  19
#define TFT_SCL  20

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_SDA, TFT_SCL, TFT_RST);

void setup() {
  tft.initR(INITR_REDTAB);   // Oder BLACKTAB → testen
  tft.setRotation(2);        // 180° drehen
  tft.invertDisplay(false);  // Farben korrigieren

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("ST7735S OK!");
}

void loop() {
  // Farbwechsel-Test
  tft.fillScreen(ST77XX_BLUE);
  delay(500);
  tft.fillScreen(ST77XX_RED);
  delay(500);
  tft.fillScreen(ST77XX_GREEN);
  delay(500);
}
