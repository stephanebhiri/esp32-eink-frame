#pragma once
#include <Arduino.h>
#include <SPI.h>

// ==================== Types Waveshare ====================
typedef uint8_t  UBYTE;
typedef uint16_t UWORD;
typedef uint32_t UDOUBLE;

/**
 * GPIO Pin Mapping for Adafruit HUZZAH32 Feather
 * 
 * This configuration is optimized for the HUZZAH32 Feather board
 * connected to a Waveshare 13.3" e-Paper HAT.
 * 
 * Power considerations:
 * - The display can be powered directly from the Feather's BAT pin (3.7-4.2V)
 * - The HAT's onboard LDO regulators handle voltage conversion
 * - This enables full battery operation with charge management
 */

// SPI Hardware pins (fixed on HUZZAH32 Feather)
#define EPD_SCK_PIN      5    // Hardware SCK pin
#define EPD_MOSI_PIN    18    // Hardware MOSI pin
// #define EPD_MISO_PIN  19   // Not used by 13.3" display

// Display control pins - Optimized for HUZZAH32 Feather
#define EPD_CS_M_PIN    33    // Chip Select Master (GPIO33/A9)
#define EPD_CS_S_PIN    15    // Chip Select Slave (GPIO15/A8)
#define EPD_DC_PIN      14    // Data/Command (GPIO14/A6)
#define EPD_RST_PIN     32    // Reset (GPIO32/A7)
#define EPD_BUSY_PIN    27    // Busy signal (GPIO27/A10)

// Power control - GPIO21 avoids boot restrictions of GPIO12
#define EPD_PWR_PIN     21    // Power control (GPIO21)

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