#include <WiFi.h>
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

void setup() {
  Serial.begin(115200);
  DEV_Module_Init();

#ifdef EPD_PWR_PIN
  pinMode(EPD_PWR_PIN, OUTPUT);
  DEV_Digital_Write(EPD_PWR_PIN, HIGH);
#endif

  // Connect to WiFi with timeout (10 seconds)
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
  } else {
    Serial.println("\nWiFi connection failed - continuing in offline mode");
  }

  // Initialize display and show boot splash (no unnecessary white clear)
  EPD_13IN3E_Init();
  EPD_13IN3E_ShowBootSplash(WIFI_SSID, TCP_PORT);

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
    c.stop();
  }
}

void loop() {}