#pragma once
#include <Arduino.h>
#include <SPI.h>

// ==================== Types Waveshare ====================
typedef uint8_t  UBYTE;
typedef uint16_t UWORD;
typedef uint32_t UDOUBLE;

// ==================== Mapping GPIO ESP32 =================
// SPI (VSPI)
#define EPD_SCK_PIN     18
#define EPD_MOSI_PIN    23
// #define EPD_MISO_PIN  19   // non utilisé par le 13.3E

// Contrôle écran
#define EPD_CS_M_PIN     5    // CSM -> GPIO5
#define EPD_CS_S_PIN    15    // CSS -> GPIO15
#define EPD_DC_PIN      27
#define EPD_RST_PIN     33
#define EPD_BUSY_PIN     4

// HAT Rev 2.3 : PWR doit être HIGH pour allumer le driver
#define EPD_PWR_PIN     26    // si non géré par la lib, le sketch s'en charge

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