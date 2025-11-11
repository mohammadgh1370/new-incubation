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
#include "ADC.h"
#include "24c32.h"

// Custom characters for LCD
const uint8_t fan[8] PROGMEM        = {0x00, 0x0C, 0x05, 0x1F, 0x14, 0x06, 0x00, 0x00};
const uint8_t humidifier[8] PROGMEM = {0x04, 0x0E, 0x19, 0x1D, 0x1F, 0x1F, 0x0E, 0x00};
const uint8_t heater[8] PROGMEM     = {0x00, 0x09, 0x12, 0x12, 0x09, 0x09, 0x12, 0x00};
const uint8_t rotate[8] PROGMEM     = {0x1E, 0x12, 0x07, 0x0A, 0x1C, 0x09, 0x0F, 0x00};

// Global variables
volatile bool update_display_flag = false;
volatile bool control_flag = false;
volatile bool read_dht_flag = false;
volatile bool read_adc_flag = false;
volatile bool int1_flag = false;
volatile bool read_rtc_flag = false;

double temp = 0, hum = 0;
float out_temp;

double arr_avr_t[DHT_AVERAGE_COUNT] = {0};
double arr_avr_h[DHT_AVERAGE_COUNT] = {0};
double arr_avr_out_t[LM35_AVERAGE_COUNT] = {0};
volatile double avr_temp = 0, avr_hum = 0;
volatile uint8_t dht_index = 0;
volatile uint8_t lm35_index = 0;
volatile struct tm* t = NULL;

volatile uint32_t rotation_start_time = 0;
volatile uint32_t fan_start_time = 0;
volatile uint32_t last_element_start = 0;
volatile uint32_t heater_start_time = 0;
volatile uint8_t last_second = 0;
volatile uint32_t int1_press_start = 0;

// Control states
typedef enum { OFF, ON } ControlState;
ControlState heater_state = OFF;
ControlState prev_heater_state = OFF;
ControlState fan_state = OFF;
ControlState prev_fan_state = OFF;
ControlState vent_fan_state = OFF;
ControlState rotate_state = OFF;
ControlState prev_rotate_state = OFF;
ControlState humidifier_state = OFF;
ControlState display_state = OFF;

// Function prototypes
void LCD_SetupCustomChars(void);
void Timer1_Init(void);
void INT0_Init(void);
void INT1_Init(void);
ControlState HysteresisControl(double current, double setpoint, double hysteresis, ControlState current_state);
void UpdateDisplay(void);
void ControlDevices(void);
void ReadDHT(void);
void ReadADC(void);

// Setup custom characters for LCD
void LCD_SetupCustomChars(void) {
	LCD_Custom_Char(fan, 0);
	LCD_Custom_Char(humidifier, 1);
	LCD_Custom_Char(heater, 2);
	LCD_Custom_Char(rotate, 3);
}

// Initialize Timer1 for periodic control tasks (~500ms)
void Timer1_Init(void) {
	// Set up Timer1 for CTC mode with a prescaler of 256
	// This will generate an interrupt every 50ms (at F_CPU = 1MHz)
	TCCR1B |= (1 << WGM12) | (1 << CS12);
	OCR1A = 195; // (1000000 / 256) / 195 = ~20Hz (50ms)
	TIMSK |= (1 << OCIE1A); // Enable Timer1 compare match interrupt
}

// Initialize INT0 (rotation stop)
void INT0_Init(void) {
	GICR |= (1 << INT0);
	MCUCR |= (1 << ISC01); // Falling edge
	MCUCR &= ~(1 << ISC00);
}
 
