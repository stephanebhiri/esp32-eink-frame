#include <WiFi.h>
#include "esp_wifi.h"  // For Power Save mode
#include "DEV_Config.h"
#include "EPD_13in3e.h"
#include "WiFiConfig.h"

#define EPD_W 1200
#define EPD_H 1600
static const int BYTES_PER_LINE_HALF = EPD_W/4; // 300

static bool readN(WiFiClient& c, uint8_t* buf, size_t n) {
  size_t got=0; unsigned long t0=millis();
  while (got<n) {
    int av=c.available();
    if (av>0) { int r=c.read(buf+got, min((size_t)av, n-got)); if (r>0) got+=r; }
    else { if (millis()-t0>15000) return false; delay(1); }
  }
  return true;
}

/**
 * Calculate battery percentage for HUZZAH32 Feather
 * 
 * The HUZZAH32 includes a voltage divider on pin A13 (GPIO35) that reads
 * half the battery voltage. This allows monitoring LiPo battery levels
 * without additional hardware.
 * 
 * @return Battery percentage (0-100), or -1 if no battery detected
 */
int getBatteryPercentage() {
  // Configure ADC for optimal battery reading
  analogSetAttenuation(ADC_11db);  // 3.3V range with minimal noise
  analogSetWidth(12);              // 12-bit resolution (0-4095)
  
  // Take multiple readings for stability and average them
  int raw_total = 0;
  const int num_readings = 5;
  for (int i = 0; i < num_readings; i++) {
    raw_total += analogRead(A13);   // A13 = GPIO35 with built-in voltage divider
    delay(10);                      // Brief delay between readings
  }
  int raw_avg = raw_total / num_readings;
  
  // Convert ADC reading to actual battery voltage
  // HUZZAH32 voltage divider: VBAT/2 → ADC, so multiply by 2
  float voltage = (raw_avg / 4095.0) * 3.3 * 2.0;
  
  // Check for very low readings (no battery or USB power only)
  if (raw_avg < 100) {
    Serial.println("No battery detected - using USB power");
    return -1;  // Special value for display logic
  }
  
  // Calculate percentage using realistic LiPo voltage range
  // 3.0V = 0% (safe minimum), 4.2V = 100% (full charge)
  float percentage = ((voltage - 3.0) / (4.2 - 3.0)) * 100.0;
  
  // Ensure percentage stays within bounds
  if (percentage < 0) percentage = 0;
  if (percentage > 100) percentage = 100;
  
  Serial.printf("Battery: %.2fV (%d%%)\n", voltage, (int)percentage);
  return (int)percentage;
}

/**
 * ESP32 E-Ink Display Controller - Main Application
 * 
 * This application manages a Waveshare 13.3" 6-color e-ink display
 * with TCP streaming capabilities for real-time image updates.
 * 
 * Features:
 * - WiFi connectivity with automatic connection management
 * - TCP server for receiving packed 6-color image data
 * - Power optimization for battery operation
 * - Dual-controller synchronization for 1200x1600 resolution
 * 
 * Hardware: Adafruit HUZZAH32 Feather + Waveshare 13.3" E-Paper HAT
 * Protocol: Custom packed 6-color format (4 bits per pixel)
 */

void setup() {
  Serial.begin(115200);
  
  // Power optimization: Reduce CPU frequency for lower consumption
  // HUZZAH32 Feather benefits from this when running on battery
  setCpuFrequencyMhz(160);  // Reduced from default 240MHz
  
  // Initialize hardware pins and SPI communication
  DEV_Module_Init();

#ifdef EPD_PWR_PIN
  pinMode(EPD_PWR_PIN, OUTPUT);
  // Power management: start with screen OFF to save power
  DEV_Digital_Write(EPD_PWR_PIN, LOW);
#endif

  // WiFi Configuration
  // Connect in station mode with 10-second timeout for resilience
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("WiFi… SSID=%s\n", WIFI_SSID);
  
  unsigned long wifi_start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifi_start < 10000) { 
    delay(300); 
    Serial.print("."); 
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nOK, IP=%s\n", WiFi.localIP().toString().c_str());
    
    // Enable WiFi Power Save mode for ~75% power reduction
    // Radio sleeps between beacon intervals, wakes to check for data
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    Serial.println("WiFi Power Save enabled (20mA vs 80mA)");
  } else {
    Serial.println("\nWiFi connection failed - continuing in offline mode");
  }

  // Power ON screen for boot splash - longer stabilization
