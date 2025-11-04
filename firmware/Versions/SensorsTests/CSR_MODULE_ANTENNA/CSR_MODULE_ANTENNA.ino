void setup() {
  // Serielle Schnittstelle zum PC öffnen (über USB)
  // -> Damit du über den Serial Monitor kommunizieren kannst.
  Serial.begin(115200);

  // Zweite serielle Schnittstelle für das Funkmodul aktivieren
  // Der Arduino Mega hat mehrere Hardware-Serials:
  // Serial0 = USB (PC)
  // Serial1 = Pins 18 (TX1) und 19 (RX1)
  Serial1.begin(9600);  // Typische Baudrate für Bluetooth-Module (kann variieren)

  // Startmeldung an den PC schicken
  Serial.println("Starte Test...");
}

void loop() {
  // 🔹 Alles, was vom Funkmodul über Serial1 kommt,
  // wird direkt an den PC weitergeleitet.
  // So kannst du im Serial Monitor sehen, ob das Modul etwas sendet.
  if (Serial1.available()) {
    char c = Serial1.read();
    Serial.write(c);   // Zeichen an PC weitergeben
  }

  // 🔹 Alles, was du im Serial Monitor tippst,
  // wird an das Funkmodul geschickt.
  // Damit kannst du z. B. AT-Befehle senden.
  if (Serial.available()) {
    char c = Serial.read();
    Serial1.write(c);  // Zeichen an Funkmodul senden
  }
}
