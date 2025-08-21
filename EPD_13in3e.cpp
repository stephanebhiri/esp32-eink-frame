/******************************************************************************
 * EPD 13.3" 6-Color E-Paper Display Driver
 * 
 * Driver for Waveshare 13.3inch e-Paper (K) with 6-color support
 * Resolution: 1200x1600 pixels
 * Colors: Black, White, Yellow, Red, Blue, Green
 * 
 * Copyright (c) 2025 Stephane Bhiri
 ******************************************************************************/

#include "EPD_13in3e.h"
#include "Debug.h"
#include <WiFi.h>

// SPI Configuration Constants
const UBYTE PSR_V[2] = {0xDF, 0x69};
const UBYTE PWR_V[6] = {0x0F, 0x00, 0x28, 0x2C, 0x28, 0x38};
const UBYTE POF_V[1] = {0x00};
const UBYTE DRF_V[1] = {0x00};
const UBYTE CDI_V[1] = {0xF7};
const UBYTE TCON_V[2] = {0x03, 0x03};
const UBYTE TRES_V[4] = {0x04, 0xB0, 0x03, 0x20};
const UBYTE CMD66_V[6] = {0x49, 0x55, 0x13, 0x5D, 0x05, 0x10};
const UBYTE EN_BUF_V[1] = {0x07};
const UBYTE CCSET_V[1] = {0x01};
const UBYTE PWS_V[1] = {0x22};
const UBYTE AN_TM_V[9] = {0xC0, 0x1C, 0x1C, 0xCC, 0xCC, 0xCC, 0x15, 0x15, 0x55};
const UBYTE AGID_V[1] = {0x10};
const UBYTE BTST_P_V[2] = {0xE8, 0x28};
const UBYTE BOOST_VDDP_EN_V[1] = {0x01};
const UBYTE BTST_N_V[2] = {0xE8, 0x28};
const UBYTE BUCK_BOOST_VDDN_V[1] = {0x01};
const UBYTE TFT_VCOM_POWER_V[1] = {0x02};

/******************************************************************************
 * 8x8 Bitmap Font Table
 * ASCII printable characters (32-126)
 ******************************************************************************/
