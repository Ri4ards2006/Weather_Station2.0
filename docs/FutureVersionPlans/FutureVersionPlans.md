
# 🌟 WeatherStation 2.0 - Roadmap & Zukunftsentwicklung  
*"Vom Prototyp zum skalierbaren IoT-Netzwerk – Schritt für Schritt."*  

---

## 📅 Current Status (Stand)  
⚠️ **Aktueller Stand (v1.2):**  
- Master-Arduino (Mega/Uno) läuft stabil, liest BMP280 (Temp/Druck), Ultrasonic (Distanz) und RTC (Zeit) ab.  
- Sensordaten werden über Serial (115200 Baud) als debug-Ausgabe an den Monitor gesendet.  
- Grundstruktur für I2C-Kommunikation (Master ↔ Slaves) ist implementiert.  
- Erste Version TFT-Display-Anzeige (v1.x) existiert, aber nicht vollständig integriert (Aktuell: Serial-Ausgabe primär).  

---

## 🚀 Roadmap Overview  
Die Entwicklung wird in **3 Phasen** unterteilt, basierend auf Prioritäten, Skalierbarkeit und Nutzen für Nutzer:  

| Phase       | Fokus                  | Zeitrahmen (voraussichtlich) | Nutzen für Nutzer                          |
|-------------|------------------------|-------------------------------|-------------------------------------------|
| **Short-Term** | Display-Integration    | 2-4 Wochen                    | Benutzerfriendliche Datenausgabe statt Serial-Logs. |
| **Mid-Term**   | Wireless & ESP32       | 1-2 Monate                     | Remote-Zugriff, IoT-Integration, Stable Netzwerk. |
| **Long-Term**  | Skalierung & Erweiterungen | 3+ Monate                      | Skalierbar auf viele Sensoren/Displays, Cloud-Logs, Automatisierung. |

---

## 💼 Detailed Phasen (Aufgaben & Prioritäten)  

### **Phase 1: Short-Term – TFT Display-Integration (Kernfunktionen)**  
**Ziel:** Daten statt über Serial direkt auf einem TFT-Display visualisieren, um die Wetterstation interaktiv und "standalone" zu machen.  

#### **1.1 Red TFT (1.8"/2.4") – Master-Display**  
- **Aufgabe:** Vollständige Implementierung der TFT-Anzeige (Adafruit_ST7735 oder MCUFRIEND-kbv).  
  - *Details:*  
    - Überarbeite die `renderAll()`-Funktion (aus früheren Codes) oder erstelle eine neue `displayData()`-Funktion, um Sensordaten (Temp, Druck, Distanz, Zeit) auf dem Display zu zeichnen.  
    - Nutze farbige Symbole/Texte für Status:  
      - Temp: Rot bei >40°C, Gelb bei 35-40°C, Grün sonst.  
      - Distanz: Ampel-Design (Rot <10cm, Gelb 10-20cm, Grün ≥20cm).  
      - RTC-Zeit: Weißer Text, korrigierte Positionierung (z.B. unten links).  
    - Prüfe Pin-Konfiguration (TFT_CS, TFT_DC, TFT_RST) und vermeide Konflikte mit anderen Geräten (z.B. SDA/SCL für I2C).  
  - *Warum wichtig:* Ein eigenes Display macht die Station unabhängig vom Computer, perfekt für Einrichtung in Räumen oder Outdoor.  
  - *Status:* Offen (aktueller Code hat Serial-Ausgabe, Display-Code fragmentarisch).  

#### **1.2 Blue TFT (1.8") – Slave-Display (Optional)**  
- **Aufgabe:** Integriere ein zweites TFT-Display als "Slave", gesteuert über I2C vom Master.  
  - *Details:*  
    - Konfiguriere das Blue TFT mit einer eindeutigen I2C-Adresse (z.B. 0x20).  
    - Erstelle eine I2C-Kommunikationsschicht, um vom Master an das Slave-Display Daten zu senden (z.B. "Distanz: 15cm" oder Ampel-Status).  
    - Implementiere ein kleines Dashboard auf dem Slave-Display (nur essenzielle Metriken).  
  - *Warum wichtig:* Skaliert die Anzeige auf mehrere Orte (z.B. Master im Wohnzimmer, Slave im Garten).  
  - *Status:* Offen (Konzept existiert, keine Code-Implementierung).  

---