#ifdef EPD_PWR_PIN
  DEV_Digital_Write(EPD_PWR_PIN, HIGH);
  delay(100);  // Wait for power stabilization
#endif
  
  // Initialize display and show boot splash (no unnecessary white clear)
  EPD_13IN3E_Init();
  int battery_pct = getBatteryPercentage();
  EPD_13IN3E_ShowBootSplash(WIFI_SSID, TCP_PORT, battery_pct);
  
  // Power OFF screen after boot splash to save power
#ifdef EPD_PWR_PIN
  delay(1000);  // Let user see the splash
  DEV_Digital_Write(EPD_PWR_PIN, LOW);
  Serial.println("Screen powered OFF - will turn ON for updates");
#endif

  static WiFiServer server(TCP_PORT);
  server.begin();
  Serial.printf("TCP server on %u (send packed 6-color frame)\n", TCP_PORT);

  static uint8_t line[BYTES_PER_LINE_HALF];

  for (;;) {
    WiFiClient c = server.available();
    if (!c) { delay(20); continue; }
    Serial.println("Client connected");

    // Header: "E6" + w + h + fmt(0)
    uint8_t hdr[7];
    if (!readN(c, hdr, sizeof hdr)) { Serial.println("Header timeout"); c.stop(); continue; }
    uint16_t w = hdr[2] | (hdr[3] << 8);
    uint16_t h = hdr[4] | (hdr[5] << 8);
    uint8_t  f = hdr[6];
    Serial.printf("Header: w=%u h=%u fmt=%u\n", w, h, f);
    if (!(hdr[0]=='E' && hdr[1]=='6' && w==EPD_W && h==EPD_H && f==0)) {
      Serial.println("Bad header"); c.stop(); continue;
    }

    // Power ON screen for update - much longer stabilization
#ifdef EPD_PWR_PIN
    DEV_Digital_Write(EPD_PWR_PIN, HIGH);
    delay(100);  // Wait for power stabilization
#endif
    
    // Important: ensure clean state every frame
    EPD_13IN3E_Init();

    // Left (M)
    EPD_13IN3E_BeginFrameM();
    size_t totalM=0;
    for (int y=0; y<EPD_H; ++y) {
      if (!readN(c, line, BYTES_PER_LINE_HALF)) { Serial.println("Stream M error"); break; }
      EPD_13IN3E_WriteLineM(line);
      totalM += BYTES_PER_LINE_HALF;
      if ((y%100)==0) Serial.printf("M line %d/%d\r", y, EPD_H);
    }
    EPD_13IN3E_EndFrameM();
    Serial.printf("\nM total bytes=%u\n", (unsigned)totalM);

    // Right (S)
    EPD_13IN3E_BeginFrameS();
    size_t totalS=0;
    for (int y=0; y<EPD_H; ++y) {
      if (!readN(c, line, BYTES_PER_LINE_HALF)) { Serial.println("Stream S error"); break; }
      EPD_13IN3E_WriteLineS(line);
      totalS += BYTES_PER_LINE_HALF;
      if ((y%100)==0) Serial.printf("S line %d/%d\r", y, EPD_H);
    }
    EPD_13IN3E_EndFrameS();
    Serial.printf("\nS total bytes=%u\n", (unsigned)totalS);

    if (totalM == (size_t)EPD_H*BYTES_PER_LINE_HALF &&
        totalS == (size_t)EPD_H*BYTES_PER_LINE_HALF) {
      Serial.println("Refresh…");
      EPD_13IN3E_RefreshNow();
      Serial.println("Frame done");
      // Optional power-save; we always re-init at next frame:
      EPD_13IN3E_Sleep();
    } else {
      Serial.println("Incomplete frame; skip refresh");
    }
    
    // Power OFF screen after update to save power
#ifdef EPD_PWR_PIN
    delay(500);  // Let refresh complete
    DEV_Digital_Write(EPD_PWR_PIN, LOW);
    Serial.println("Screen powered OFF until next update");
#endif
    c.stop();
  }
}

void loop() {}