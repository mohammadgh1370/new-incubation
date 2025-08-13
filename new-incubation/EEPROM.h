#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

#define EEPROM_24C32_WRITE 0xA0
#define EEPROM_24C32_READ  0xA1
#define EEPROM_BASE_ADDR   0x0000

void EEPROM_Write_Time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day);
void EEPROM_Read_Time(uint8_t* sec, uint8_t* min, uint8_t* hour, uint8_t* day);

void EEPROM_External_Write_Time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day);
void EEPROM_External_Read_Time(uint8_t* sec, uint8_t* min, uint8_t* hour, uint8_t* day);

#endif