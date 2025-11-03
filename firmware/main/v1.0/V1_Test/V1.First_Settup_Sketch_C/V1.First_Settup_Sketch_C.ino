// ==============================================
// WeatherStation 2.0 Master - mega_master_display.ino
// ==============================================
// Purpose: Manage an I2C Master node that queries I2C slave sensors for data and displays received information on a TFT screen.
//          Handles sensor communication, error checking, and clear data visualization.
// ==============================================
// Libraries: Required for I2C communication, TFT display control, and SPI (if needed for TFT)
// ==============================================
#include <Wire.h>        // I2C Communication Library (Master/Slave protocol). Enables functions like Wire.requestFrom() and Wire.read()
#include <SPI.h>         // Serial Peripheral Interface (SPI) Library. Used for SPI communication (required by some TFT drivers)
#include <Adafruit_GFX.h>// Adafruit Graphics Library. Abstract base for display operations (e.g., text rendering, drawing)
#include <Adafruit_ST7735.h>// Adafruit ST7735 TFT Library. Implements GFX functions for 1.8"/2.4" ST7735 displays

// ==============================================
// TFT Display Pin Definitions (Master Node)
// ==============================================
// Pins to control SPI communication with the TFT display
#define TFT_CS   10  // Chip Select (CS) Pin. Activates the display (LOW = selected, HIGH = deselected)
#define TFT_DC    9  // Data/Command (DC) Pin. Determines if incoming data is a command (LOW) or pixel data (HIGH)
#define TFT_RST   8  // Reset (RST) Pin. Initializes the display (LOW = reset, HIGH = normal operation)

// ==============================================
// TFT Display Object Initialization
// ==============================================
// Create an Adafruit_ST7735 display object using the defined pins
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ==============================================
// Slave Sensor Addresses and Count
// ==============================================
// Array of I2C addresses for connected slave sensors. Each address corresponds to a unique sensor.
// Example: 0x10 (Sensor1), 0x11 (Sensor2). Supports up to 8 slaves (matches the size of the 'slaves' array)
const uint8_t slaveAddrs[] = { 0x10 /*Sensor1 (e.g., temperature/pressure sensor)*/, 0x11 /*Sensor2 (e.g., distance/ampel sensor)*/ }; 
// Number of slaves: Automatically calculated based on the size of 'slaveAddrs' to avoid manual counting
const uint8_t SLAVE_COUNT = sizeof(slaveAddrs) / sizeof(slaveAddrs[0]); 

// ==============================================
// Time Management Variables
// ==============================================
// 'lastPoll': Timestamp (ms) of the last full sensor poll cycle. Used to track when the next poll is needed
unsigned long lastPoll = 0; 
// 'POLL_INTERVAL': Interval (ms) between full poll cycles. 1500ms = 1.5 seconds. Controls how often all slaves are queried
const unsigned long POLL_INTERVAL = 1500; 
// 'PER_SLAVE_DELAY': Pause (ms) between querying individual slaves. 100ms prevents I2C bus overload
const unsigned long PER_SLAVE_DELAY = 100; 

// ==============================================
// Slave Data Structure
// ==============================================
// Structure to store data from a single I2C slave sensor
struct SlaveData {
  uint8_t id;          // I2C address of the slave (unique identifier)
  float temp;          // Recorded temperature (°C). Float for precision (e.g., 23.5°C)
  float pressure;      // Recorded air pressure (hPa). Float for precision (e.g., 1013.2hPa)
  float altitude;      // Recorded altitude (m). Stored but not displayed (future extension candidate)
  float distance;      // Recorded distance (cm). Float to store values like 5.5cm
  int ampel;           // Traffic light state indicator (0-3). Integer for discrete states (0=Red, 1=Red-Yellow, 2=Green, 3=Yellow)
  bool valid;          // Validity flag. True = data is current and valid; False = data is invalid (e.g., no response, parsing error)
  unsigned long lastSeen; // Timestamp (ms) of the last valid data reception. Used for error tracking (e.g., timeouts)
};

