#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

#define USB_VID 0x303a
#define USB_PID 0x1001

static const uint8_t TX = 43;
static const uint8_t RX = 44;

static const uint8_t SDA = 8;
static const uint8_t SCL = 18;

static const uint8_t SS = 14;
// static const uint8_t MOSI = 39;
// static const uint8_t MISO = 11;
static const uint8_t MOSI = 38;
static const uint8_t MISO = 39;

static const uint8_t SCK = 15;
static const uint8_t RST_LoRa = 40;
static const uint8_t BUSY_LoRa = 41;

// #define SPI_MOSI                    (11)
// #define SPI_SCK                     (14)
// #define SPI_MISO                    (2)
// #define SPI_CS                      (13)

// #define SDCARD_CS                   SPI_CS

#endif /* Pins_Arduino_h */
