// Ampel mit ISR-Steuerung (Timer1) – für Arduino Mega 2560

enum AmpelZustand { ROT, ROT_GELB, GRUEN, GELB };
volatile AmpelZustand aktuellerZustand = ROT;

// Pins
const int LED_ROT = 8;
const int LED_GELB = 9;
const int LED_GRUEN = 10;

// Timer-Intervall (in Sekunden)
const unsigned long INTERVALL = 3;  // alle 3 Sekunden wechseln

void setup() {
  pinMode(LED_ROT, OUTPUT);
  pinMode(LED_GELB, OUTPUT);
  pinMode(LED_GRUEN, OUTPUT);

  // Anfangszustand
  zeigeZustand();

  // Timer1 Setup (CTC-Modus)
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  // 1 Hz bei 16 MHz/1024 -> 15625
  OCR1A = 15625 * INTERVALL;
  TCCR1B |= (1 << WGM12);   // CTC-Modus
  TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024
  TIMSK1 |= (1 << OCIE1A);  // Interrupt aktivieren
  interrupts();
}

ISR(TIMER1_COMPA_vect) {
  // Nächsten Zustand wählen
  switch (aktuellerZustand) {
    case ROT:        aktuellerZustand = ROT_GELB; break;
    case ROT_GELB:   aktuellerZustand = GRUEN;    break;
    case GRUEN:      aktuellerZustand = GELB;     break;
    case GELB:       aktuellerZustand = ROT;      break;
  }
  zeigeZustand();
}

void zeigeZustand() {
  switch (aktuellerZustand) {
    case ROT:
      digitalWrite(LED_ROT, HIGH);
      digitalWrite(LED_GELB, LOW);
      digitalWrite(LED_GRUEN, LOW);
      break;

    case ROT_GELB:
      digitalWrite(LED_ROT, HIGH);
      digitalWrite(LED_GELB, HIGH);
      digitalWrite(LED_GRUEN, LOW);
      break;

    case GRUEN:
      digitalWrite(LED_ROT, LOW);
      digitalWrite(LED_GELB, LOW);
      digitalWrite(LED_GRUEN, HIGH);
      break;

    case GELB:
      digitalWrite(LED_ROT, LOW);
      digitalWrite(LED_GELB, HIGH);
      digitalWrite(LED_GRUEN, LOW);
      break;
  }
}

void loop() {
  // leer – alles läuft über Interrupt
}
