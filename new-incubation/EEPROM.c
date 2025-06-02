#include <avr/io.h>
#include <avr/eeprom.h>
#include "EEPROM.h"
#include "CONFIG.h"

void EEPROM_Write_Time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day) {
	eeprom_write_byte((uint8_t*)EEPROM_SEC_ADDR, sec);
	eeprom_write_byte((uint8_t*)EEPROM_MIN_ADDR, min);
	eeprom_write_byte((uint8_t*)EEPROM_HOUR_ADDR, hour);
	eeprom_write_byte((uint8_t*)EEPROM_DAY_ADDR, day);
}

void EEPROM_Read_Time(uint8_t* sec, uint8_t* min, uint8_t* hour, uint8_t* day) {
	*sec = eeprom_read_byte((uint8_t*)EEPROM_SEC_ADDR);
	*min = eeprom_read_byte((uint8_t*)EEPROM_MIN_ADDR);
	*hour = eeprom_read_byte((uint8_t*)EEPROM_HOUR_ADDR);
	*day = eeprom_read_byte((uint8_t*)EEPROM_DAY_ADDR);
}