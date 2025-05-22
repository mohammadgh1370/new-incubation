#include "CONFIG.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "LCD_16x2.h"
#include "I2C_MASTER.h"
#include "DS1307.h"
#include "DHT.h"

// Custom characters for LCD
const uint8_t fan[8] PROGMEM        = {0x00, 0x0C, 0x05, 0x1F, 0x14, 0x06, 0x00, 0x00};
const uint8_t humidifier[8] PROGMEM = {0x00, 0x0A, 0x14, 0x0A, 0x1F, 0x1F, 0x00, 0x00};
const uint8_t heater[8] PROGMEM     = {0x00, 0x09, 0x12, 0x12, 0x09, 0x09, 0x12, 0x00};
const uint8_t rotate[8] PROGMEM     = {0x1E, 0x12, 0x07, 0x0A, 0x1C, 0x09, 0x0F, 0x00};

// Global variables
volatile bool first_run_flag = false;
volatile bool update_display_flag = false;
volatile bool control_flag = false;
volatile bool read_dht_flag = false;
volatile bool rotate_flag = false;
double temp = 0, hum = 0;
double arr_avr_t[DHT_AVERAGE_COUNT] = {0};
double arr_avr_h[DHT_AVERAGE_COUNT] = {0};
volatile double avr_temp = 0, avr_hum = 0;
volatile uint8_t dht_index = 0;
volatile struct tm* t = NULL;
volatile uint8_t timer0_counter = 0;
volatile uint8_t timer2_counter = 0;
volatile int i,j;

// Control states
typedef enum { OFF, ON } ControlState;
ControlState heater_state = OFF;
ControlState humidifier_state = OFF;
ControlState humidifier_fan_state = OFF;
ControlState fan_state = OFF;

// Function prototypes
void WDT_Init(void);
void LCD_SetupCustomChars(void);
void Timer0_Init(void);
void Timer1_Init(void);
void Timer2_Init(void);
// void INT0_Init(void);
// void INT1_Init(void);
ControlState HysteresisControl(double current, double setpoint, double hysteresis, ControlState current_state);
void UpdateDisplay(void);
void ControlDevices(void);
void ReadDHT(void);

// Initialize watchdog timer (call once)
void WDT_Init(void) {
	WDTCR = (1 << WDE) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0); // Enable watchdog with ~2.1s timeout
}

// Setup custom characters for LCD
void LCD_SetupCustomChars(void) {
	LCD_Custom_Char(fan, 0);
	LCD_Custom_Char(humidifier, 1);
	LCD_Custom_Char(heater, 2);
	LCD_Custom_Char(rotate, 3);
}

// Initialize Timer0 for periodic display updates (~500ms)
void Timer0_Init(void) {
	TCCR0 |= (1 << WGM01) | (1 << CS01) | (1 << CS00); // CTC mode, prescaler 64
	OCR0 = 249; // ~16ms per interrupt
	TIMSK |= (1 << OCIE0); // Enable Timer0 compare match interrupt
}

// Initialize Timer1 for periodic control tasks (~500ms)
void Timer1_Init(void) {
	TCCR1B |= (1 << WGM12) | (1 << CS01) | (1 << CS00); // CTC mode, prescaler 64
	OCR1A = 7811; // ~500ms
	TIMSK |= (1 << OCIE1A); // Enable Timer1 compare match interrupt
}

// Initialize Timer2 for periodic DHT sensor readings (~500ms)
void Timer2_Init(void) {
	TCCR2 |= (1 << WGM21) | (1 << CS22); // CTC mode, prescaler 64
	OCR2 = 249; // ~16ms per interrupt
	TIMSK |= (1 << OCIE2); // Enable Timer2 compare match interrupt
}

// Initialize INT0 (system reset)
// void INT0_Init(void) {
// 	GICR |= (1 << INT0);
// 	MCUCR |= (1 << ISC01); // Falling edge
// }
// 
// Initialize INT1 (rotation stop)
// void INT1_Init(void) {
// 	GICR |= (1 << INT1);
// 	MCUCR |= (1 << ISC11); // Falling edge
// }

// Hysteresis control logic
ControlState HysteresisControl(double current, double setpoint, double hysteresis, ControlState current_state) {
	if (current < setpoint - hysteresis && current_state == OFF) {
		return ON;
	} else if (current > setpoint + hysteresis && current_state == ON) {
		return OFF;
	}
	return current_state;
}

