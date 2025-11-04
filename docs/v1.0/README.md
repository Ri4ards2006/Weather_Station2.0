# 🌦 WeatherStation 2.0 - v1 Master Display

## Übersicht
Dies ist die **erste funktionale Master-Version** der WeatherStation 2.0.  
Der Master sammelt über **I2C** Daten von bis zu 8 Sklaven-Sensoren und zeigt sie auf einem **1.8"/2.4" ST7735 TFT-Display** an.

---

## Zweck
- I2C-Sklaven abfragen
- Empfangene Daten parsen
- Darstellung der Daten auf TFT-Display
- Ampelzustände der Sensoren visualisieren

---

## Hardware
- **TFT-Display (ST7735, 1.8"/2.4")**
  - CS = 10
  - DC = 9
  - RST = 8
- **I2C-Slaves**
  - Adresse 0x10: Sensor 1
  - Adresse 0x11: Sensor 2
  - Max. 8 Slaves unterstützt
- **Optionaler Debug**: USB-Serial Monitor

---

## Bibliotheken
- [Wire](https://www.arduino.cc/en/reference/wire) (I2C-Kommunikation)
- [Adafruit_GFX](https://github.com/adafruit/Adafruit-GFX-Library) (Grafikgrundlage)
- [Adafruit_ST7735](https://github.com/adafruit/Adafruit-ST7735-Library) (TFT-Display)

---

## Software & Datenfluss
1. Master fragt jeden Sklaven über I2C ab (max. 64 Bytes pro Anfrage)
2. Empfangene Daten im Format:

- ID: Sklavenadresse
- TEMP: Temperatur (°C)
- PRESS: Luftdruck (hPa)
- ALT: Höhe (m)
- DIST: Entfernung/Ultraschall (cm)
- AMP: Ampelzustand (0=Rot, 1=Rot-Gelb, 2=Grün, 3=Gelb)
3. Validierte Daten werden in `SlaveData` Array gespeichert
4. Alle 1,5 Sekunden werden die Daten auf dem Display aktualisiert

---

## Anzeige auf TFT
- Titel: `WeatherStation 2.0`
- Jeder Sklave:
- ID (Cyan)
- Temperatur (weiß)
- Druck (weiß)
- Distanz (weiß)
- Ampel-Zustand (weiß, textuell)
- Stoppt bei Y>140, um Display-Überlauf zu vermeiden

---

## Poll-Intervalle
- **Vollständige Poll-Runde**: 1500 ms (`POLL_INTERVAL`)
- **Pause zwischen Einzel-Slave-Abfragen**: 100 ms (`PER_SLAVE_DELAY`)

---

## Fehlerbehandlung
- Keine Antwort vom Sklaven → `valid=false`
- Ungültige Daten → Temperatur/Druck/Distanz = `NAN`, Anzeige `N/A`
- Maximale Slots: 8 (neue Slaves nur, wenn Slot frei)

---

## Limitierungen
- Altitude-Werte gespeichert, aber noch nicht auf Display angezeigt
- Debug über Serial (115200 Baud)
- Nur erste 8 Slaves können gleichzeitig verwaltet werden

---

## Nächste Schritte / To-Do
- Ampel-Visualisierung in Farbe
- Alarm/Output-Funktionen integrieren
- Erweiterung auf weitere Sensoren
- Dokumentation der Verkabelung und Pinbelegung

---

## Hinweis
Dieses Projekt ist **Work-in-Progress** und dient als Basis für zukünftige Versionen der WeatherStation 2.0.

