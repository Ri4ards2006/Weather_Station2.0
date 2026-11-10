# AUDIT — Weather_Station2.0 (Stand 2026-11-10)

Bestandsaufnahme **nur aus dem Code im Repo** (Branch `main` @ `151dc0c`, plus Remote-Branches
`coding`, `docs`, `hardware`). Kein Code wurde geändert. Was nicht aus dem Code belegbar ist, steht als TODO.

## 0. Scope-Klarstellung

| Erwartet laut Auftrag | Befund im Repo |
|---|---|
| ESP32-Telemetrie-Node | **Nicht vorhanden.** Kein ESP32/ESP8266-Code, kein Wi-Fi, kein MQTT, kein OTA. ESP32 taucht nur als Plan in `docs/FutureVersionPlans/` auf. |
| STM32-Projekt | **Nicht vorhanden** (weder `main` noch Remote-Branches). |
| x86-Projekt | **Nicht vorhanden.** |
| LED-Uhr-Code | **Nicht in diesem Repo.** (Die einzige „Uhr“ ist das DS3231-RTC-Modul der Wetterstation.) |
| AVR-Code | **Vorhanden** — das gesamte Repo ist AVR: Arduino Mega 2560 (ATmega2560). |

→ Falls STM32 / x86 / LED-Uhr gemeint waren: die liegen in einem anderen Repository. TODO: Pfad klären.

## 1. Hardware (aus dem Code)

**MCU:** Arduino Mega 2560 (ATmega2560, 16 MHz, 5 V). Belege: Pins 42–53, `A15`, `Serial1`,
Kommentare „für Arduino Mega 2560“ (`Traffic_Light_Code_TestISR.ino`, `GasSensor.ino`, `CSR_MODULE_ANTENNA.ino`),
Timer1-Register `TCCR1A/B`, `OCR1A` mit 16 MHz/1024.

### Aktueller Stand V1.4 (`firmware/main/V1.4/…/V1.4_Screen_Output_SM.ino`)

| Funktion | Bauteil | Bus | Pin(s) laut Code |
|---|---|---|---|
| Temperatur, Luftdruck | BMP280 | I2C, Adresse `0x76` | SDA/SCL (Mega: 20/21, Hardware-I2C) |
| Uhrzeit | DS3231 (RTClib `RTC_DS3231`) | I2C (Default-Adresse der Lib) | SDA/SCL |
| Distanz | HC-SR04 | GPIO, `pulseIn` Timeout 25 ms | TRIG 43, ECHO 42 |
| Luftqualität (Rohwert 0–1023) | MQ-135 | ADC | `A15` |
| Ampel | LEDs | GPIO | Grün 51, Gelb 50, Rot 49 |
| Buzzer | Buzzer (`tone()` 1 kHz) | GPIO | **53** |
| Testtaster | Taster gegen GND | GPIO, `INPUT_PULLUP` | 26 |
| Display | TFT-Shield (MCUFRIEND_kbv, ID per `readID()`) | 8-Bit-Parallel-Shield | `TFT_CS` **53**, `TFT_RS` 9, `TFT_RST` 8, Backlight 13 |
| Debug / RTC setzen | USB-Serial | UART0 115200 Bd | `SET:YYYY-MM-DD HH:MM:SS` |

Feuchte: wird als `0.0` an die UI übergeben, **kein Feuchtesensor im Code**.

### Ältere Stände / Testsketches (abweichende Belegungen)

| Sketch | Abweichung |
|---|---|
| V1.0 (`firmware/main/v1.0/…`) | I2C-**Master** fragt Slaves `0x10`, `0x11` ab (Slave-Firmware **nicht im Repo**); ST7735 auf CS 10 / DC 9 / RST 8 |
| V1.1 | HC-SR04 TRIG 52 / ECHO 53; Rohdaten an `Serial1` 9600 Bd (`TEMP:`, `PRESS:`, `DIST:`) |
| V1.2 / V1.3 | Pins wie V1.4 ohne TFT; V1.3 ergänzt MQ-135 + `MQ135:` auf `Serial1` |
| `Alarm_System_TL_Buzzer` | LEDs 50/48/46, Buzzer 44, Taster 26 |
| `GasSensor` | MQ-135 `A15`, Buzzer 50 |
| `HumiditySensor-HC-SR04` | trotz Name: HC-SR04 auf TRIG 9 / ECHO 10 (kein Feuchtesensor) |
| `ST7735S_1.8` (+ `_Bad`) | ST7735 auf CS 16 / DC 17 / RST 18 / „SDA“ 19 / „SCL“ 20 |
| `WetterStationScreen_Better` | ST7735 auf CS 44 / DC 47 / RST 46 / „SDA“ 20 / „SCL“ 21 |
| `CSR_MODULE_ANTENNA` | Funk-/BT-Modul an `Serial1` 9600 Bd (Pins 18/19), Modultyp: TODO |
| `Traffic_Light_Code_TestISR` | Ampel per Timer1-ISR auf 8/9/10 |

## 2. Toolchain

| Punkt | Befund |
|---|---|
| Build-System | **Arduino IDE** (je Sketch ein Ordner mit gleichnamiger `.ino`). Kein `platformio.ini`, kein ESP-IDF, kein Makefile/CMake. |
| Bibliotheken (nicht versioniert) | Adafruit BMP280 (+ Adafruit Unified Sensor), RTClib, Adafruit GFX, MCUFRIEND_kbv, Adafruit ST7735, Wire, SPI |
| CI | keine |
| Tests | keine |

### Verifizierter Build (ohne Repo-Änderung, Scratch-Kopie)