// Initialize INT1 (system reset)
void INT1_Init(void) {
	GICR |= (1 << INT1);
	MCUCR |= (1 << ISC11); // Falling edge
	MCUCR &= ~(1 << ISC10);
}

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
	
	if (display_state == OFF){
		enum DHT_Status_t status = DHT_GetStatus();
		if (status == DHT_Ok) {
			char temp_str[10], hum_str[10];
			dtostrf(avr_temp, 2, 1, temp_str);
			dtostrf(avr_hum, 2, 1, hum_str);
			sprintf(buffer, "T=%sC H=%s%%", temp_str, hum_str);
		} else {
			sprintf(buffer, "T=%dErr H=%dErr", status, status);
		}
	} else {
		sprintf(buffer,"OUT TEMP=%d%cC  ", (int)out_temp, degree_sysmbol);
	}
	LCD_String_xy(1,0,buffer);

	LCD_Char_xy(0, 11, rotate_state ? 3 : ' ');
	LCD_Char_xy(0, 12, fan_state ? 0 : ' ');
	LCD_Char_xy(0, 13, heater_state ? 2 : ' ');
	LCD_Char_xy(0, 14, humidifier_state ? 1 : ' ');
	LCD_String_xy(1, 15, (t->day - 1 < SETTER_PERIOD_DAYS) ? "S" : "H");
}

