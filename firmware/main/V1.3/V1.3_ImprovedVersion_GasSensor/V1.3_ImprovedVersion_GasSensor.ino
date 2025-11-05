#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <RTClib.h>

// --- Sensoren definieren ---
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

// --- Test Button ---
#define BUTTON_PIN 26

// --- MQ-135 Luftqualität ---
#define MQ135_PIN A15  // Analog-Pin für Luftqualität

// --- Timing ---
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 2000;  // ms

// --- Funktionsprototypen ---
float measureDistanceCM();
void printSensorData();
void triggerAlarm(float distance, float temperature, int mqValue);
bool parseAndSetRTC(const String &s);

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  Wire.begin();

  Serial.println(F("🌦 Wetterstation v2.0 startet..."));

  // BMP
  if (!bmp.begin(0x76)) Serial.println(F("❌ BMP280 nicht gefunden!"));
  else Serial.println(F("✅ BMP280 verbunden"));

  // RTC
  if (!rtc.begin()) Serial.println(F("❌ RTC-Modul nicht gefunden! Prüfe SDA/SCL und VCC/GND"));
  else Serial.println(F("✅ RTC verbunden"));

  if (rtc.lostPower()) Serial.println(F("⚠️ RTC meldet Stromverlust (lostPower()). Zeit ggf. nicht gesetzt."));

  // Ultraschall-Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Ampel + Buzzer
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Anfangszustand
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(BUZZER, LOW);

  Serial.println(F("Setup abgeschlossen. Drücke Button zum Testen des Alarms."));
}

// Hilfsfunktion zum RTC-Setzen
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
  if (Y < 2000 || m<1 || m>12 || d<1 || d>31 || H<0 || H>23 || M<0 || M>59 || S<0 || S>59) return false;

  rtc.adjust(DateTime(Y,m,d,H,M,S));
  return true;
}

void loop() {
  // Button-Test
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("🔔 Button gedrückt: Test-Alarm!");
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    tone(BUZZER, 1000, 500);
    delay(500);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  }

  // RTC-Set via Serial
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      if (parseAndSetRTC(line)) Serial.println(F("✅ RTC gesetzt via SET-Befehl."));
      else Serial.println(F("❌ Ungültiges SET-Format. SET:YYYY-MM-DD HH:MM:SS"));
    }
  }

  // Periodische Sensoren
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();
    printSensorData();
  }
}

// ---------- Sensorfunktionen ----------
float measureDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  if (duration == 0) return -1;
  return (duration * 0.0343) / 2.0;
}

void printSensorData() {
  float temperature = NAN;
  float pressure = NAN;
  int mqValue = analogRead(MQ135_PIN);

  if (bmp.begin(0x76)) {
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure()/100.0F;
  }

  float distance = measureDistanceCM();
  DateTime now = rtc.now();
  char timeString[20];
  snprintf(timeString,sizeof(timeString),"%02d:%02d:%02d",now.hour(),now.minute(),now.second());

  Serial.println(F("----------- Messdaten -----------"));
  Serial.print(F("🌡 Temperatur: ")); Serial.print(temperature,1); Serial.println(F(" °C"));
  Serial.print(F("🌬 Luftdruck:  ")); Serial.print(pressure,1); Serial.println(F(" hPa"));
  Serial.print(F("📏 Entfernung: ")); Serial.print(distance,1); Serial.println(F(" cm"));
  Serial.print(F("🌫️ Luftqualität: ")); Serial.println(mqValue);
  Serial.print(F("⏰ Zeit:       ")); Serial.println(timeString);
  Serial.println(F("--------------------------------\n"));

  Serial1.print(F("TEMP:")); Serial1.println(temperature);
  Serial1.print(F("PRESS:")); Serial1.println(pressure);
  Serial1.print(F("DIST:")); Serial1.println(distance);
  Serial1.print(F("MQ135:")); Serial1.println(mqValue);

  // Ampel + Alarm
  triggerAlarm(distance, temperature, mqValue);
}

// ---------- Ampel + Alarmlogik ----------
void triggerAlarm(float distance, float temperature, int mqValue) {
  // Standard
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(BUZZER, LOW);

  // MQ135-Alarm-Schwellen
  bool badAir = (mqValue >= 700); // >700 = schlechte Luft
  bool warnAir = (mqValue >= 400 && mqValue < 700);

  // Distanz-Alarm
  bool closeObj = (distance >= 0 && distance < 10);
  bool warnDist = (distance >=0 && distance < 20);

  // Temperatur-Alarm
  bool hotTemp = (!isnan(temperature) && temperature > 40);

  // Kritisch (rot + buzzer)
  if (badAir || closeObj || hotTemp) {
    digitalWrite(LED_RED,HIGH);
    digitalWrite(LED_GREEN,LOW);
    tone(BUZZER,1000,500);
  }
  // Warnung gelb
  else if (warnAir || warnDist) {
    digitalWrite(LED_YELLOW,HIGH);
    digitalWrite(LED_GREEN,LOW);
  }
  // Alles OK
  else digitalWrite(LED_GREEN,HIGH);
}
