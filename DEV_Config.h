#pragma once
#include <Arduino.h>
#include <SPI.h>

// ==================== Types Waveshare ====================
typedef uint8_t  UBYTE;
typedef uint16_t UWORD;
typedef uint32_t UDOUBLE;

// ==================== Mapping GPIO HUZZAH32 =================
// SPI Hardware pins (fixed on HUZZAH32)
#define EPD_SCK_PIN      5    // Hardware SCK pin
#define EPD_MOSI_PIN    18    // Hardware MOSI pin
// #define EPD_MISO_PIN  19   // non utilisé par le 13.3E

// Contrôle écran - Adafruit HUZZAH32 Feather compatible
#define EPD_CS_M_PIN    33    // CSM -> GPIO33/A9 (Master)
#define EPD_CS_S_PIN    15    // CSS -> GPIO15/A8 (Slave)
#define EPD_DC_PIN      14    // DC  -> GPIO14/A6
#define EPD_RST_PIN     32    // RST -> GPIO32/A7
#define EPD_BUSY_PIN    27    // BUSY-> GPIO27/A10

// Power control (optional) - PIN 12 peut avoir des restrictions au boot
#define EPD_PWR_PIN     21    // PWR -> GPIO21 (plus sûr)

// Helpers
#define DEV_Digital_Write(pin, val) digitalWrite((pin), (val))
#define DEV_Digital_Read(pin)       digitalRead((pin))
#define DEV_Delay_ms(ms)            delay(ms)

// API attendue par le driver
int  DEV_Module_Init(void);
void DEV_Module_Exit(void);

// SPI primitives utilisées par EPD_13in3E
void DEV_SPI_WriteByte(UBYTE data);
void DEV_SPI_Write_nByte(UBYTE *data, UDOUBLE len);