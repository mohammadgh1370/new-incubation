#include "ADC.h"

void ADC_Init(void)
{
	DDRA = 0x00;	        /* Make ADC port as input */
	ADCSRA = 0x87;          /* Enable ADC, with freq/128  */
	ADMUX = 0x40;           /* Vref: AVcc, ADC channel: 0 */
}

int ADC_Read(char channel)
{
	ADMUX = 0x40 | (channel & 0x07);   /* set input channel to read */
	ADCSRA |= (1 << ADSC);             /* Start ADC conversion */
	while (!(ADCSRA & (1 << ADIF)));   /* Wait for conversion to complete */
	ADCSRA |= (1 << ADIF);             /* Clear interrupt flag */
	_delay_ms(1);                      /* Small delay */
	return ADCW;                       /* Return ADC result */
}
