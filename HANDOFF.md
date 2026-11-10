# HANDOFF — claude/firmware-v2 (2026-11-10)

## Erledigt
- `AUDIT.md`: Bestandsaufnahme aus dem Code (Hardware, Pins, Toolchain, funktioniert/fragil, Doku-Drift).
- V1.4 per `pio ci` für `megaatmega2560` testgebaut (Scratch-Kopie, Repo unverändert): Build OK.

## Bewusst NICHT gemacht (auf Anweisung: „don't change code“)
- Tasks 1–7 (PlatformIO-Migration, Firmware-Split, Deep-Sleep, MQTT/OTA, docker-compose, CI, README/PORTFOLIO).
  Grund: Repo ist Arduino Mega 2560 (AVR), kein ESP32; Wi-Fi/Deep-Sleep/OTA nicht ohne Hardware-Wechsel
  und erfundene Pinbelegung umsetzbar.

## Befunde mit Handlungsbedarf
1. Klartext-Zugangsdaten in lokaler `.git/config` (Gitea-Push-URL) → Passwort rotieren.
2. Pin 53 doppelt (Buzzer + TFT_CS) in V1.4.
3. STM32 / x86 / LED-Uhr-Code ist **nicht** in diesem Repo → richtiges Repo benennen.

## Nächster sinnvoller Schritt
Entscheidung Zielplattform (Mega behalten + PlatformIO/CI, Mega + ESP32-Bridge, oder ESP32-Port),
danach Tasks 1–7 auf diesem Branch.