### **Phase 2: Mid-Term – Wireless Data Transmission & ESP32 Integration**  
**Ziel:** Daten nicht mehr nur lokal anzeigen, sondern wirelessly an externe Geräte (z.B. Smartphone, Cloud) senden.  

#### **2.1 Funkmodule (nRF24L01/RFM69) – Klassische Lösung**  
- **Aufgabe:** Stabile Funkübertragung zwischen Master und Remote-Geräten (z.B. ESP32, Raspberry Pi).  
  - *Details:*  
    - Integriere nRF24L01+-Modul am Master (I2C oder GPIO-Pins).  
    - Teste Sender/Receiver-Logik: Master sendet alle 2s Sensorpakete (Temp, Druck, Distanz, Zeit) an Remote.  
    - Implementiere Fehlerbehandlung (z.B. Retry bei fehlgeschlagener Übertragung, CRC-Prüfung).  
  - *Warum wichtig:* Unabhängig von USB-Kabeln, perfekt für mobile Stationen oder Outdoor-Deployment.  
  - *Status:* Offen (Konzept, keine Hardware/Code-Integration).  

#### **2.2 ESP32 als Bridge (Webinterface & IoT)**  
- **Aufgabe:** ESP32 als Gateway nutzen, um Sensordaten ins Web zu bringen (via Webserver, MQTT oder Cloud).  
  - *Details:*  
    1. **ESP32-Hardware-Setup:**  
       - Verbinde ESP32 mit Arduino-Master über I2C oder Serial (z.B. Serial1 für Datenübertragung).  
       - Sorge für elektrische Kompatibilität (3.3V vs. 5V).  
    2. **Datenempfang am ESP32:**  
       - Schreibe Code, der Sensordaten vom Master empfängt und in ein JSON-Format konvertiert (z.B. `{"temp":25.3,"pressure":1012.8,"distance":22.1,"time":"09:30"}`).  
    3. **Webserver-Implementierung:**  
       - Hoste einen einfachen Webserver (via ArduinoWebServer Library) auf dem ESP32, der die Daten per HTTP anfordern kann.  
       - Schnittstelle: Browser-basiertes Dashboard mit Diagrammen (Temp/Druck über Zeit) und Echtzeit-Updates.  
    4. **MQTT-Integration (Optional):**  
       - Konfiguriere den ESP32 als MQTT-Client, um Daten an Broker (z.B. Mosquitto, Adafruit IO) zu senden.  
  - *Warum wichtig:* Öffnet Möglichkeiten für Remote-Monitoring, Cloud-Analyse und Integration in Smart Home-Systeme.  
  - *Status:* Offen (Konzept, keine Library- oder Code-Arbeiten).  

---

### **Phase 3: Long-Term – Skalierung, Erweiterungen & Automatisierung**  
**Ziel:** Die Station zu einem universellen, erweiterbaren Tool für Umweltmonitoring zu machen.  

#### **3.1 Mehrfach-Sensor-Skalierung**  
- **Aufgabe:** Erweitere den Master, um mehrere I2C-Slaves (bis zu 8) gleichzeitig zu steuern.  
  - *Details:*  
    - Aktualisiere die `slaveAddrs[]`-Liste (aktuell BMP280/RCT) und erlaube dynamische Hinzufügung via Config-Datei.  
    - Optimiere den Poll-Loop (`pollAllSlaves()`-Funktion), um Zeitverlust bei vielen Slaves zu minimieren.  
    - Implementiere Sensor-Typ-Erkennung (z.B. "BMP280" vs. "BH1750" → unterschiedliche Parsing-Rules).  
  - *Warum wichtig:* Nutzer können je nach Bedarf Sensoren hinzufügen (z.B. Luftfeuchtigkeit, CO2, Licht).  

#### **3.2 Datenaufzeichnung & Analyse**  
- **Aufgabe:** Speichere Sensordaten lokal (SD-Karte) oder in der Cloud.  
  - *Details:*  
    - Füge eine SD-Karte-Schnittstelle hinzu (via SPI), um CSV-Dateien zu schreiben (Format: Zeit, Temp, Druck, Distanz).  
    - Integriere Cloud-Services (z.B. Firebase, InfluxDB) über ESP32-MQTT oder HTTP.  
    - Implementiere Zeitstempel-Validierung (vermeide ungültige Logs bei RTC-Fehler).  
  - *Warum wichtig:* Langfristige Trends analysieren (z.B. Tages-Temperaturverlauf, Monats-Druckmuster).  

