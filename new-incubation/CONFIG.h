#ifndef CONFIG_H_
#define CONFIG_H_

#include "IO_MACROS.h"

// Clock frequency
#ifndef F_CPU
#define F_CPU 1000000UL
#endif

// RTC Clock frequency
#define F_SCL 50000L			/* Define SCL clock frequency */

// Pin definitions
#define HUMIDIFIER_FAN_PIN  D,0  // Humidifier fan pin
#define HUMIDIFIER_PIN      D,1  // Humidifier pin
#define INT0_PIN            D,2  // External interrupt 0 pin (system reset)
#define INT1_PIN            D,3  // External interrupt 1 pin (rotation stop)
#define ROTATION_PIN        D,4  // Egg rotation motor pin
#define DHT_PIN             D,5  // DHT22 sensor pin
#define HEATER_FAN_PIN      D,6  // Heater and fan pin
#define FAN_PIN             D,7  // Ventilation fan pin

// LCD pin definitions
#define LCD_PORT            PORTA  // LCD data port
#define LCD_DIR             DDRA   // LCD data port direction
#define LCD_RS              A,0    // LCD Register Select pin
#define LCD_RW              A,1    // LCD Read/Write pin
#define LCD_EN              A,2    // LCD Enable pin

// Default control parameters
#define SETTER_PERIOD_DAYS  19     // Setter period duration (days)
#define TEMP_SETTER         37.7   // Temperature setpoint for setter period (°C)
#define TEMP_HATCHER        37.3   // Temperature setpoint for hatcher period (°C)
#define HUM_SETTER          60.0   // Humidity setpoint for setter period (%)
#define HUM_HATCHER         70.0   // Humidity setpoint for hatcher period (%)
#define TEMP_HYSTERESIS     0.2    // Temperature hysteresis (°C)
#define HUM_HYSTERESIS      5.0    // Humidity hysteresis (%)
#define FAN_TRIGGER_TEMP    0.4    // Temperature offset to trigger ventilation fan (°C)
#define FAN_TRIGGER_HUM     4.0    // Humidity offset to trigger ventilation fan (%)

// Rotation settings
#define ROTATION_INTERVAL_HOURS  2   // Rotation interval during setter period (hours)

// DHT22 settings
#define DHT_Type			DHT22  //DHT11 or DHT22
#define DHT_AVERAGE_COUNT   5      // Number of samples for averaging

#endif // CONFIG_H