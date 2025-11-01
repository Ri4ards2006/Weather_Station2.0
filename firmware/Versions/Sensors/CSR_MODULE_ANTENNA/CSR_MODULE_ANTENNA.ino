void setup() {
  Serial.begin(115200);    // PC-Monitor
  Serial1.begin(9600);     // Funkmodul an RX1/TX1
  Serial.println("Starte Test...");
}

void loop() {
  // Alles was vom Modul kommt, an PC weitergeben
  if (Serial1.available()) {
    char c = Serial1.read();
    Serial.write(c);
  }

  // Alles was du im Serial-Monitor tippst, ans Modul senden
  if (Serial.available()) {
    char c = Serial.read();
    Serial1.write(c);
  }
}