#### **3.3 Automatische Alarm-Meldungen**  
- **Aufgabe:** Alarme nicht mehr nur lokal anzeigen, sondern per E-Mail/Telegram benachrichtigen.  
  - *Details:*  
    - Integriere einen MQTT-Alarm-Topic (z.B. `weatherstation/alarm`).  
    - Schreibe ein Python-Skript (oder Cloud-Funktion), das Alarme abonniert und per API E-Mails (Gmail) oder Telegram-Nachrichten sendet.  
    - Konfiguriere Nutzerperlen (z.B. Max-Temp, Min-Distanz) via Serial-Befehle oder Webinterface.  
  - *Warum wichtig:* Nutzer werden auch außer Haus über kritische Bedingungen informiert (z.B. Überflutung, zu hohe Hitze).  

#### **3.4 Power-Management & Sleep-Modus**  
- **Aufgabe:** Reduziere Stromverbrauch für portative Anwendungen.  
  - *Details:*  
    - Implementiere Sleep-Modus für Sensoren (z.B. Ultrasonic alle 10s statt 2s messen).  
    - Nutze Deep-Sleep-Modus am ESP32 (falls eingesetzt), um standby-Treibstoff zu minimieren.  
    - Füge eine Lithium-Batterie mit Charging-Circuit hinzu (z.B. TP4056) für autonome Betrieb.  
  - *Warum wichtig:* Makes the station suitable for outdoor or battery-powered setups (z.B. Garten, Ferienhaus).  

---

## 📝 To-Do-Liste (Kanban-Friendly)  
Für eine klare Überwachung der Entwicklung:  

| **Task**                                  | **Status**       | **Priorität** | **Abhängigkeiten**          |  
|-------------------------------------------|------------------|---------------|----------------------------|  
| Red TFT-Display: Daten visualisieren       | Offen            | ⭐⭐⭐ (High)   | -                          |  
| Blue TFT-Slave: I2C-Konfiguration          | Offen            | ⭐⭐ (Medium)  | Red TFT fertiggestellt      |  
| nRF24L01+: Funkübertragungs-Test          | Offen            | ⭐⭐⭐ (High)   | Red TFT fertiggestellt      |  
| ESP32-Bridge: Webserver-Setup              | Offen            | ⭐⭐⭐ (High)   | Funkübertragung stabil      |  
| Datenaufzeichnung (SD-Karte) implementieren| Offen            | ⭐⭐ (Medium)  | Software-Skalierung         |  
| Telegram/E-Mail Alarme einrichten          | Offen            | ⭐ (Low)       | Datenaufzeichnung + MQTT    |  
| Power-Management (Sleep-Modus) testen      | Offen            | ⭐ (Low)       | Alle Phasen 1-2 abgeschlossen |  

---

## 🛠 Tools & Libraries für Umsetzung  
- **Display:** Adafruit_ST7735, MCUFRIEND_kbv, TFT-GFX Libraries.  
- **Wireless:** nRF24L01+ Library, RFM69 Library, ArduinoWebServer (ESP32).  
- **ESP32:** ESP32 Arduino Core, MQTT Client (PubSubClient).  
- **SD-Karte:** SD Library, CSV-Formatter (Eigenentwicklung).  
- **Alarm:** Python (Requests-API), Telegram Bot API, Gmail SMTP.  

---

## 🤝 Collaboration & Contributions  
Diese Roadmap ist Open Source und open to contributions!  
- **Wie du helfen kannst:**  
  - Code für Display-Integration oder Wireless-Module beitragen.  
  - Testberichte zu Hardware-Komponenten (z.B.Blue TFT, nRF24L01+) einreichen.  
  - Verbesserungsvorschläge für die Roadmap oder Prioritäten diskutieren.  
- **Kontakt:**  
  - Issues: Melde Bugs oder Ideen im Repository.  
  - PRs: Schicke deine Code-Änderungen direkt.  

---

## 🌈 Vision  
Am Ende der Long-Term-Phase soll die WeatherStation 2.0 ein **skalierbares, IoT-integriertes System** sein, das:  
- Lokal auf Displays (TFT, LCD, 7-segment) und remotely via Web visualisiert.  
- Sensoren für Temp, Druck, Distanz, Licht, Luftfeuchtigkeit, CO2 usw. unterstützt.  
- Automatische Alarms an Nutzer (per App, E-Mail) sendet.  
- Energieeffizient genug ist, um monatelang von einer Batterie zu laufen.  

---  


