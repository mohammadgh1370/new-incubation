#ifndef DS1307_H_
#define DS1307_H_

#include <avr/io.h>
#include <stdbool.h>
#include "I2C_MASTER.h"


#define	AM_PM					0x20
#define	Device_Write_address	0xD0			/* Define RTC DS1307 slave address for write operation */
#define	Device_Read_address		0xD1			/* Make LSB bit high of slave address for read operation */
#define	hour_12_AM				40
#define	hour_12_PM				60
#define	hour_24					0

struct tm {
	volatile int second;		// 0 to 59
	volatile int minute;		// 0 to 59
	volatile int hour;			// 0 to 23
	volatile int day;			// 1 to 31
	volatile int month;			// 1 to 12
	volatile int year;			// year-99
	volatile int dayofweek;		// 1-7
};

void RTC_Clock_Write(uint8_t _hour, uint8_t _minute, uint8_t _second, uint8_t _AMPM);
void RTC_Calendar_Write(uint8_t _day, uint8_t _date, uint8_t _month, uint8_t _year);

void RTC_Clock_Read(uint8_t* _hour, uint8_t* _minute, uint8_t* _second);
void RTC_Calendar_Read(uint8_t* _dayofweek, uint8_t* _day, uint8_t* _month, uint8_t* _year);

struct tm* RTC_Read_TimeDate(void);

void Miladi2Shamsi(int *j_y, int *j_m, int *j_d, int  g_y, int  g_m, int  g_d);
void Shamsi2Miladi(int *g_y, int *g_m, int *g_d, int  j_y, int  j_m, int  j_d);

uint8_t DecimalToBCD (uint8_t decimalByte);
uint8_t BCDToDecimal (uint8_t bcdByte);

#endif