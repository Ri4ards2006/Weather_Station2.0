// MQ-135 Luftqualitätsensor Basismessung
// Anwendung für Arduino Uno/Mega (5V-Betrieb)

// Definition des Sensors-Analog-Eingangs
#define MQ135_PIN A0  // Der MQ-135-Analogausgang wird an den Arduino-Analogeingang A0 angeschlossen

void setup() {
  // Initialisierung der seriellen Kommunikation zur Datenausgabe
  Serial.begin(9600);  
  // Startmeldung an den Seriellen Monitor
  Serial.println("MQ135 Luftqualitaetssensor gestartet...");
  // Aufwärmzeit für den Sensor (erfordert für stabile Messungen)
  delay(2000);          
}

void loop() {
  // Rohwert des Sensors einlesen (0–1023 bei 10-Bit-A/D-Wandler)
  int SensorWertRoh = analogRead(MQ135_PIN);       

  // Umrechnung des Rohwertes in Spannung
  // Der Arduino nutzt eine interne Referenzspannung von 5V
  float SpannungVolt = SensorWertRoh * (5.0 / 1023.0);  

  // Ausgabe der Rohwerte und der Spannung an den Seriellen Monitor
  Serial.print("Rohwert: ");       // Text-Tag für den Rohwert
  Serial.print(SensorWertRoh);     // Rohwert des Sensors ausgeben
  Serial.print(" | Spannung: ");   // Text-Tag für die Spannung
  Serial.print(SpannungVolt, 2);   // Spannung mit 2 Dezimalstellen ausgeben
  Serial.println(" V");            // Einheit (Volt) anhängen

  // Bewertung der Luftqualität anhand des Sensorwertes
  // Diese Kategorien sind subjektiv und können mit Kalibrierungsdaten angepasst werden
  if (SensorWertRoh < 200) {
    Serial.println("Luftqualitaet: Sehr gut");
  } else if (SensorWertRoh < 400) {
    Serial.println("Luftqualitaet: Gut");
  } else if (SensorWertRoh < 700) {
    Serial.println("Luftqualitaet: Mittel");
  } else {
    Serial.println("Luftqualitaet: Schlecht");
  }

  // Trennzeile für die Serielausgabe zur Lesbarkeit
  Serial.println("------------------------------");  

  // Verzögerung zwischen Messungen (1 Sekunde)
  delay(1000);  
}