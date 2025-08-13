#ifndef _24C32_H_
#define _24C32_H_

#include "CONFIG.h"
#include <stdint.h>
#include "I2C_MASTER.h"

#define FALSE 0
#define TRUE 1

#define EEPROM_WRITE_ADDR 0xA0  // SLA+W (0x50 << 1)
#define EEPROM_READ_ADDR  0xA1  // SLA+R
#define EEPROM_WRITE_DELAY_MS 10

void EEPROM_Init(void);
uint8_t EEPROM_WriteByte(uint16_t address, uint8_t data);
uint8_t EEPROM_ReadByte(uint16_t address);
uint8_t EEPROM_WriteMultiple(uint16_t address, uint8_t* data, uint8_t length);
uint8_t EEPROM_ReadMultiple(uint16_t address, uint8_t* data, uint8_t length);  // ???? ???? ???? ?????? ????????

void EEPROM_Write_Time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day);
void EEPROM_Read_Time(uint8_t* sec, uint8_t* min, uint8_t* hour, uint8_t* day);

#endif