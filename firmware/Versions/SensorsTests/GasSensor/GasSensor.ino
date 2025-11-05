// MQ-135 Luftqualitätssensor + Buzzer-Alarm
// Für Arduino Mega (5V)

#define MQ135_PIN A15    // MQ-135 Analogausgang
#define BUZZER_PIN 50    // Buzzer Digital-Pin

void setup() {
  Serial.begin(9600);
  Serial.println("MQ135 Luftqualitaetssensor gestartet...");

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  delay(2000);  // Aufwärmzeit
}

void loop() {
  int SensorWertRoh = analogRead(MQ135_PIN);
  float SpannungVolt = SensorWertRoh * (5.0 / 1023.0);

  Serial.print("Rohwert: ");
  Serial.print(SensorWertRoh);
  Serial.print(" | Spannung: ");
  Serial.print(SpannungVolt, 2);
  Serial.println(" V");

  // Bewertung der Luftqualität
  if (SensorWertRoh < 200) {
    Serial.println("Luftqualitaet: Sehr gut");
    digitalWrite(BUZZER_PIN, LOW);
  } else if (SensorWertRoh < 400) {
    Serial.println("Luftqualitaet: Gut");
    digitalWrite(BUZZER_PIN, LOW);
  } else if (SensorWertRoh < 700) {
    Serial.println("Luftqualitaet: Mittel");
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    Serial.println("Luftqualitaet: Schlecht!");
    digitalWrite(BUZZER_PIN, HIGH);  // Alarm
  }

  Serial.println("------------------------------");
  delay(1000);
}

