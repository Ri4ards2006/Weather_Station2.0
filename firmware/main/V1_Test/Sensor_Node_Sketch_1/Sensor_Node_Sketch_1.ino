// === Master: mega_master_display.ino ===

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define TFT_CS   10  // Chip select
#define TFT_DC    9  // Data/Command
#define TFT_RST   8  // Reset

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

const uint8_t slaveAddrs[] = { 0x10 /*sensor1*/, 0x11 /*sensor2*/ }; // add more addresses if you have slaves
const uint8_t SLAVE_COUNT = sizeof(slaveAddrs) / sizeof(slaveAddrs[0]);

unsigned long lastPoll = 0;
const unsigned long POLL_INTERVAL = 1500; // ms between full cycle polls
const unsigned long PER_SLAVE_DELAY = 100; // ms between individual slave queries

struct SlaveData {
  uint8_t id;
  float temp;
  float pressure;
  float altitude;
  float distance;
  int ampel;
  bool valid;
  unsigned long lastSeen;
};

SlaveData slaves[8]; // support up to 8 slaves; match slaveAddrs length

void setup() {
  Serial.begin(115200);
  Wire.begin(); // join I2C as master
  tft.initR(INITR_REDTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);
  tft.setTextWrap(false);

  // initialize slave data
  for (uint8_t i=0;i<8;i++) { slaves[i].valid=false; slaves[i].id=0; }

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(0,0);
  tft.println("WeatherStation 2.0 - Master");
  delay(800);
  tft.fillScreen(ST77XX_BLACK);
}

void loop() {
  unsigned long now = millis();
  if (now - lastPoll >= POLL_INTERVAL) {
    pollAllSlaves();
    lastPoll = now;
    renderAll();
  }
}

void pollAllSlaves() {
  for (uint8_t i=0;i<SLAVE_COUNT;i++) {
    uint8_t addr = slaveAddrs[i];
    requestFromSlave(addr);
    delay(PER_SLAVE_DELAY);
  }
}

void requestFromSlave(uint8_t addr) {
  Wire.requestFrom(addr, (uint8_t)64); // request up to 64 bytes
  String reply = "";
  unsigned long start = millis();
  while (Wire.available()) {
    char c = Wire.read();
    reply += c;
    // safety timeout
    if (millis() - start > 200) break;
  }
  if (reply.length() > 0) {
    Serial.print("Reply from 0x"); Serial.print(addr, HEX); Serial.print(": "); Serial.println(reply);
    parseAndStore(reply);
  } else {
    Serial.print("No reply from 0x");
    Serial.println(addr, HEX);
    // mark slave invalid
    for (uint8_t j=0;j<8;j++) {
      if (slaves[j].id == addr) { slaves[j].valid=false; }
    }
  }
}

void parseAndStore(const String &s) {
  // expected: "ID,TEMP,PRESS,ALT,DIST,AMP"
  // split by comma
  int idx = 0;
  int start = 0;
  String parts[8];
  for (int i=0;i<7;i++) parts[i]="";
  int p = 0;
  for (int i=0;i<s.length();i++) {
    if (s[i] == ',') {
      parts[p++] = s.substring(start, i);
      start = i+1;
    }
  }
  // last token
  parts[p++] = s.substring(start);

  if (p < 6) {
    Serial.println("Parse error: insufficient tokens");
    return;
  }

  // parse ID (hex or decimal)
  String idStr = parts[0];
  uint8_t id = (uint8_t) strtol(idStr.c_str(), NULL, 0); // auto base (0x.. or decimal)

  // find slot or first free
  int slot = -1;
  for (int i=0;i<8;i++) {
    if (slaves[i].valid && slaves[i].id == id) { slot = i; break; }
  }
  if (slot == -1) {
    for (int i=0;i<8;i++) if (!slaves[i].valid) { slot = i; break; }
  }
  if (slot == -1) { Serial.println("No slot free for slave"); return; }

  slaves[slot].id = id;
  slaves[slot].valid = true;
  slaves[slot].lastSeen = millis();

  // parse floats safely
  slaves[slot].temp = parseFloatToken(parts[1]);
  slaves[slot].pressure = parseFloatToken(parts[2]);
  slaves[slot].altitude = parseFloatToken(parts[3]);
  slaves[slot].distance = parseFloatToken(parts[4]);
  slaves[slot].ampel = parts[5].toInt();
}

float parseFloatToken(const String &tok) {
  if (tok.length() == 0) return NAN;
  if (tok == "nan" || tok == "NaN") return NAN;
  return tok.toFloat();
}

void renderAll() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0,0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("WeatherStation 2.0");
  tft.println("---------------------------");

  int y = 24;
  for (int i=0;i<8;i++) {
    if (!slaves[i].valid) continue;
    tft.setCursor(0, y);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_CYAN);
    tft.print("ID 0x"); tft.print(slaves[i].id, HEX);
    tft.setCursor(0, y+10);
    tft.setTextColor(ST77XX_WHITE);
    // temp
    if (!isnan(slaves[i].temp)) {
      tft.print("T: ");
      tft.print(slaves[i].temp, 1);
      tft.print(" C  ");
    } else tft.print("T: N/A  ");

    // pressure
    if (!isnan(slaves[i].pressure)) {
      tft.print("P: ");
      tft.print(slaves[i].pressure, 1);
      tft.print(" hPa ");
    } else tft.print("P: N/A ");

    // distance
    if (!isnan(slaves[i].distance)) {
      tft.print(" D:");
      tft.print(slaves[i].distance, 1);
      tft.print("cm ");
    } else tft.print(" D:N/A ");

    // ampel
    tft.setCursor(0, y+20);
    tft.print("State: ");
    switch (slaves[i].ampel) {
      case 0: tft.print("ROT"); break;
      case 1: tft.print("ROT_GELB"); break;
      case 2: tft.print("GRUEN"); break;
      case 3: tft.print("GELB"); break;
      default: tft.print("N/A"); break;
    }

    y += 36;
    if (y > 140) break; // page limit
  }
}
