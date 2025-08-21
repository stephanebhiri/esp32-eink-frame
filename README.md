# ESP32 E-ink Display Driver

Professional driver for Waveshare 13.3" 6-color e-Paper display with TCP streaming support.

## Gallery

### Boot Sequence
![Boot Sequence](images/boot-sequence.jpg)

### Art Display
![Art Display](images/art-display.jpg)

## Features

- **Display**: 13.3 inch, 1200x1600 resolution
- **Colors**: 6-color support (Black, White, Yellow, Red, Blue, Green)
- **Controller**: Dual SPI controller architecture (Master/Slave)
- **Connectivity**: WiFi-enabled TCP image streaming
- **Protocol**: Custom binary protocol for real-time updates
- **Boot Screen**: Displays connection info and system status

## Hardware Requirements

- **MCU**: ESP32 Development Board (tested on ESP32-DevKitC)
- **Display**: Waveshare 13.3inch e-Paper (K) 6-color
- **Power**: 5V USB or external power supply
- **Connections**: SPI interface with dual chip select

### Pin Connections

| ESP32 Pin | Display Pin | Function |
|-----------|-------------|----------|
| GPIO 5    | CS_M        | Master Chip Select |
| GPIO 4    | CS_S        | Slave Chip Select |
| GPIO 23   | MOSI        | SPI Data |
| GPIO 18   | SCK         | SPI Clock |
| GPIO 16   | RST         | Reset |
| GPIO 17   | DC          | Data/Command |
| GPIO 19   | BUSY        | Busy Signal |
| GPIO 15   | PWR         | Power Control (optional) |

## Software Setup

### Prerequisites

- Arduino IDE 1.8+ or PlatformIO
- ESP32 Board Package
- WiFi network credentials

### Installation

1. Clone this repository:
```bash
git clone https://github.com/stephanebhiri/esp32-eink-display.git
cd esp32-eink-display
```

2. Copy WiFi configuration:
```bash
cp WiFiConfig.h.example WiFiConfig.h
```

3. Edit `WiFiConfig.h` with your network credentials:
```cpp
const char* WIFI_SSID = "YourNetwork";
const char* WIFI_PASS = "YourPassword";
const uint16_t TCP_PORT = 3333;
```

4. Open project in Arduino IDE and upload to ESP32

## TCP Streaming Protocol

The display accepts packed 6-color image data over TCP port 3333.

### Protocol Format

```
Header (7 bytes):
- Magic: "E6" (2 bytes)
- Width: uint16_t little-endian (2 bytes)
- Height: uint16_t little-endian (2 bytes)
- Format: 0x00 (1 byte)

Data:
- Left half: 300 bytes per line × 1600 lines
- Right half: 300 bytes per line × 1600 lines
```

### Color Encoding

Each pixel uses 4 bits (2 pixels per byte):
- 0x0: Black
- 0x1: White
- 0x2: Yellow
- 0x3: Red
- 0x5: Blue
- 0x6: Green

## API Reference

### Core Functions

```cpp
void EPD_13IN3E_Init(void);
void EPD_13IN3E_Clear(UBYTE color);
void EPD_13IN3E_Sleep(void);
```

### Streaming Functions

```cpp
void EPD_13IN3E_BeginFrameM(void);  // Start Master frame
void EPD_13IN3E_WriteLineM(const UBYTE *data);  // Write line to Master
void EPD_13IN3E_EndFrameM(void);    // End Master frame

void EPD_13IN3E_BeginFrameS(void);  // Start Slave frame
void EPD_13IN3E_WriteLineS(const UBYTE *data);  // Write line to Slave
void EPD_13IN3E_EndFrameS(void);    // End Slave frame

void EPD_13IN3E_RefreshNow(void);   // Update display
```

## Performance

- Full refresh: ~15 seconds
- Data transfer: ~5 seconds for complete frame
- Power consumption: <50mA average, <500mA peak

## Boot Screen

The display shows system information on startup:
- Device identification
- WiFi connection status
- IP address and TCP port
- Ready state indicator

## Troubleshooting

### Display not updating
- Check SPI connections
- Verify dual reset sequence completes
- Ensure proper power supply (5V, >1A)

### WiFi connection fails
- Display continues in offline mode after 10 seconds
- Check SSID and password in WiFiConfig.h
- Verify network is 2.4GHz compatible

### Partial display updates
- Ensure both Master and Slave controllers receive data
- Check CS_M and CS_S pin connections
- Verify complete data transmission (960KB per frame)

## License

Copyright (c) 2025 Stephane Bhiri

This project is licensed under the MIT License - see LICENSE file for details.

## Author

**Stephane Bhiri**
- GitHub: [@stephanebhiri](https://github.com/stephanebhiri)

## Acknowledgments

- Waveshare for display hardware documentation
- ESP32 community for development tools