// ==============================================
// Slave Data Array
// ==============================================
// Array to store data for up to 8 slave sensors. Index 0-7
// Initialized in setup() as invalid (valid=false). Holds current sensor data
SlaveData slaves[8]; 

// ==============================================
// Setup Function (Runs once at startup)
// ==============================================
void setup() {
  // Initialize serial monitor for debugging (115200 baud for fast logging)
  Serial.begin(115200); 
  
  // Initialize I2C Master (Wire library). Sets Arduino as I2C Master
  Wire.begin(); 
  
  // Initialize TFT Display
  tft.initR(INITR_REDTAB); // Initialize with REDTAB color palette (red background, white text, etc.)
  tft.fillScreen(ST77XX_BLACK); // Clear display by filling with black
  tft.setRotation(1); // Set display rotation to 90° (portrait mode)
  tft.setTextWrap(false); // Disable text wrapping (prevents automatic line breaks for long text)
  
  // Initialize all slave entries as invalid (valid=false) and ID=0
  for (uint8_t i=0; i<8; i++) { 
    slaves[i].valid = false;   // Mark slot as invalid initially
    slaves[i].id = 0;          // Clear ID field
  }
  
  // Show welcome message on display
  tft.setTextColor(ST77XX_WHITE); // Set text color to white (high contrast on black background)
  tft.setTextSize(1);         // Set text size to standard (1x)
  tft.setCursor(0,0);         // Position cursor at top-left corner (X=0, Y=0)
  tft.println("WeatherStation 2.0 - Master"); // Print welcome title
  delay(800);                  // Wait 800ms to let user read the message
  
  // Clear display for data rendering
  tft.fillScreen(ST77XX_BLACK);
}

// ==============================================
// Main Loop (Runs infinitely)
// ==============================================
void loop() {
  // Get current time since startup (ms). Used to check poll intervals
  unsigned long now = millis(); 
  
  // Check if it's time for a new full poll cycle (if time since last poll ≥ POLL_INTERVAL)
  if (now - lastPoll >= POLL_INTERVAL) {
    pollAllSlaves();    // Poll all slaves to collect new data
    lastPoll = now;    // Update last poll timestamp to current time
    renderAll();        // Render collected data to the display
  }
}

// ==============================================
// pollAllSlaves() Function: Poll all slaves sequentially
// ==============================================
void pollAllSlaves() {
  // Iterate over each defined slave address in 'slaveAddrs'
  for (uint8_t i=0; i<SLAVE_COUNT; i++) { 
    uint8_t addr = slaveAddrs[i]; // Get current slave address (e.g., 0x10)
    requestFromSlave(addr);       // Query this slave for data
    delay(PER_SLAVE_DELAY);        // Short delay (100ms) between slave polls to prevent bus overload
  }
}

// ==============================================
// requestFromSlave() Function: Query a single slave and process its response
// ==============================================
void requestFromSlave(uint8_t addr) {
  // Request up to 64 bytes of data from the slave with address 'addr' (I2C protocol max)
  Wire.requestFrom(addr, (uint8_t)64); 
  
  // Initialize variables to collect response and track timeout
  String reply = "";          // Stores received data from the slave
  unsigned long start = millis(); // Timestamp to monitor response timeout (200ms max)
  
  // Collect incoming data while I2C bus has data available and within timeout
  while (Wire.available()) { 
    char c = Wire.read();     // Read one byte (char) from I2C bus
    reply += c;               // Append byte to response string
    
    // Safety timeout: Stop collecting if 200ms have passed since request started
    if (millis() - start > 200) break; 
  }
  
  // Process response if data was received
  if (reply.length() > 0) {
    Serial.print("Reply from 0x"); Serial.print(addr, HEX); Serial.print(": "); Serial.println(reply); // Debug log: Print received data
    parseAndStore(reply);      // Parse and store data in the 'slaves' array
  } else {
    // No response received from the slave
    Serial.print("No reply from 0x"); Serial.println(addr, HEX); // Debug log: Alert no response
    
    // Mark the slave as invalid if it previously had valid data
    for (uint8_t j=0; j<8; j++) { 
      if (slaves[j].valid && slaves[j].id == addr) { // Check if slot j is valid and matches the current address
        slaves[j].valid = false;  // Mark as invalid (data no longer current)
        break;                    // Exit loop once updated
      }
    }
  }
}

