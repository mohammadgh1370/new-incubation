#include "DS1307.h"

struct tm _tm;
const int           DaysToMonth[13] =     {  0,31,59,90,120,151,181,212,243,273,304,334,365};
const unsigned int  g_days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const unsigned int  j_days_in_month[12] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};

void RTC_Clock_Write(uint8_t _hour, uint8_t _minute, uint8_t _second, uint8_t _AMPM)
{
	_hour |= DecimalToBCD(_AMPM);
	I2C_Start(Device_Write_address);	/* Start I2C communication with RTC */
	I2C_Write(0);						/* Write 0 address for second */
	I2C_Write(DecimalToBCD(_second));	/* Write second on 00 location */
	I2C_Write(DecimalToBCD(_minute));	/* Write minute on 01(auto increment) location */
	I2C_Write(DecimalToBCD(_hour));		/* Write hour on 02 location */
	I2C_Stop();							/* Stop I2C communication */
}

void RTC_Calendar_Write(uint8_t _day, uint8_t _date, uint8_t _month, uint8_t _year)
{
	I2C_Start(Device_Write_address);	/* Start I2C communication with RTC */
	I2C_Write(3);						/* Write 3 address for day */
	I2C_Write(DecimalToBCD(_day));		/* Write day on 03 location */
	I2C_Write(DecimalToBCD(_date));		/* Write date on 04 location */
	I2C_Write(DecimalToBCD(_month));	/* Write month on 05 location */
	I2C_Write(DecimalToBCD(_year));		/* Write year on 06 location */
	I2C_Stop();							/* Stop I2C communication */
}

void RTC_Clock_Read(uint8_t *_hour, uint8_t *_minute, uint8_t *_second)
{
	I2C_Start(Device_Write_address);			/* Start I2C communication with RTC */
	I2C_Write(0);								/* Write address to read */
	I2C_Repeated_Start(Device_Read_address);	/* Repeated start with device read address */
	*_second = BCDToDecimal(I2C_Read_Ack());	/* Read second */
	*_minute = BCDToDecimal(I2C_Read_Ack());	/* Read minute */
	*_hour = BCDToDecimal(I2C_Read_Nack());		/* Read hour with Nack */
	I2C_Stop();									/* Stop i2C communication */
}

void RTC_Calendar_Read(uint8_t *_dayofweek, uint8_t *_day, uint8_t *_month, uint8_t *_year)
{
	I2C_Start(Device_Write_address);			/* Start I2C communication with RTC */
	I2C_Write(3);								/* Write address to read */
	I2C_Repeated_Start(Device_Read_address);	/* Repeated start with device read address */
	*_dayofweek = BCDToDecimal(I2C_Read_Ack());	/* Read dayofweek */
	*_day = BCDToDecimal(I2C_Read_Ack());		/* Read day */
	*_month = BCDToDecimal(I2C_Read_Ack());		/* Read month */
	*_year = BCDToDecimal(I2C_Read_Nack());		/* Read the year with Nack */
	I2C_Stop();									/* Stop i2C communication */
}

struct tm* RTC_Read_TimeDate(void)
{
	uint8_t second,minute,hour,dayofweek,day,month,year;

	RTC_Clock_Read(&hour,&minute,&second);
	RTC_Calendar_Read(&dayofweek,&day,&month,&year);

	_tm.second = second;
	_tm.minute = minute;
	_tm.hour = hour;
	_tm.dayofweek = dayofweek;
	_tm.day = day;
	_tm.month = month;
	_tm.year = year+2000;

	return &_tm;
}

uint8_t DecimalToBCD (uint8_t decimalByte)
{
	return (((decimalByte / 10) << 4) | (decimalByte % 10));
}

uint8_t BCDToDecimal (uint8_t bcdByte)
{
	return (((bcdByte& 0xF0) >> 4) * 10) + (bcdByte& 0x0F);
}

void Miladi2Shamsi(int *j_y, int *j_m, int *j_d, int  g_y, int  g_m, int  g_d) // Miladi To shamsi
{
	unsigned  long gy, gm, gd;
	unsigned long jy, jm, jd;
	unsigned  long g_day_no, j_day_no;
	unsigned long j_np;
	unsigned long i;
	
	if(g_y<2000 || g_y>2100) g_y=2012;
	if(g_m==0   || g_m>12)   g_m=1;
	if(g_d==0   || g_d>31)   g_d=1;

	gy = g_y-1600;
	gm = g_m-1;
	gd = g_d-1;

	g_day_no = 365*gy+(gy+3)/4-(gy+99)/100+(gy+399)/400;
	for (i=0;i<gm;++i)
	g_day_no += g_days_in_month[i];
	if (gm>1 && ((gy%4==0 && gy%100!=0) || (gy%400==0)))
	/* leap and after Feb */
	++g_day_no;
	g_day_no += gd;

	j_day_no = g_day_no-79;

	j_np = j_day_no / 12053;
	j_day_no %= 12053;

	jy = 979+33*j_np+4*(j_day_no/1461);
	j_day_no %= 1461;

	if (j_day_no >= 366)
	{
		jy += (j_day_no-1)/365;
		j_day_no = (j_day_no-1)%365;
	}

	for (i = 0; i < 11 && j_day_no >= j_days_in_month[i]; ++i)
	{
		j_day_no -= j_days_in_month[i];
	}

	jm = i+1;
	jd = j_day_no+1;
	*j_y = jy;
	*j_m = jm;
	*j_d = jd;
}

void Shamsi2Miladi(int *g_y,int *g_m,int *g_d,int  j_y,int  j_m,int  j_d)// shamsi To Mildai
{
	unsigned long gy, gm, gd;
	unsigned long jy, jm, jd;
	unsigned long g_day_no, j_day_no;
	unsigned long leap;

	unsigned long i;

	jy = j_y-979;
	jm = j_m-1;
	jd = j_d-1;

	j_day_no = 365*jy + (jy/33)*8 + (jy%33+3)/4;
	for (i=0; i < jm; ++i)
	j_day_no += j_days_in_month[i];

	j_day_no += jd;

	g_day_no = j_day_no+79;

	gy = 1600 + 400*(g_day_no/146097); /* 146097 = 365*400 + 400/4 - 400/100 + 400/400 */
	g_day_no = g_day_no % 146097;

	leap = 1;
	if (g_day_no >= 36525) /* 36525 = 365*100 + 100/4 */
	{
		g_day_no--;
		gy += 100*(g_day_no/36524); /* 36524 = 365*100 + 100/4 - 100/100 */
		g_day_no = g_day_no % 36524;

		if (g_day_no >= 365)
		g_day_no++;
		else
		leap = 0;
	}

	gy += 4*(g_day_no/1461); /* 1461 = 365*4 + 4/4 */
	g_day_no %= 1461;

	if (g_day_no >= 366) {
		leap = 0;

		g_day_no--;
		gy += g_day_no/365;
		g_day_no = g_day_no % 365;
	}

	for (i = 0; g_day_no >= g_days_in_month[i] + (i == 1 && leap); i++)
	g_day_no -= (unsigned long)g_days_in_month[i] + (i == 1 && leap);
	gm = i+1;
	gd = g_day_no+1;
	
	*g_y = gy;
	*g_m = gm;
	*g_d = gd;
}
