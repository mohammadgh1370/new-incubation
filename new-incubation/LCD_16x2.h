#ifndef LCD_16x2_H_						/* Define library H file if not defined */
#define LCD_16x2_H_

#include "CONFIG.h"

#include <avr/io.h>						/* Include AVR std. library file */
#include <util/delay.h>					/* Include Delay header file */
#include <inttypes.h>

#define LCD_Dir DDRA					/* Define LCD data port direction */
#define LCD_Port PORTA					/* Define LCD data port */
#define EN PA2							/* Define Enable signal pin */
#define RW PA1							/* Define Read/Write signal pin */
#define RS PA0							/* Define Register Select (data reg./command reg.) signal pin */

#define PROGRESSPIXELS_PER_CHAR	6
 
void LCD_Reset(void);
void LCD_Command (unsigned char);		/* LCD command write function */
void LCD_Char (char);					/* LCD data write function */
void LCD_Init (void);					/* LCD Initialize function */
void LCD_String (char*);				/* Send string to LCD function */
void LCD_String_xy (char,char,char*);	/* Send row, position and string to LCD function */
void LCD_Char_xy (char,char,char);		/* Send row, position and char to LCD function */
void LCD_Clear (void);					/* LCD clear function */
void LCD_Custom_Char(const uint8_t *pc, uint8_t char_code);
void LCD_GotoXY(uint8_t x, uint8_t y);

void LCD_progressBar(uint8_t progress, uint8_t maxprogress);
void LCD_Loading(uint8_t);
void LCD_Shift_Left_String(uint8_t r,char *str);
void LCD_Cursor_Blink_Position(char row, char col);

void LCD_Shift_Right(uint8_t n);	//shift by n characters Right
void LCD_Shift_Left(uint8_t n);		//shift by n characters Left
void LCD_Cursor_On(void);			//Underline cursor ON
void LCD_Cursor_OnBlink(void);		//Underline blinking cursor ON
void LCD_Cursor_OFF(void);			//Cursor OFF
void LCD_Blank(void);				//LCD blank but not cleared
void LCD_Visible(void);				//LCD visible
void LCD_Cursor_Left(uint8_t n);	//Shift cursor left by n
void LCD_Cursor_Right(uint8_t n);	//shift cursor right by n

#endif								/* LCD_20x4_H_FILE_H_ */