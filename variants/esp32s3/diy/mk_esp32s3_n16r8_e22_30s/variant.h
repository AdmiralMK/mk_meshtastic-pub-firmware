// Отключаем Ethernet полностью
#undef HAS_ETHERNET

// I2C
#define I2C_SDA SDA     // SDA = 8;
#define I2C_SCL SCL     // SCL = 18;
#define HAS_SCREEN 1
#define USE_SSD1306

// UART
#define UART_TX 43
#define UART_RX 44

// Buzzer
#define PIN_BUZZER 5

// External Notification (LED / Relay / Active Buzzer)
#define EXT_NOTIFY_OUT 4

// Button
#define BUTTON_PIN 0
#define BUTTON_NEED_PULLUP

// Power detector
#define EXT_PWR_DETECT 2

// Battery voltmeter
#define BATTERY_PIN                     1
#define ADC_CHANNEL                     ADC_CHANNEL_0
#define ADC_ATTENUATION                 ADC_ATTEN_DB_12
#define BATTERY_SENSE_RESOLUTION_BITS   12
#define ADC_MULTIPLIER                  1.68     // Для делителя 1 МОм и 680 кОм (верхнее плечо) 

// Чтобы при 5.7 В на пине получать 2.5 В, нужно изменить коэффициент деления напряжения.
// Коэффициент деления = 5.7 В / 2.5 В = 2.28
// Вам нужно подобрать резисторы делителя так, чтобы их соотношение давало коэффициент ~2.28 – 2.30.
// Пример стандартных номиналов:
// R1 (верхнее плечо): 130 кОм
// R2 (нижнее плечо): 100 кОм
// Итоговый коэффициент: (130 + 100) / 100 = 2.3
// 
// #define ADC_ATTENUATION                 ADC_ATTEN_DB_12
// #define ADC_MULTIPLIER                  2.30     // Физический коэффициент делителя (R1+R2)/R2
// 
// После прошивки вам нужно будет лишь слегка 
// подстроить ADC_MULTIPLIER (например, поставить 2.28 или 2.32), сверившись 
// с мультиметром, чтобы убрать микро-погрешность самих резисторов (у них есть допуск 1%).

// NEOPIXEL
#define HAS_NEOPIXEL 1                         // Enable the use of neopixels
#define NEOPIXEL_COUNT 1                     // How many neopixels are connected
#define NEOPIXEL_DATA 48                     // GPIO pin used to send data to the neopixels
#define NEOPIXEL_TYPE (NEO_GRB + NEO_KHZ800) // Type of neopixels in use
#define ENABLE_AMBIENTLIGHTING               // Turn on Ambient Lighting

// GPS
#define HAS_GPS 0
#undef GPS_RX_PIN
#undef GPS_TX_PIN
// #define HAS_GPS 1 // Don't need to set this to 0 to prevent a crash as it doesn't crash if GPS not found, will probe by default
// #define PIN_GPS_EN 11
// #define GPS_EN_ACTIVE 1
// #define GPS_TX_PIN 12 // rx
// #define GPS_RX_PIN 13 // tx
// #define GPS_BAUDRATE 38400
// #define GPS_UBLOX10

// LoRa
#define USE_SX1262
#define SX126X_MAX_POWER    22          // Only for E22-900m30s !!!!!!!!!!!!!!!!
//#define SX126X_MAX_POWER 9            // Only for E22-900m33s !!!!!!!!!!!!!!!!
#define SX126X_DIO3_TCXO_VOLTAGE 1.8    // E22 series TCXO reference voltage is 1.8V

// ==================================================
// Настройки для LORA по умолчанию для MK ESP32S3 N16R8 E22
// ==================================================
#define SX126X_CS       14  // EBYTE module's NSS pin // FIXME: rename to SX126X_SS
#define SX126X_SCK      15  // EBYTE module's SCK pin
#define SX126X_MOSI     38  // EBYTE module's MOSI pin
#define SX126X_MISO     39  // EBYTE module's MISO pin
#define SX126X_RESET    40  // EBYTE module's NRST pin
#define SX126X_BUSY     41  // EBYTE module's BUSY pin
#define SX126X_DIO1     42  // EBYTE module's DIO1 pin
#define SX126X_RXEN     17  // Schematic connects EBYTE module's RXEN pin to MCU
#define SX126X_TXEN     16  // Schematic connects EBYTE module's TXEN pin to MCU

#define LORA_CS     SX126X_CS       // Compatibility with variant file configuration structure
#define LORA_SCK    SX126X_SCK      // Compatibility with variant file configuration structure
#define LORA_MOSI   SX126X_MOSI     // Compatibility with variant file configuration structure
#define LORA_MISO   SX126X_MISO     // Compatibility with variant file configuration structure
#define LORA_DIO1   SX126X_DIO1     // Compatibility with variant file configuration structure