V1.4 (`V1.4_Screen_Output_SM.ino`) wurde mit PlatformIO 6.2.0 erfolgreich gebaut:

```bash
pio ci main.cpp -b megaatmega2560 \
  -O 'lib_deps=adafruit/Adafruit BMP280 Library@2.6.8, adafruit/RTClib@2.1.4, adafruit/Adafruit GFX Library@1.11.11, prenticedavid/MCUFRIEND_kbv@3.1.0-Beta'
# (main.cpp = .ino + "#include <Arduino.h>" in Zeile 1)
# RAM 742/8192 B (9.1 %), Flash 36804/253952 B (14.5 %)
```

Arduino-IDE-Weg: Board „Arduino Mega or Mega 2560“, obige Bibliotheken über den Library Manager, Sketch-Ordner öffnen, Upload.
Nicht verifiziert: andere Sketches, Lauf auf echter Hardware.

## 3. Was funktioniert (laut Code + Build)

- V1.4 kompiliert für Mega 2560 mit deutlichem RAM/Flash-Spielraum.
- Sensorabfrage alle 2 s nicht-blockierend über `millis()`.
- Ampel-/Buzzer-Alarm mit Schwellen: Rot bei MQ ≥ 700 ∨ Distanz < 10 cm ∨ Temp > 40 °C; Gelb bei MQ ≥ 400 ∨ Distanz < 20 cm.
- RTC über Serial setzbar (`SET:…`), HC-SR04 liefert `-1` bei Timeout.
- Keine Secrets im getrackten Code oder in der Git-History (Suche mit `rg` nach password/ssid/token/api_key/secret: nur Parser-Kommentare).

## 4. Was fragil ist

| # | Problem | Ort | Auswirkung |
|---|---|---|---|
| 1 | **Pin 53 doppelt belegt**: `BUZZER` und `TFT_CS` | V1.4 | `pinMode(BUZZER)` / `tone()` stören das Display-CS (Pin 53 ist zudem SPI-SS des Mega) |
| 2 | Sensoren gelesen und `updateWeatherUI()` aufgerufen **bevor** `bmp.begin()`, `tft.begin()` | V1.4 `setup()` | erstes Bild/Werte undefiniert, TFT-Zugriff vor Init |
| 3 | Sensorfehler nur einmal geloggt, danach werden Werte ungeprüft weiterverwendet (`NaN`, RTC-Zeit beliebig) | V1.1–V1.4 | Falschalarme, Müll auf Display |
| 4 | Blockierend: `delay(500)` im Taster-Pfad, `pulseIn` bis 25 ms | V1.4 `loop()` | Jitter, kein Debounce |
| 5 | `tft.fillScreen()` alle 2 s für komplettes Redraw | V1.4 | sichtbares Flackern |
| 6 | Viel `String`-Konkatenation | V1.4 UI | Heap-Fragmentierung auf 8 KB SRAM |
| 7 | Kein Watchdog | alle | Hänger (z. B. I2C) ohne Recovery |
| 8 | MQ-135 nur Rohwert, keine Kalibrierung/Aufwärmzeit (außer `GasSensor.ino`: 2 s) | V1.3/V1.4 | Schwellen 400/700 ohne physikalische Einheit |
| 9 | ST7735-Testsketches legen „SDA/SCL“ auf 20/21 = Mega-I2C-Pins | `WetterStationScreen_Better` | Kollision mit BMP280/DS3231 am selben Bus |
| 10 | Bibliotheksversionen nicht gepinnt, kein reproduzierbarer Build | Repo | „works on my machine“ |
| 11 | Viele Duplikate (`_Commented` vs. Original, zwei V1.4-Varianten, `V1.First_Settup_Sketch_C` doppelt) | `firmware/` | unklar, welcher Stand gilt |

## 5. Doku-Drift

- `README.md` beschreibt **v1.2** mit Pins TRIG 52 / ECHO 53, LEDs 46/48/50, Buzzer 44 — der V1.2-Code nutzt 43/42, 51/50/49, 53.
- README verweist auf `LICENSE` und `docs/images/wiring_master_v1.2.png` — **beide fehlen**.
- README-Tabellen teils zerschnitten („Distance 40°C“).
- `test.txt` im Root: Reste von Push-Tests.

## 6. Repo / Infrastruktur

- Branches: `main`, `coding` (= älterer `main`-Stand), `docs` und `hardware` (identisch, ältere Struktur `Node1/`, `Sensors/`).
- `hardware/` und `web/` enthalten nur `.gitkeep`: **keine Schaltpläne, kein KiCad, keine Web-UI**.
- ⚠️ Lokale `.git/config`: die Push-URL des zweiten Remotes (Gitea im LAN) enthält **Zugangsdaten im Klartext**.
  Nicht getrackt, aber in Shell-History/Logs sichtbar. Empfehlung: Passwort rotieren, Credential-Helper/Token nutzen.
- `gitleaks` ist lokal nicht installiert → kein gitleaks-Scan gelaufen (TODO).

## 7. Offene TODOs (nicht aus Code ableitbar)

- TODO: Wo liegen STM32-, x86- und LED-Uhr-Projekte?
- TODO: Slave-Firmware für I2C `0x10`/`0x11` (V1.0)?
- TODO: Genaues TFT-Shield-Modell / Controller-ID, Funkmodultyp an `Serial1`.
- TODO: Stromversorgung, Stromaufnahme, Gehäuse — keine Angaben im Repo.
- TODO: Soll v2 auf ESP32 portiert werden (Pinbelegung dann neu festzulegen)?