static const uint8_t font8x8_basic[95][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // ' ' (space)
    { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   // !
    { 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},   // "
    { 0x6C, 0xFE, 0x6C, 0x6C, 0x6C, 0xFE, 0x6C, 0x00},   // #
    { 0x30, 0x7C, 0xC0, 0x78, 0x0C, 0xF8, 0x30, 0x00},   // $
    { 0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00},   // %
    { 0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00},   // &
    { 0x60, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00},   // '
    { 0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00},   // (
    { 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00},   // )
    { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},   // *
    { 0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0x00},   // +
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x60, 0x00},   // ,
    { 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00},   // -
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00},   // .
    { 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00},   // /
    { 0x7C, 0xC6, 0xCE, 0xDE, 0xF6, 0xE6, 0x7C, 0x00},   // 0
    { 0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00},   // 1
    { 0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00},   // 2
    { 0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00},   // 3
    { 0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00},   // 4
    { 0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00},   // 5
    { 0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00},   // 6
    { 0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00},   // 7
    { 0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00},   // 8
    { 0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00},   // 9
    { 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00},   // :
    { 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x60, 0x00},   // ;
    { 0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00},   // <
    { 0x00, 0x00, 0xFC, 0x00, 0x00, 0xFC, 0x00, 0x00},   // =
    { 0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00},   // >
    { 0x78, 0xCC, 0x0C, 0x18, 0x30, 0x00, 0x30, 0x00},   // ?
    { 0x7C, 0xC6, 0xDE, 0xDE, 0xDE, 0xC0, 0x78, 0x00},   // @
    { 0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00},   // A
    { 0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00},   // B
    { 0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00},   // C
    { 0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00},   // D
    { 0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00},   // E
    { 0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00},   // F
    { 0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3E, 0x00},   // G
    { 0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00},   // H
    { 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},   // I
    { 0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00},   // J
    { 0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00},   // K
    { 0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00},   // L
    { 0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00},   // M
    { 0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00},   // N
    { 0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x00},   // O
    { 0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00},   // P
    { 0x78, 0xCC, 0xCC, 0xCC, 0xDC, 0x78, 0x1C, 0x00},   // Q
    { 0xFC, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0xE6, 0x00},   // R
    { 0x78, 0xCC, 0xE0, 0x70, 0x1C, 0xCC, 0x78, 0x00},   // S
    { 0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},   // T
    { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x00},   // U
    { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00},   // V
    { 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0xEE, 0xC6, 0x00},   // W
    { 0xC6, 0xC6, 0x6C, 0x38, 0x38, 0x6C, 0xC6, 0x00},   // X
    { 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00},   // Y
    { 0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00},   // Z
    { 0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00},   // [
    { 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00},   // \
    { 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00},   // ]
    { 0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00},   // ^
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // _
    { 0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},   // `
    { 0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00},   // a
    { 0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0xDC, 0x00},   // b
    { 0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00},   // c
    { 0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00},   // d
    { 0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00},   // e
    { 0x38, 0x6C, 0x60, 0xF0, 0x60, 0x60, 0xF0, 0x00},   // f
    { 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8},   // g
    { 0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00},   // h
    { 0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00},   // i
    { 0x0C, 0x00, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78},   // j
    { 0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00},   // k
    { 0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},   // l
    { 0x00, 0x00, 0xCC, 0xFE, 0xFE, 0xD6, 0xC6, 0x00},   // m
    { 0x00, 0x00, 0xF8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00},   // n
    { 0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00},   // o
    { 0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0},   // p
    { 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E},   // q
    { 0x00, 0x00, 0xDC, 0x76, 0x66, 0x60, 0xF0, 0x00},   // r
    { 0x00, 0x00, 0x7C, 0xC0, 0x78, 0x0C, 0xF8, 0x00},   // s
    { 0x10, 0x30, 0x7C, 0x30, 0x30, 0x34, 0x18, 0x00},   // t
    { 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00},   // u
    { 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00},   // v
    { 0x00, 0x00, 0xC6, 0xD6, 0xFE, 0xFE, 0x6C, 0x00},   // w
    { 0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00},   // x
    { 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8},   // y
    { 0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00},   // z
    { 0x1C, 0x30, 0x30, 0xE0, 0x30, 0x30, 0x1C, 0x00},   // {
    { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // |
    { 0xE0, 0x30, 0x30, 0x1C, 0x30, 0x30, 0xE0, 0x00},   // }
    { 0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // ~
};

// Helper functions
static void EPD_13IN3E_CS_ALL(UBYTE Value) {
    DEV_Digital_Write(EPD_CS_M_PIN, Value);
    DEV_Digital_Write(EPD_CS_S_PIN, Value);
}

static void EPD_13IN3E_SPI_Sand(UBYTE Cmd, const UBYTE *buf, UDOUBLE Len) {
    DEV_SPI_WriteByte(Cmd);
    DEV_SPI_Write_nByte((UBYTE *)buf,Len);
}

static void EPD_13IN3E_Reset(void) {
    // Official Waveshare double reset sequence for dual-controller initialization
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(30);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(30);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(30);
    DEV_Digital_Write(EPD_RST_PIN, 0);  // Critical: Second reset cycle
    DEV_Delay_ms(30);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(30);
}

static void EPD_13IN3E_SendCommand(UBYTE Reg) {
    DEV_SPI_WriteByte(Reg);
}

static void EPD_13IN3E_SendData(UBYTE Reg) {
    DEV_SPI_WriteByte(Reg);
}

static void EPD_13IN3E_SendData2(const UBYTE *buf, uint32_t Len) {
    if (!buf || Len == 0) return;
    DEV_SPI_Write_nByte((UBYTE *)buf,Len);
}

static void EPD_13IN3E_ReadBusyH(void) {
    Debug("e-Paper busy\r\n");
    while(!DEV_Digital_Read(EPD_BUSY_PIN)) {
        DEV_Delay_ms(10);
    }
    DEV_Delay_ms(20);
    Debug("e-Paper busy release\r\n");
}

static void EPD_13IN3E_TurnOnDisplay(void) {
    printf("Write PON \r\n");
    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SendCommand(0x04);
    EPD_13IN3E_CS_ALL(1);
    EPD_13IN3E_ReadBusyH();

    printf("Write DRF \r\n");
    DEV_Delay_ms(50);
    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SPI_Sand(DRF, DRF_V, sizeof(DRF_V));
    EPD_13IN3E_CS_ALL(1);
    EPD_13IN3E_ReadBusyH();

    printf("Write POF \r\n");
    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SPI_Sand(POF, POF_V, sizeof(POF_V));
    EPD_13IN3E_CS_ALL(1);
    // Critical: Official driver does NOT wait for busy after POF - timing sensitive
    printf("Display Done!! \r\n");
}

/******************************************************************************
 * Display Initialization and Control Functions
 ******************************************************************************/
void EPD_13IN3E_Init(void) {
    EPD_13IN3E_Reset();

    DEV_Digital_Write(EPD_CS_M_PIN, 0);
    EPD_13IN3E_SPI_Sand(AN_TM, AN_TM_V, sizeof(AN_TM_V));
    EPD_13IN3E_CS_ALL(1);

    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SPI_Sand(CMD66, CMD66_V, sizeof(CMD66_V));
    EPD_13IN3E_CS_ALL(1);

    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SPI_Sand(PSR, PSR_V, sizeof(PSR_V));
    EPD_13IN3E_CS_ALL(1);

    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SPI_Sand(CDI, CDI_V, sizeof(CDI_V));
    EPD_13IN3E_CS_ALL(1);

    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SPI_Sand(TCON, TCON_V, sizeof(TCON_V));
    EPD_13IN3E_CS_ALL(1);

    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SPI_Sand(AGID, AGID_V, sizeof(AGID_V));
    EPD_13IN3E_CS_ALL(1);

    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SPI_Sand(PWS, PWS_V, sizeof(PWS_V));
    EPD_13IN3E_CS_ALL(1);

    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SPI_Sand(CCSET, CCSET_V, sizeof(CCSET_V));
    EPD_13IN3E_CS_ALL(1);

    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SPI_Sand(TRES, TRES_V, sizeof(TRES_V));
    EPD_13IN3E_CS_ALL(1);

    DEV_Digital_Write(EPD_CS_M_PIN, 0);
    EPD_13IN3E_SPI_Sand(PWR_epd, PWR_V, sizeof(PWR_V));
    EPD_13IN3E_CS_ALL(1);

    DEV_Digital_Write(EPD_CS_M_PIN, 0);
    EPD_13IN3E_SPI_Sand(EN_BUF, EN_BUF_V, sizeof(EN_BUF_V));
    EPD_13IN3E_CS_ALL(1);

    DEV_Digital_Write(EPD_CS_M_PIN, 0);
    EPD_13IN3E_SPI_Sand(BTST_P, BTST_P_V, sizeof(BTST_P_V));
    EPD_13IN3E_CS_ALL(1);

    DEV_Digital_Write(EPD_CS_M_PIN, 0);
    EPD_13IN3E_SPI_Sand(BOOST_VDDP_EN, BOOST_VDDP_EN_V, sizeof(BOOST_VDDP_EN_V));
    EPD_13IN3E_CS_ALL(1);

    DEV_Digital_Write(EPD_CS_M_PIN, 0);
    EPD_13IN3E_SPI_Sand(BTST_N, BTST_N_V, sizeof(BTST_N_V));
    EPD_13IN3E_CS_ALL(1);

    DEV_Digital_Write(EPD_CS_M_PIN, 0);
    EPD_13IN3E_SPI_Sand(BUCK_BOOST_VDDN, BUCK_BOOST_VDDN_V, sizeof(BUCK_BOOST_VDDN_V));
    EPD_13IN3E_CS_ALL(1);

    DEV_Digital_Write(EPD_CS_M_PIN, 0);
    EPD_13IN3E_SPI_Sand(TFT_VCOM_POWER, TFT_VCOM_POWER_V, sizeof(TFT_VCOM_POWER_V));
    EPD_13IN3E_CS_ALL(1);
}

/******************************************************************************
Clear Screen Function
******************************************************************************/
void EPD_13IN3E_Clear(UBYTE color) {
    UDOUBLE Width, Height;
    UBYTE Color;
    Width = (EPD_13IN3E_WIDTH % 2 == 0)? (EPD_13IN3E_WIDTH / 2 ): (EPD_13IN3E_WIDTH / 2 + 1);
    Height = EPD_13IN3E_HEIGHT;
    Color = (color<<4)|color;

    DEV_Digital_Write(EPD_CS_M_PIN, 0);
    EPD_13IN3E_SendCommand(0x10);
    for (UDOUBLE j = 0; j < Height; j++) {
        for (UDOUBLE i = 0; i < Width/2; i++) {
            EPD_13IN3E_SendData(Color);
        }
        DEV_Delay_ms(1);
    }
    EPD_13IN3E_CS_ALL(1);

    DEV_Digital_Write(EPD_CS_S_PIN, 0);
    EPD_13IN3E_SendCommand(0x10);
    for (UDOUBLE j = 0; j < Height; j++) {
        for (UDOUBLE i = 0; i < Width/2; i++) {
            EPD_13IN3E_SendData(Color);
        }
        DEV_Delay_ms(1);
    }
    EPD_13IN3E_CS_ALL(1);

    EPD_13IN3E_TurnOnDisplay();
}

/******************************************************************************
 * Boot Splash Display Function
 ******************************************************************************/
void EPD_13IN3E_DisplayTextScreen(const char* ssid, uint16_t port) {
    Serial.println("*** e-Frame with Color Bands + Text ***");
    
    // Line buffer for rendering
    static const int BYTES_PER_LINE_HALF = EPD_13IN3E_WIDTH / 4; // 300 bytes per half line
    uint8_t line[BYTES_PER_LINE_HALF];
    
    // Get WiFi info for display - convert to uppercase for better font rendering
    String ip_line;
    String wifi_line;
    
    if (WiFi.status() == WL_CONNECTED) {
        ip_line = "IP: " + WiFi.localIP().toString() + " PORT: " + String(port);
        String ssid_upper = String(ssid);
        ssid_upper.toUpperCase();
        wifi_line = "WIFI: " + ssid_upper;
    } else {
        ip_line = "NO WIFI CONNECTION";
        wifi_line = "OFFLINE MODE";
    }
    
    // MAX 30 CHARACTERS (1200px / 40px per char = 30 chars)
    // Each line below is <= 30 chars for perfect fit
    const char* band_texts[] = {
        "E-INK FRAME (C) 2025",                      // Band 0 (black) - 20 chars
        ip_line.c_str(),                             // Band 1 (white) - Dynamic, ~25 chars
        wifi_line.c_str(),                           // Band 2 (yellow) - Dynamic, ~15 chars
        "WELCOME TO YOUR CANVAS",                    // Band 3 (red) - 22 chars
        "13.3 INCH COLOR DISPLAY",                   // Band 4 (blue) - 23 chars
        "READY FOR YOUR IMAGES"                      // Band 5 (green) - 21 chars
    };
    
    // Initialize the display (same as working code)
    EPD_13IN3E_Init();
    
    // Left half (Master)
    EPD_13IN3E_BeginFrameM();
    
    for (int y = 0; y < EPD_13IN3E_HEIGHT; y++) {
        // Determine color band and text for this line
        int band_index = y / 266;  // Which band (0-5)
        if (band_index > 5) band_index = 5;
        
        UBYTE band_color;
        if (band_index == 0) band_color = EPD_13IN3E_BLACK;
        else if (band_index == 1) band_color = EPD_13IN3E_WHITE;
        else if (band_index == 2) band_color = EPD_13IN3E_YELLOW;
        else if (band_index == 3) band_color = EPD_13IN3E_RED;
        else if (band_index == 4) band_color = EPD_13IN3E_BLUE;
        else band_color = EPD_13IN3E_GREEN;
        
        // Start with band background color
        UBYTE packed_color = (band_color << 4) | band_color;
        memset(line, packed_color, BYTES_PER_LINE_HALF);
        
        // Add large text in the middle of each band
        int y_in_band = y % 266;  // Position within the band (0-265)
        if (y_in_band >= 100 && y_in_band < 164) {  // Text zone (64 pixels tall)
            const char* text = band_texts[band_index];
            
            // Choose contrasting text color
            UBYTE text_color;
            if (band_color == EPD_13IN3E_BLACK) text_color = EPD_13IN3E_WHITE;
            else if (band_color == EPD_13IN3E_WHITE) text_color = EPD_13IN3E_BLACK;
            else if (band_color == EPD_13IN3E_YELLOW) text_color = EPD_13IN3E_BLACK;
            else text_color = EPD_13IN3E_WHITE;
            
            // Calculate which font row we're in (4x scaling vertically)
            int scaled_font_y = (y_in_band - 100) / 4;  // 0-15 range
            if (scaled_font_y < 8) {  // Only process first 8 rows (32 pixel height)
                int font_y = scaled_font_y;
                
                // Render text with 4x scaling (left half only)
                int text_x = 20;  // Start position
                for (const char* p = text; *p && text_x < 600; p++) {
                    if (*p >= 32 && *p <= 126) {
                        const uint8_t* font_char = font8x8_basic[*p - 32];
                        uint8_t line_data = font_char[font_y];
                        
                        // Draw character with 4x scaling horizontally
                        for (int bit = 0; bit < 8; bit++) {
                            if (line_data & (0x80 >> bit)) {
                                // Draw 4x4 block for each original pixel
                                for (int scale_x = 0; scale_x < 4; scale_x++) {
                                    int pixel_x = text_x + (bit * 4) + scale_x;
                                    if (pixel_x < 600) {
                                        int byte_idx = pixel_x / 2;
                                        if (byte_idx < BYTES_PER_LINE_HALF) {
                                            if (pixel_x % 2 == 0) {
                                                line[byte_idx] = (line[byte_idx] & 0x0F) | (text_color << 4);
                                            } else {
                                                line[byte_idx] = (line[byte_idx] & 0xF0) | (text_color & 0x0F);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        text_x += 40;  // Character spacing (32 + 8)
                    }
                }
            }
        }
        
        EPD_13IN3E_WriteLineM(line);
        
        if ((y % 100) == 0) {
            Serial.printf("M line %d/%d\r", y, EPD_13IN3E_HEIGHT);
        }
    }
    EPD_13IN3E_EndFrameM();
    
    // Right half (Slave) - continue text seamlessly
    EPD_13IN3E_BeginFrameS();
    
    for (int y = 0; y < EPD_13IN3E_HEIGHT; y++) {
        // Same band logic as Master
        int band_index = y / 266;
        if (band_index > 5) band_index = 5;
        
        UBYTE band_color;
        if (band_index == 0) band_color = EPD_13IN3E_BLACK;
        else if (band_index == 1) band_color = EPD_13IN3E_WHITE;
        else if (band_index == 2) band_color = EPD_13IN3E_YELLOW;
        else if (band_index == 3) band_color = EPD_13IN3E_RED;
        else if (band_index == 4) band_color = EPD_13IN3E_BLUE;
        else band_color = EPD_13IN3E_GREEN;
        
        // Start with band background color
        UBYTE packed_color = (band_color << 4) | band_color;
        memset(line, packed_color, BYTES_PER_LINE_HALF);
        
        // Continue text from left half
        int y_in_band = y % 266;
        if (y_in_band >= 100 && y_in_band < 164) {
            const char* text = band_texts[band_index];
            
            // Same contrasting text color as left
            UBYTE text_color;
            if (band_color == EPD_13IN3E_BLACK) text_color = EPD_13IN3E_WHITE;
            else if (band_color == EPD_13IN3E_WHITE) text_color = EPD_13IN3E_BLACK;
            else if (band_color == EPD_13IN3E_YELLOW) text_color = EPD_13IN3E_BLACK;
            else text_color = EPD_13IN3E_WHITE;
            
            int scaled_font_y = (y_in_band - 100) / 4;
            if (scaled_font_y < 8) {
                int font_y = scaled_font_y;
                
                // PIXEL-LEVEL CONTINUATION: Calculate exact pixel position where left half ends
                // Left half: characters start at x=20, each char is 40 pixels wide
                // We need to continue EXACTLY where left half stopped, not at character boundaries
                
                // Calculate where the text would end on left half (ignoring 600px limit)
                int theoretical_end_x = 20;  // Start position
                for (const char* p = text; *p; p++) {
                    if (*p >= 32 && *p <= 126) {
                        theoretical_end_x += 40;  // Each character takes 40 pixels
                    }
                }
                
                // How much text actually fits in 600 pixels?
                int actual_end_x_left = min(theoretical_end_x, 600);
                
                // Right half continues from pixel 600 (conceptually pixel 600 of full 1200px line)
                // But we render it as pixels 0-599 in right buffer
                // Offset = how many pixels past 600 the text would be
                int continuation_offset = max(0, actual_end_x_left - 600);
                
                // Start rendering text from where it would naturally continue
                int text_x = continuation_offset;
                int current_x = 20;  // Track position in full 1200px space
                
                for (const char* p = text; *p && text_x < 600; p++) {
                    if (*p >= 32 && *p <= 126) {
                        // Only render if this character extends into right half (past pixel 600)
                        if (current_x + 40 > 600) {  // Character overlaps into right half
                            // Calculate which part of character is in right half
                            int char_start_in_right = max(0, 600 - current_x);  // Skip pixels that were on left
                            int char_end_in_right = min(40, 600 + 600 - current_x);  // Don't exceed right boundary
                            
                            const uint8_t* font_char = font8x8_basic[*p - 32];
                            uint8_t line_data = font_char[font_y];
                            
                            for (int bit = 0; bit < 8; bit++) {
                                if (line_data & (0x80 >> bit)) {
                                    for (int scale_x = 0; scale_x < 4; scale_x++) {
                                        int pixel_offset = (bit * 4) + scale_x;  // 0-31 within character
                                        
                                        // Only render the part that belongs to right half
                                        if (pixel_offset >= char_start_in_right && pixel_offset < char_end_in_right) {
                                            int pixel_x = text_x + (pixel_offset - char_start_in_right);
                                            if (pixel_x < 600) {
                                                int byte_idx = pixel_x / 2;
                                                if (byte_idx < BYTES_PER_LINE_HALF) {
                                                    if (pixel_x % 2 == 0) {
                                                        line[byte_idx] = (line[byte_idx] & 0x0F) | (text_color << 4);
                                                    } else {
                                                        line[byte_idx] = (line[byte_idx] & 0xF0) | (text_color & 0x0F);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            text_x += (char_end_in_right - char_start_in_right);
                        }
                        current_x += 40;
                    }
                }
            }
        }
        
        EPD_13IN3E_WriteLineS(line);
        
        if ((y % 100) == 0) {
            Serial.printf("S line %d/%d\r", y, EPD_13IN3E_HEIGHT);
        }
    }
    EPD_13IN3E_EndFrameS();
    
    Serial.println("\nRefreshing display...");
    EPD_13IN3E_RefreshNow();
    
    Serial.println("Boot splash complete");
}


void EPD_13IN3E_ShowBootSplash(const char* ssid, uint16_t port) {
    EPD_13IN3E_DisplayTextScreen(ssid, port);
}

/******************************************************************************
 * Power Management Functions
 ******************************************************************************/
void EPD_13IN3E_Sleep(void) {
    EPD_13IN3E_CS_ALL(0);
    EPD_13IN3E_SendCommand(0x07);
    EPD_13IN3E_SendData(0XA5);
    EPD_13IN3E_CS_ALL(1);
    DEV_Delay_ms(100);
}

/******************************************************************************
 * TCP Streaming Functions
 ******************************************************************************/
void EPD_13IN3E_BeginFrameM(void) {
    DEV_Digital_Write(EPD_CS_M_PIN, 0);
    EPD_13IN3E_SendCommand(0x10);
}

void EPD_13IN3E_WriteLineM(const UBYTE *p300) {
    if (!p300) return;
    // Master handles left half - send all 300 bytes
    EPD_13IN3E_SendData2(p300, EPD_13IN3E_WIDTH/4);
}

void EPD_13IN3E_EndFrameM(void) {
    EPD_13IN3E_CS_ALL(1);
}

void EPD_13IN3E_BeginFrameS(void) {
    // Ensure Master is deselected before selecting Slave
    EPD_13IN3E_CS_ALL(1);  // Deselect all first
    DEV_Digital_Write(EPD_CS_S_PIN, 0);  // Select only Slave
    EPD_13IN3E_SendCommand(0x10);
}

void EPD_13IN3E_WriteLineS(const UBYTE *p300) {
    if (!p300) return;
    // Master handles left half - send all 300 bytes
    EPD_13IN3E_SendData2(p300, EPD_13IN3E_WIDTH/4);
}

void EPD_13IN3E_EndFrameS(void) {
    EPD_13IN3E_CS_ALL(1);
}

void EPD_13IN3E_RefreshNow(void) {
    EPD_13IN3E_TurnOnDisplay();
}

/******************************************************************************
Power Management Functions
******************************************************************************/
void EPD_13IN3E_PowerOn(void) {
    #ifdef EPD_PWR_PIN
    DEV_Digital_Write(EPD_PWR_PIN, 1);
    DEV_Delay_ms(100);
    #endif
    EPD_13IN3E_Reset();
    EPD_13IN3E_Init();
}

void EPD_13IN3E_PowerOff(void) {
    EPD_13IN3E_Sleep();
    #ifdef EPD_PWR_PIN
    DEV_Digital_Write(EPD_PWR_PIN, 0);
    #endif
}