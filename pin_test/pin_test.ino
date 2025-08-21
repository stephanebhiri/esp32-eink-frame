#include "../DEV_Config.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== PIN DIAGNOSTIC TEST ===");
  
  // Init pins
  pinMode(EPD_CS_M_PIN, OUTPUT);
  pinMode(EPD_CS_S_PIN, OUTPUT);
  pinMode(EPD_DC_PIN, OUTPUT);
  pinMode(EPD_RST_PIN, OUTPUT);
  pinMode(EPD_PWR_PIN, OUTPUT);
  pinMode(EPD_BUSY_PIN, INPUT);
  pinMode(EPD_SCK_PIN, OUTPUT);
  pinMode(EPD_MOSI_PIN, OUTPUT);
  
  Serial.println("Testing pin outputs...");
  
  // Test all output pins
  testPin("CS_M", EPD_CS_M_PIN);
  testPin("CS_S", EPD_CS_S_PIN);
  testPin("DC", EPD_DC_PIN);
  testPin("RST", EPD_RST_PIN);
  testPin("PWR", EPD_PWR_PIN);
  testPin("SCK", EPD_SCK_PIN);
  testPin("MOSI", EPD_MOSI_PIN);
  
  Serial.println("\nTesting BUSY pin (input):");
  Serial.printf("BUSY pin %d = %s\n", EPD_BUSY_PIN, digitalRead(EPD_BUSY_PIN) ? "HIGH" : "LOW");
  
  Serial.println("\n=== HARDWARE RESET TEST ===");
  digitalWrite(EPD_PWR_PIN, HIGH);
  delay(100);
  digitalWrite(EPD_RST_PIN, LOW);
  delay(10);
  digitalWrite(EPD_RST_PIN, HIGH);
  delay(200);
  Serial.printf("After reset, BUSY = %s\n", digitalRead(EPD_BUSY_PIN) ? "HIGH" : "LOW");
  
  Serial.println("\n=== SPI TEST ===");
  SPI.begin(EPD_SCK_PIN, -1, EPD_MOSI_PIN, -1);
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  
  digitalWrite(EPD_CS_M_PIN, LOW);
  SPI.transfer(0x55); // Test pattern
  digitalWrite(EPD_CS_M_PIN, HIGH);
  Serial.println("Sent 0x55 to Master controller");
  
  digitalWrite(EPD_CS_S_PIN, LOW);
  SPI.transfer(0xAA); // Test pattern
  digitalWrite(EPD_CS_S_PIN, HIGH);
  Serial.println("Sent 0xAA to Slave controller");
  
  SPI.endTransaction();
  
  Serial.println("\n=== CONTINUOUS MONITORING ===");
  Serial.println("Monitoring BUSY pin... (press reset to stop)");
}

void testPin(const char* name, int pin) {
  Serial.printf("Testing %s (pin %d): ", name, pin);
  
  digitalWrite(pin, LOW);
  delay(10);
  Serial.print("LOW ");
  
  digitalWrite(pin, HIGH);
  delay(10);
  Serial.println("HIGH - OK");
}

void loop() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 1000) {
    Serial.printf("BUSY=%s ", digitalRead(EPD_BUSY_PIN) ? "H" : "L");
    lastCheck = millis();
  }
}