// --- Ampel + Buzzer ---
#define LED_GREEN  50
#define LED_YELLOW 48
#define LED_RED    46
#define BUZZER     44

// --- Button zum Testen ---
#define BUTTON_PIN 26

void setup() {
  Serial.begin(115200);

  // Pins als OUTPUT
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Button als INPUT_PULLUP (kein externes Pullup nötig)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // LEDs & Buzzer aus
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(BUZZER, LOW);

  Serial.println("🔔 Alarm-Test bereit. Drücke den Button zum Auslösen.");
}

void loop() {
  // Button prüfen (LOW = gedrückt)
  if (digitalRead(BUTTON_PIN) == LOW) {
    triggerAlarm();
    delay(500); // kurze Entprellung + Pause, damit der Button nicht mehrfach auslöst
  }
}

// --- Funktion für Alarm ---
void triggerAlarm() {
  Serial.println("⚠️ Alarm aktiviert!");

  // Rot + Piepton
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  // Buzzer für 500 ms piepen
  tone(BUZZER, 1000, 500); // 1 kHz, 500 ms

  delay(500);

  // Alarm zurücksetzen (alles aus)
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH); // grün als "alles okay" Anzeige
}
