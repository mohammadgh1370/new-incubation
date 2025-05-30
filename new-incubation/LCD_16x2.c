#include "LCD_16x2.h"						/* Include LCD header file */
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

void LCD_Reset(void){
	LCD_PORT = 0x00;
}

void LCD_Command(unsigned char cmd )
{
	LCD_PORT = (LCD_PORT & 0x0F) | (cmd & 0xF0); /* sending upper nibble */
	LCD_PORT &= ~ (1<<LCD_RS);				/* RS=0, command reg. */
	LCD_PORT |= (1<<LCD_EN);				/* Enable pulse */
	_delay_us(1);
	LCD_PORT &= ~ (1<<LCD_EN);

	_delay_us(200);

	LCD_PORT = (LCD_PORT & 0x0F) | (cmd << 4);  /* sending lower nibble */
	LCD_PORT |= (1<<LCD_EN);
	_delay_us(1);
	LCD_PORT &= ~ (1<<LCD_EN);
	_delay_ms(2);
}

void LCD_Char(char data)
{
	LCD_PORT = (LCD_PORT & 0x0F) | (data & 0xF0); /* sending upper nibble */
	LCD_PORT |= (1<<LCD_RS);				/* RS=1, data reg. */
	LCD_PORT|= (1<<LCD_EN);
	_delay_us(1);
	LCD_PORT &= ~ (1<<LCD_EN);

	_delay_us(200);

	LCD_PORT = (LCD_PORT & 0x0F) | (data << 4); /* sending lower nibble */
	LCD_PORT |= (1<<LCD_EN);
	_delay_us(1);
	LCD_PORT &= ~ (1<<LCD_EN);
	_delay_ms(2);
}

void LCD_Init (void)					/* LCD Initialize function */
{
	LCD_DIR = 0xFF;						/* Make LCD command port direction as o/p */
	_delay_ms(20);						/* LCD Power ON delay always >15ms */
	
	LCD_Command(0x33);
	LCD_Command(0x32);		    		/* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);              	/* Use 2 line and initialize 5*7 matrix in (4-bit mode)*/
	LCD_Command(0x0c);              	/* Display on cursor off*/
	LCD_Command(0x06);              	/* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              	/* Clear display screen*/
	_delay_ms(2);
	LCD_Command (0x80);					/* Cursor 1st row 0th position */
}

void LCD_String (char *str)				/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)				/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);		/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);		/* Command of first row and required position<16 */
	LCD_String(str);					/* Call LCD string function */
}

void LCD_Char_xy (char row, char pos, char str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);		/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);		/* Command of first row and required position<16 */
	LCD_Char(str);						/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);					/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);					/* Cursor 1st row 0th position */
}

void LCD_Custom_Char(const uint8_t *pc, uint8_t char_code){
	uint8_t a, pcc;
	uint16_t i;
	a=(char_code<<3)|0x40;
	for (i=0; i<8; i++){
		pcc=pgm_read_byte(&pc[i]);
		LCD_Command(a++);
		LCD_Char(pcc);
	}
}

void LCD_progressBar(uint8_t progress, uint8_t maxprogress)
{
	uint8_t bar[8] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,0x1F };
	uint8_t nbar[8] = { 0x0, 0x1F, 0x0, 0x0, 0x0, 0x0, 0x1F,0x0 };
	char i;
	
	LCD_Custom_Char(bar, 0);
	LCD_Custom_Char(nbar, 1);
	LCD_Clear();
	for(i=0;i<maxprogress;i++)	/* Function will send data 1 to 8 to lcd */
	{
		LCD_GotoXY(0,i);
		LCD_Char(0);
		_delay_ms(40);
	}
	for(i=progress;i<maxprogress;i++)	/* Function will send data 1 to 8 to lcd */
	{
		LCD_Char(1);
		_delay_ms(10);
	}
}

void LCD_Loading(uint8_t maxprogress)
{
	uint8_t bar[8] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,0x1F };
	uint8_t nbar[8] = { 0x0, 0x1F, 0x0, 0x0, 0x0, 0x0, 0x1F,0x0 };
	char i;
	
	LCD_Custom_Char(bar, 0);
	LCD_Custom_Char(nbar, 1);
	LCD_Clear();
	LCD_String_xy(0,4,"Loading");
	for(i=0;i<=maxprogress;i++)	/* Function will send data 1 to 8 to lcd */
	{
		LCD_GotoXY(1,i);
		LCD_Char(1);
	}
	
	for(i=0;i<maxprogress;i++)	/* Function will send data 1 to 8 to lcd */
	{
		LCD_GotoXY(1,i);
		LCD_Char(0);
		_delay_ms(40);
	}
	LCD_Clear();
	LCD_String_xy(0,4,"welcome");
	_delay_ms(500);
}

void LCD_GotoXY(uint8_t x, uint8_t y)	//Cursor to X Y position
{
	
	if (x == 0 && y<16)
	LCD_Command((y & 0x0F)|0x80);		/* Command of first row and required position<16 */
	else if (x == 1 && y<16)
	LCD_Command((y & 0x0F)|0xC0);		/* Command of first row and required position<16 */
	
}

void LCD_Shift_Left_String(uint8_t r,char *str)	//Scroll n of characters Right
{
	char string[40];
	uint8_t i;
	uint8_t j;
	for (i=0;str[i]!=0;i++)
	{
		for (j=0;j<40;j++)
		{
			string[j]=str[i+j];
		}
		LCD_String_xy(r,0,string);
		LCD_Command(0x1E);
		_delay_ms(5);
	}
	i=0;
	j=0;
}

void LCD_Shift_Left(uint8_t n)	//Scroll n of characters Right
{
	for (uint8_t i=0;i<n;i++)
	{
		LCD_Command(0x1E);
	}
}

void LCD_Shift_Right(uint8_t n)	//Scroll n of characters Left
{
	for (uint8_t i=0;i<n;i++)
	{
		LCD_Command(0x18);
	}
}

void LCD_Cursor_Blink_Position (char row, char col)		/* blink LCD cursor */
{
	if (row == 0 && col<16){
		LCD_Command((col & 0x0F)|0x80);
		LCD_Command(0x0D);
		}else if (row == 1 && col<16){
		LCD_Command((col & 0x0F)|0xc0);
		LCD_Command(0x0D);
	}
}

void LCD_Cursor_Left(uint8_t n)			//Moves cursor by n positions left
{
	for (uint8_t i=0;i<n;i++)
	{
		LCD_Command(0x10);
	}
}

void LCD_Cursor_Right(uint8_t n)	//Moves cursor by n positions left
{
	for (uint8_t i=0;i<n;i++)
	{
		LCD_Command(0x14);
	}
}

void LCD_Cursor_On(void)			//displays LCD cursor
{
	LCD_Command(0x0E);
}

void LCD_Cursor_OnBlink(void)		//displays LCD blinking cursor
{
	LCD_Command(0x0F);
}

void LCD_Cursor_OFF(void)			//turns OFF cursor
{
	LCD_Command(0x0C);
}

void LCD_Blank(void)				//blanks LCD
{
	LCD_Command(0x08);
}

void LCD_Visible(void)			//Shows LCD
{
	LCD_Command(0x0C);
}