// Control devices based on temperature and humidity
void ControlDevices(void) {
	uint32_t current_time = t->second + t->minute * 60 + t->hour * 3600;
	double temp_setpoint = (t->day - 1 < SETTER_PERIOD_DAYS) ? TEMP_SETTER : TEMP_HATCHER;
	double hum_setpoint = (t->day - 1 < SETTER_PERIOD_DAYS) ? HUM_SETTER : HUM_HATCHER;

	ControlState new_heater_state = HysteresisControl(avr_temp, temp_setpoint, TEMP_HYSTERESIS, heater_state);
	if (prev_heater_state != new_heater_state)
	{
		heater_start_time = current_time;
		prev_heater_state = new_heater_state;
	}
	if (new_heater_state != heater_state)
	{
		heater_state = new_heater_state;
		if (heater_state && !fan_state && current_time - heater_start_time > 30)
		{
			heater_state = OFF;
		}
		DigitalWrite(HEATER_PIN, heater_state);
		LCD_Reset();
		LCD_Init();
	}
		
	humidifier_state = HysteresisControl(avr_hum, hum_setpoint, HUM_HYSTERESIS, humidifier_state);
	DigitalWrite(HUMIDIFIER_PIN, humidifier_state);
	
	ControlState new_fan_state = heater_state || humidifier_state ? ON : OFF;
	if (prev_fan_state != new_fan_state)
	{
		fan_start_time = current_time;
		prev_fan_state = new_fan_state;
	}
	if (fan_state != new_fan_state && current_time - fan_start_time > FAN_DELAY_START)
	{
		fan_state = new_fan_state;
		DigitalWrite(FAN_PIN, fan_state);
		LCD_Reset();
		LCD_Init();
	}
		
	vent_fan_state = (avr_temp > temp_setpoint + FAN_TRIGGER_TEMP || avr_hum > hum_setpoint + FAN_TRIGGER_HUM) ? ON : OFF;
	DigitalWrite(VENT_FAN_PIN, vent_fan_state);

	// check in setter period
	if (t->day - 1 < SETTER_PERIOD_DAYS && t->hour % ROTATION_INTERVAL_HOURS == 0 && t->minute == 0 && t->second == 0) {
		rotate_state = ON;
		rotation_start_time = current_time;
	}
	if (rotate_state != prev_rotate_state) {
		prev_rotate_state = rotate_state;
		DigitalWrite(ROTATION_PIN, rotate_state);
		LCD_Reset();
		LCD_Init();
	}
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

// Read LM35 sensor
void ReadADC(void) {
	lm35_index = (lm35_index + 1) % LM35_AVERAGE_COUNT;
	arr_avr_out_t[lm35_index] = ((ADC_Read(1) * 4.88) / 10.00);
	out_temp = average(arr_avr_out_t, LM35_AVERAGE_COUNT);
}

ISR(TIMER1_COMPA_vect) {
	static uint8_t control_counter = 0;
	static uint8_t display_counter = 0;
	static uint8_t sensor_counter = 0;
	
	control_counter++;
	display_counter++;
	sensor_counter++;
	
	if (control_counter >= 10) { // Check every 500ms (10 * 50ms)
		wdt_reset();
		read_rtc_flag = true;
		control_flag = true;
		control_counter = 0;
	}
	
	if (sensor_counter >= 20) { // Check every 1s (20 * 50ms)
		read_dht_flag = true;
		read_adc_flag = true;
		sensor_counter = 0;
	}

	if (display_counter >= 20) { // Update display every 1s (20 * 50ms)
		update_display_flag = true;
		display_counter = 0;
	}
}

// INT0 ISR: Stop rotation with debounce
ISR(INT0_vect) {
	uint32_t current_time = t->second + t->minute * 60 + t->hour * 3600;
	if (rotate_state && (current_time - rotation_start_time >= ROTATION_IGNORE_DELAY))
	{
		rotate_state = OFF;
	}
}

// INT1 ISR: Reset system with debounce
ISR(INT1_vect) {
	display_state = !display_state;
	if (!int1_flag) {
		int1_flag = true;
		int1_press_start = t->second + t->minute * 60 + t->hour * 3600;;
	}
}

int main(void) {
	LCD_Init();
    I2C_Init();
    DHT_Setup();
    LCD_SetupCustomChars();
	ADC_Init();
    Timer1_Init();
    INT0_Init();
    INT1_Init();

	wdt_enable(WDTO_2S); // Enable Watchdog with 2s timeout

	PinMode(FAN_PIN, Output);
    PinMode(ROTATION_PIN, Output);
    PinMode(HEATER_PIN, Output);
    PinMode(HUMIDIFIER_PIN, Output);
    PinMode(VENT_FAN_PIN, Output);
	
	t = RTC_Read_TimeDate();
	if (t->second >= 60 || t->minute >= 60 || t->hour >= 24) {
		uint8_t sec, min, hour, day;
		EEPROM_Read_Time(&sec, &min, &hour, &day);
		if (sec < 60 && min < 60 && hour < 24 && day < 255) {
			RTC_Clock_Write(sec, min, hour, hour_24);
			RTC_Calendar_Write(day, 1, 1, 19);
		} else {
			RTC_Clock_Write(0, 0, 0, hour_24);
			RTC_Calendar_Write(1, 1, 1, 19);
			EEPROM_Write_Time(0, 0, 0, 1);
		}
	}

    sei();
    set_sleep_mode(SLEEP_MODE_IDLE);

    while (1) {
		if (read_rtc_flag)
		{
			t = RTC_Read_TimeDate();
			read_rtc_flag = false;
		}
	    if (update_display_flag) {
		    UpdateDisplay();
		    update_display_flag = false;
	    }
	    if (control_flag) {
		    ControlDevices();
		    control_flag = false;
	    }
		if (read_adc_flag) {
			ReadADC();
			read_adc_flag = false;
		}
	    if (read_dht_flag) {
		    ReadDHT();
		    read_dht_flag = false;
	    }
		if (t->second == 0 && t->minute == 0 && t->hour % 2 == 0) {
			EEPROM_Write_Time(t->second, t->minute, t->hour, t->day);
		}
		if (int1_press_start != 0)
		{
			uint32_t current_time = t->second + t->minute * 60 + t->hour * 3600;
			if (DigitalRead(INT1_PIN) == Low) {
				if (current_time - int1_press_start >= BUTTON_HOLD_TIME) {
					RTC_Clock_Write(0, 0, 0, hour_24);
					RTC_Calendar_Write(1, 1, 1, 19);
					int1_flag = false;
					int1_press_start = 0;
				}
			} else {
				int1_flag = false;
				int1_press_start = 0;
			}
		}
	    sleep_mode();
    }
}