#include "24c32.h"
#include <avr/io.h>
#include <util/delay.h>

static uint8_t EEPROM_WaitForWriteComplete(void) {
	uint8_t attempts = 100;
	while (attempts--) {
		if (I2C_Start(EEPROM_WRITE_ADDR) == 1) {
			I2C_Stop();
			return TRUE;
		}
		I2C_Stop();
		_delay_ms(1);
	}
	return FALSE;
}

void EEPROM_Init(void) {
	I2C_Init();
}

uint8_t EEPROM_WriteByte(uint16_t address, uint8_t data) {
	uint8_t status;

	status = I2C_Start(EEPROM_WRITE_ADDR);
	if (status != 1) return FALSE;

	status = I2C_Write(address >> 8);
	if (status != 0) return FALSE;

	status = I2C_Write(address & 0xFF);
	if (status != 0) return FALSE;

	status = I2C_Write(data);
	if (status != 0) return FALSE;

	I2C_Stop();

	if (!EEPROM_WaitForWriteComplete()) return FALSE;

	return TRUE;
}

uint8_t EEPROM_ReadByte(uint16_t address) {
	uint8_t status;
	uint8_t data = 0xFF;

	status = I2C_Start(EEPROM_WRITE_ADDR);
	if (status != 1) return data;

	status = I2C_Write(address >> 8);
	if (status != 0) {
		I2C_Stop();
		return data;
	}

	status = I2C_Write(address & 0xFF);
	if (status != 0) {
		I2C_Stop();
		return data;
	}

	status = I2C_Repeated_Start(EEPROM_READ_ADDR);
	if (status != 1) {
		I2C_Stop();
		return data;
	}

	data = I2C_Read_Nack();
	I2C_Stop();

	return data;
}

uint8_t EEPROM_WriteMultiple(uint16_t address, uint8_t* data, uint8_t length) {
	uint8_t status;
	uint8_t i;

	if (length > 32) return FALSE;  // ??????? ???? 32 ????

	status = I2C_Start(EEPROM_WRITE_ADDR);
	if (status != 1) return FALSE;

	status = I2C_Write(address >> 8);
	if (status != 0) return FALSE;

	status = I2C_Write(address & 0xFF);
	if (status != 0) return FALSE;

	for (i = 0; i < length; i++) {
		status = I2C_Write(data[i]);
		if (status != 0) return FALSE;
	}

	I2C_Stop();

	if (!EEPROM_WaitForWriteComplete()) return FALSE;

	return TRUE;
}

uint8_t EEPROM_ReadMultiple(uint16_t address, uint8_t* data, uint8_t length) {
	uint8_t status;
	uint8_t i;

	status = I2C_Start(EEPROM_WRITE_ADDR);
	if (status != 1) return FALSE;

	status = I2C_Write(address >> 8);
	if (status != 0) {
		I2C_Stop();
		return FALSE;
	}

	status = I2C_Write(address & 0xFF);
	if (status != 0) {
		I2C_Stop();
		return FALSE;
	}

	status = I2C_Repeated_Start(EEPROM_READ_ADDR);
	if (status != 1) {
		I2C_Stop();
		return FALSE;
	}

	for (i = 0; i < length - 1; i++) {
		data[i] = I2C_Read_Ack();
	}
	data[length - 1] = I2C_Read_Nack();

	I2C_Stop();

	return TRUE;
}

void EEPROM_Write_Time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day) {
	uint8_t time_data[4] = {sec, min, hour, day};
	EEPROM_WriteMultiple(0x0000, time_data, 4);
}

void EEPROM_Read_Time(uint8_t* sec, uint8_t* min, uint8_t* hour, uint8_t* day) {
	uint8_t time_data[4];
	if (EEPROM_ReadMultiple(0x0000, time_data, 4)) {
		*sec = time_data[0];
		*min = time_data[1];
		*hour = time_data[2];
		*day = time_data[3];
	} else {
		*sec = 0xFF;
		*min = 0xFF;
		*hour = 0xFF;
		*day = 0xFF;
	}
}