// ==============================================
// parseAndStore() Function: Parse received string data and store in SlaveData structure
// ==============================================
void parseAndStore(const String &s) {
  // Expected format: "ID,TEMP,PRESS,ALT,DIST,AMP"
  // Example: "0x10,23.5,1013.2,150.0,5.5,2" → ID=0x10, Temp=23.5°C, Pressure=1013.2hPa, etc.
  // Goals: Validate format, extract values, and store in the 'slaves' array

  int idx = 0;                // Unused index (potential leftover from earlier versions; may be removed)
  int start = 0;              // Starting index for substring extraction
  String parts[8];            // Array to split the received string into components (max 8 parts for safety)
  
  // Initialize parts array with empty strings to avoid undefined values
  for (int i=0; i<7; i++) parts[i] = ""; 
  
  int p = 0;                  // Counter for number of extracted parts (components)
  
  // Split the input string by commas (',') to separate data fields
  for (int i=0; i<s.length(); i++) {
    if (s[i] == ',') {        // Detect comma to split parts
      parts[p++] = s.substring(start, i); // Extract substring from 'start' to 'i' and store in parts[p]
      start = i+1;            // Update start index to next character after comma
    }
  }
  // Extract the last part (after the final comma)
  parts[p++] = s.substring(start); 
  
  // Validate that at least 6 parts were extracted (ID, TEMP, PRESS, ALT, DIST, AMP required)
  if (p < 6) { 
    Serial.println("Parse error: insufficient tokens"); // Debug log: Not enough data fields
    return; // Exit function if invalid format
  }
  
  // Parse slave ID from the first part (supports hex/decimal strings)
  String idStr = parts[0]; 
  uint8_t id = (uint8_t) strtol(idStr.c_str(), NULL, 0); // Convert string to integer (auto-detect base: hex, decimal, etc.)
  
  // Find an appropriate slot (index) in the 'slaves' array to store data:
  // 1. Try to reuse existing valid slot with matching ID (if exists)
  // 2. If none, use first free (invalid) slot
  // 3. If no free slots, log error and exit
  int slot = -1;  // Initialize to -1 (no slot found)
  
  // Step 1: Check for existing valid slot with matching ID
  for (int i=0; i<8; i++) {
    if (slaves[i].valid && slaves[i].id == id) { // If slot i is valid and ID matches
      slot = i;  // Found existing slot
      break;     // Exit loop
    }
  }
  
  // Step 2: If no existing slot, find first free (invalid) slot
  if (slot == -1) {
    for (int i=0; i<8; i++) {
      if (!slaves[i].valid) { // If slot i is invalid (free)
        slot = i;             // Found free slot
        break;                // Exit loop
      }
    }
  }
  
  // Step 3: If no slots available, log error and exit
  if (slot == -1) { 
    Serial.println("No slot free for slave"); // Debug log: No available storage slots
    return; 
  }
  
  // Update slave data in the found slot
  slaves[slot].id = id;          // Store slave ID (hex address)
  slaves[slot].valid = true;     // Mark data as valid
  slaves[slot].lastSeen = millis();// Record timestamp of last valid data
  
  // Safely parse numerical values (handle 'nan'/'NaN' and empty strings)
  slaves[slot].temp = parseFloatToken(parts[1]);    // Parse temperature (part 1)
  slaves[slot].pressure = parseFloatToken(parts[2]); // Parse pressure (part 2)
  slaves[slot].altitude = parseFloatToken(parts[3]); // Parse altitude (part 3, not displayed)
  slaves[slot].distance = parseFloatToken(parts[4]); // Parse distance (part 4)
  slaves[slot].ampel = parts[5].toInt();            // Parse traffic light state (part 5 → integer)
}

