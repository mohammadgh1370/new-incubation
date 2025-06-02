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
#define FAN_PIN				D,0  // Fan pin
#define ELEMENT_PIN	        D,1  // Element pin
#define INT0_PIN            D,2  // External interrupt 0 pin (rotation stop)
#define INT1_PIN            D,3  // External interrupt 1 pin (system reset)
#define ROTATION_PIN        D,4  // Egg rotation motor pin
#define HEATER_PIN			D,5  // Heater pin
#define VENT_FAN_PIN        D,6  // Ventilation fan pin
#define DHT_PIN             D,7  // DHT22 sensor pin

// LCD pin definitions
#define LCD_PORT            PORTB  // LCD data port
#define LCD_DIR             DDRB   // LCD data port direction
#define LCD_RS              PB0    // LCD Register Select pin
#define LCD_EN              PB2    // LCD Enable pin

// LM35 pin definitions
#define LM35_PIN            A,0  // LCD data port

// Default control parameters
#define SETTER_PERIOD_DAYS  19     // Setter period duration (days)
#define TEMP_SETTER         37	   // Temperature setpoint for setter period (°C)
#define TEMP_HATCHER        36.5   // Temperature setpoint for hatcher period (°C)
#define HUM_SETTER          60.0   // Humidity setpoint for setter period (%)
#define HUM_HATCHER         70.0   // Humidity setpoint for hatcher period (%)
#define TEMP_HYSTERESIS     0.2    // Temperature hysteresis (°C)
#define HUM_HYSTERESIS      4.0    // Humidity hysteresis (%)
#define FAN_TRIGGER_TEMP    0.6	   // Temperature offset to trigger ventilation fan (°C)
#define FAN_TRIGGER_HUM     5	   // Humidity offset to trigger ventilation fan (%)

// Rotation settings
#define ROTATION_INTERVAL_HOURS  2   // Rotation interval during setter period (hours)

// DHT22 settings
#define DHT_Type			DHT22  // DHT11 or DHT22
#define DHT_AVERAGE_COUNT   5      // Number of samples for averaging

// Display timing (in seconds)
#define SWITCH_DISPLAY_SECOND   5   // Time to display LM35 temperature
#define LM35_AVERAGE_COUNT		5	// Number of samples for averaging

#define DEVICE_SWITCH_DELAY		50	// Switching delay (ms)
#define ROTATION_IGNORE_DELAY	9	// Rotation delay change (second)
#define FAN_DELAY_START			3	// Fan delay start
#define ELEMENT_ON_TIME			30  // Element on
#define ELEMENT_OFF_TIME		270 // Element off

#define BUTTON_HOLD_TIME		3	// second

#endif // CONFIG_H