#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

void EEPROM_Write_Time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day);
void EEPROM_Read_Time(uint8_t* sec, uint8_t* min, uint8_t* hour, uint8_t* day);

#endif