// Update LCD display
void UpdateDisplay(void) {
	char buffer[30];
	sprintf(buffer, "%02d:%02d:%02d:%02d", t->day - 1, t->hour, t->minute, t->second);
	LCD_String_xy(0, 0, buffer);

	enum DHT_Status_t status = DHT_GetStatus();
	if (status == DHT_Ok) {
		char temp_str[10], hum_str[10];
		dtostrf(avr_temp, 2, 1, temp_str);
		dtostrf(avr_hum, 2, 1, hum_str);
		sprintf(buffer, "T=%sC H=%s%%", temp_str, hum_str);
	} else {
		sprintf(buffer, "T=%dErr H=%dErr", status, status);
	}
	LCD_String_xy(1, 0, buffer);

	LCD_Char_xy(0, 11, rotate_flag ? 3 : ' ');
	LCD_Char_xy(0, 12, heater_state ? 0 : ' ');
	LCD_Char_xy(0, 13, heater_state ? 2 : ' ');
	LCD_Char_xy(0, 14, humidifier_fan_state ? 0 : ' ');
	LCD_Char_xy(0, 15, humidifier_state ? 1 : ' ');
	LCD_String_xy(1, 15, (t->day - 1 < SETTER_PERIOD_DAYS) ? "S" : "H");
}

// Control devices based on temperature and humidity
void ControlDevices(void) {
	double temp_setpoint = (t->day - 1 < SETTER_PERIOD_DAYS) ? TEMP_SETTER : TEMP_HATCHER;
	double hum_setpoint = (t->day - 1 < SETTER_PERIOD_DAYS) ? HUM_SETTER : HUM_HATCHER;

	heater_state = HysteresisControl(avr_temp, temp_setpoint, TEMP_HYSTERESIS, heater_state);
	DigitalWrite(HEATER_FAN_PIN, heater_state);
	humidifier_state = HysteresisControl(avr_hum, hum_setpoint, HUM_HYSTERESIS, humidifier_state);
	DigitalWrite(HUMIDIFIER_PIN, humidifier_state);
	humidifier_fan_state = HysteresisControl(avr_hum, hum_setpoint, HUM_HYSTERESIS / 2, humidifier_fan_state);
	DigitalWrite(HUMIDIFIER_FAN_PIN, humidifier_fan_state);
	fan_state = (avr_temp > temp_setpoint + FAN_TRIGGER_TEMP || avr_hum > hum_setpoint + FAN_TRIGGER_HUM) ? ON : OFF;
	DigitalWrite(FAN_PIN, fan_state);

	// check in setter period
	if (t->day - 1 < SETTER_PERIOD_DAYS && t->hour % ROTATION_INTERVAL_HOURS == 0 && t->minute == 0 && t->second == 0) {
		rotate_flag = true;
	}
	DigitalWrite(ROTATION_PIN, rotate_flag);
}

// Read DHT22 sensor
void ReadDHT(void) {
	enum DHT_Status_t status = DHT_Read(&temp, &hum);
	if (status == DHT_Ok) {
		arr_avr_t[dht_index] = temp;
		arr_avr_h[dht_index] = hum;
		dht_index = (dht_index + 1) % DHT_AVERAGE_COUNT;
		avr_temp = average(arr_avr_t, DHT_AVERAGE_COUNT);
		avr_hum = average(arr_avr_h, DHT_AVERAGE_COUNT);
	}
}

// Timer0 ISR: Trigger display update every ~16ms
ISR(TIMER0_COMP_vect) {
	if (++timer0_counter >= 31) { // Trigger every 31 interrupts (~500ms)
		timer0_counter = 0;
		update_display_flag = true;
	}
}

// Timer1 ISR: Trigger device control every ~500ms
ISR(TIMER1_COMPA_vect) {
	wdt_reset();
	control_flag = true;
}

// Timer2 ISR: Trigger DHT reading every ~16ms
ISR(TIMER2_COMP_vect) {
	if (++timer2_counter >= 124) { // Trigger every 124 interrupts (~2000ms)
		timer2_counter = 0;
		read_dht_flag = true;
	}
}

// INT0 ISR: Stop rotation with debounce
ISR(INT0_vect) {
	rotate_flag = false;
}

// INT1 ISR: Reset system with debounce
ISR(INT1_vect) {
	if (!first_run_flag) {
		first_run_flag = true;
		RTC_Clock_Write(0, 0, 0, hour_24);
		RTC_Calendar_Write(1, 1, 1, 19);
	}
}

int main(void) {
	LCD_Init();
    I2C_Init();
    DHT_Setup();
    LCD_SetupCustomChars();
    WDT_Init();
    Timer0_Init();
    Timer1_Init();
    Timer2_Init();
//     INT0_Init();
//     INT1_Init();

//     PinMode(ROTATION_PIN, Output);
//     PinMode(HEATER_FAN_PIN, Output);
//     PinMode(HUMIDIFIER_PIN, Output);
//     PinMode(FAN_PIN, Output);
//     PinMode(HUMIDIFIER_FAN_PIN, Output);

    sei();
    set_sleep_mode(SLEEP_MODE_IDLE);

    while (1) {
	    if (update_display_flag) {
		    t = RTC_Read_TimeDate();
		    UpdateDisplay();
		    update_display_flag = false;
	    }
// 	    if (control_flag) {
// 		    ControlDevices();
// 		    control_flag = false;
// 	    }
	    if (read_dht_flag) {
		    ReadDHT();
		    read_dht_flag = false;
	    }
	    sleep_mode();
    }
}