// ==============================================
// parseFloatToken() Helper Function: Safely parse Float values from a string
// ==============================================
// Input: 'tok' → String containing a potential numerical value (e.g., "23.5", "nan", "")
// Output: Float value if valid; NAN (Not a Number) if invalid (empty, 'nan', 'NaN')
float parseFloatToken(const String &tok) {
  // Check if string is empty or contains 'nan'/'NaN' (case-sensitive)
  if (tok.length() == 0 || tok == "nan" || tok == "NaN") { 
    return NAN; // Return NAN for invalid values
  }
  // Convert valid string to Float and return
  return tok.toFloat(); 
}

// ==============================================
// renderAll() Function: Render all valid slave data to the TFT display
// ==============================================
void renderAll() {
  // Refresh display: Clear screen with black before rendering new data
  tft.fillScreen(ST77XX_BLACK); 
  
  // Draw title text
  tft.setCursor(0,0);         // Position cursor at top-left (X=0, Y=0)
  tft.setTextColor(ST77XX_WHITE); // Set text color to white
  tft.setTextSize(1);         // Standard text size
  tft.println("WeatherStation 2.0"); // Print main title
  tft.println("---------------------------"); // Add separator line
  
  int y = 24; // Starting Y-coordinate for sensor data (below title/separator)
  
  // Iterate over all slave data slots to render valid entries
  for (int i=0; i<8; i++) {
    if (!slaves[i].valid) continue; // Skip invalid/no-data slots
    
    // Render Slave ID (bottom-left, Cyan color)
    tft.setCursor(0, y);       // Position cursor at current Y
    tft.setTextSize(1);        // Standard text size
    tft.setTextColor(ST77XX_CYAN); // ID text color (Cyan for distinction)
    tft.print("ID 0x");         // Print "ID 0x" prefix
    tft.print(slaves[i].id, HEX); // Print ID in hex format (e.g., 0x10)
    
    // Render Temperature, Pressure, Distance (next line, Y+10)
    tft.setCursor(0, y+10);    // Move cursor down 10px
    tft.setTextColor(ST77XX_WHITE); // Use white for data values
    
    // Temperature (°C)
    if (!isnan(slaves[i].temp)) { // Check if value is valid (not NAN)
      tft.print("T: ");          // Prefix "T: "
      tft.print(slaves[i].temp, 1); // Print value with 1 decimal place (e.g., 23.5)
      tft.print(" C  ");         // Suffix "°C" with spacing for alignment
    } else {
      tft.print("T: N/A  ");     // "N/A" if temperature data invalid
    }
    
    // Pressure (hPa)
    if (!isnan(slaves[i].pressure)) {
      tft.print("P: ");          // Prefix "P: "
      tft.print(slaves[i].pressure, 1); // Print with 1 decimal place
      tft.print(" hPa ");        // Suffix with unit
    } else {
      tft.print("P: N/A ");      // "N/A" if pressure data invalid
    }
    
    // Distance (cm)
    if (!isnan(slaves[i].distance)) {
      tft.print(" D:");          // Prefix " D:" (spacing for alignment)
      tft.print(slaves[i].distance, 1); // Print with 1 decimal place
      tft.print("cm ");          // Suffix with unit
    } else {
      tft.print(" D:N/A ");      // "N/A" if distance data invalid
    }
    
    // Render Traffic Light State (next line, Y+20)
    tft.setCursor(0, y+20);    // Move cursor down 20px
    tft.print("State: ");        // Prefix "State: "
    
    // Map state integer to readable text (0-3)
    switch (slaves[i].ampel) {
      case 0: tft.print("RED"); break;        // 0 → Red state
      case 1: tft.print("RED_YELLOW"); break; // 1 → Red-Yellow state
      case 2: tft.print("GREEN"); break;       // 2 → Green state
      case 3: tft.print("YELLOW"); break;      // 3 → Yellow state
      default: tft.print("N/A"); break;        // Invalid state → "N/A"
    }
    
    // Advance Y-coordinate for next sensor entry (36px per entry to prevent overlap)
    y += 36; 
    // Stop rendering if Y exceeds 140px to avoid screen overflow (adjust based on display height)
    if (y > 140) break; 
  }
}