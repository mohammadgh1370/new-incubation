/*
 * I2C_Master_H_file.h
 * www.electronicwings.com
 */ 


#ifndef I2C_MASTER_H_FILE_H_					/* Define library H file if not defined */
#define I2C_MASTER_H_FILE_H_

#include "CONFIG.h"

#include <avr/io.h>								/* Include AVR std. library file */
#include <util/delay.h>							/* Include inbuilt defined Delay header file */
#include <math.h>								/* Include math function */
#define BITRATE(TWSR)	((F_CPU/F_SCL)-16)/(2*pow(4,(TWSR&((1<<TWPS0)|(1<<TWPS1))))) /* Define bit rate */

void I2C_Init();								/* I2C initialize function */
uint8_t  I2C_Start(char write_address);			/* I2C start function */
uint8_t  I2C_Repeated_Start(char read_address);	/* I2C repeated start function */
void I2C_Stop();								/* I2C stop function */
void I2C_Start_Wait(char write_address);		/* I2C start wait function */
uint8_t  I2C_Write(char data);					/* I2C write function */
int I2C_Read_Ack();								/* I2C read ack function */
int I2C_Read_Nack();							/* I2C read nack function */

#endif											/* I2C_MASTER_H_FILE_H_ */