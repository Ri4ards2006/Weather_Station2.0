#define TRIG_PIN 9
#define ECHO_PIN 10

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  long duration;
  float distance;

  // Trigger-Puls auslösen (10 µs HIGH)
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Echo-Zeit messen
  duration = pulseIn(ECHO_PIN, HIGH);

  // Entfernung berechnen (Schallgeschwindigkeit ~343 m/s)
  distance = duration * 0.0343 / 2;

  // Ausgabe
  Serial.print("Entfernung: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(500);
}
