#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

// --- Sensoren ---
Adafruit_BMP280 bmp;
RTC_DS3231 rtc;

// --- Ultraschall ---
#define TRIG_PIN 43
#define ECHO_PIN 42

// --- Ampel + Buzzer ---
#define LED_GREEN  51
#define LED_YELLOW 50
#define LED_RED    49
#define BUZZER     53

// --- Button ---
#define BUTTON_PIN 26

// --- MQ-135 ---
#define MQ135_PIN A15

// --- TFT Display ---
#define TFT_CS    53
#define TFT_RS     9
#define TFT_RST    8
#define BACKLIGHT 13
MCUFRIEND_kbv tft;

// --- UI Farben & Größen ---
#define BLACK       0x0000
#define WHITE       0xFFFF
#define TITLE_BLUE  0x001F
#define NAME_GRAY   0x4444
#define VALUE_RED   0xF800
#define BG_LIGHT    0xD3D3
#define BORDER_WHITE 0xFFFF

#define TEXT_SIZE_3_WIDTH_PER_CHAR 14
#define TEXT_SIZE_3_HEIGHT 24
#define TEXT_SIZE_2_WIDTH_PER_CHAR 12
#define TEXT_SIZE_2_HEIGHT 20

int screenWidth, screenHeight;
int displayLeft = 20;
int displayTop = 30;
int displayWidth;
int displayHeight = 200;
String currentTime = "00:00";
unsigned long lastTimeUpdate = 0;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 2000;

// --- Prototypen ---
void updateWeatherUI(float temp, float humidity, float pressure, float distance, int mqValue);
void updateWeatherUI();
void drawDisplayArea();
void drawMetric(int y, String name, float value, String unit, int decimals);
void drawMetric(int y, String name, int value, String unit);
void drawSeparator(int y);
float measureDistanceCM();
void triggerAlarm(float distance, float temperature, int mqValue);
bool parseAndSetRTC(const String &s);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);

  if (!bmp.begin(0x76)) Serial.println(F("❌ BMP280 nicht gefunden!"));
  if (!rtc.begin()) Serial.println(F("❌ RTC nicht gefunden!"));
  
  tft.begin(tft.readID());
  tft.setRotation(1);
  screenWidth = tft.width();
  screenHeight = tft.height();
  displayWidth = screenWidth - 2 * displayLeft;
  
  updateWeatherUI();
}

void loop() {
  // Button-Test
  if (digitalRead(BUTTON_PIN) == LOW) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    tone(BUZZER, 1000, 500);
    delay(500);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  }

  // RTC über Serial setzen
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0 && parseAndSetRTC(line))
      Serial.println(F("✅ RTC gesetzt"));
  }

  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();
    
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F;
    float distance = measureDistanceCM();
    int mqValue = analogRead(MQ135_PIN);

    // Update TFT
    updateWeatherUI(temperature, 0.0, pressure, distance, mqValue);
    triggerAlarm(distance, temperature, mqValue);
  }
}

// --- Ultraschall ---
float measureDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  return (duration == 0) ? -1 : (duration * 0.0343) / 2.0;
}

// --- Ampel & Alarm ---
void triggerAlarm(float distance, float temperature, int mqValue) {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(BUZZER, LOW);

  if (mqValue >= 700 || distance >=0 && distance <10 || temperature>40) {
    digitalWrite(LED_RED,HIGH);
    digitalWrite(LED_GREEN,LOW);
    tone(BUZZER,1000,500);
  } else if (mqValue>=400 || (distance>=0 && distance<20)) {
    digitalWrite(LED_YELLOW,HIGH);
    digitalWrite(LED_GREEN,LOW);
  }
}

// --- TFT UI ---
void updateWeatherUI(float temp, float humidity, float pressure, float distance, int mqValue) {
  tft.fillScreen(BLACK);
  drawDisplayArea();
  
  tft.setTextSize(3);
  tft.setTextColor(TITLE_BLUE);
  String title = "Wetterstation";
  int titleWidth = title.length() * TEXT_SIZE_3_WIDTH_PER_CHAR;
  tft.setCursor(displayLeft + (displayWidth - titleWidth)/2, displayTop + 20);
  tft.print(title);

  int y = displayTop + 20 + TEXT_SIZE_3_HEIGHT + 15;
  drawMetric(y, "Temp:", temp, "C", 1); y+=TEXT_SIZE_2_HEIGHT+5;
  drawMetric(y, "Druck:", pressure, "hPa", 0); y+=TEXT_SIZE_2_HEIGHT+5;
  drawMetric(y, "Dist:", distance, "cm", 1); y+=TEXT_SIZE_2_HEIGHT+5;
  drawMetric(y, "Luftqualität:", mqValue, "", 0); y+=TEXT_SIZE_2_HEIGHT+5;

  String timeStr = "Aktualisiert: " + currentTime;
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  int tw = timeStr.length()*TEXT_SIZE_2_WIDTH_PER_CHAR;
  tft.setCursor(displayLeft + (displayWidth-tw)/2, displayTop + displayHeight-30);
  tft.print(timeStr);
}

void drawDisplayArea() {
  tft.fillRoundRect(displayLeft, displayTop, displayWidth, displayHeight, 15, BG_LIGHT);
  tft.drawRect(displayLeft-3, displayTop-3, displayWidth+6, displayHeight+6, BORDER_WHITE);
  tft.drawRect(displayLeft, displayTop, displayWidth, displayHeight, NAME_GRAY);
}

void drawMetric(int y, String name, float value, String unit, int decimals) {
  tft.setTextColor(NAME_GRAY);
  tft.setCursor(displayLeft+2, y);
  tft.print(name);
  tft.setTextColor(VALUE_RED);
  tft.setCursor(displayLeft+100, y); // einfache feste Position, falls Platz reicht
  tft.print(String(value,decimals)+unit);
}

void drawMetric(int y, String name, int value, String unit) {
  tft.setTextColor(NAME_GRAY);
  tft.setCursor(displayLeft+2, y);
  tft.print(name);
  tft.setTextColor(VALUE_RED);
  tft.setCursor(displayLeft+140, y);
  tft.print(value);
} 

void updateWeatherUI() {
  // Hier kommt alles hin, was dein Wetter-UI-Update macht
  Serial.println("Weather UI updated!"); // Platzhalter
}

bool parseAndSetRTC(const String &s) {
  if (!s.startsWith("SET:")) return false;
  String body = s.substring(4);
  if (body.length() < 19) return false;

  int Y = body.substring(0,4).toInt();
  int m = body.substring(5,7).toInt();
  int d = body.substring(8,10).toInt();
  int H = body.substring(11,13).toInt();
  int M = body.substring(14,16).toInt();
  int S = body.substring(17,19).toInt();
  rtc.adjust(DateTime(Y,m,d,H,M,S));
  return true;
}
