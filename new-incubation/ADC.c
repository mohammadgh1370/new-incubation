#include "ADC.h"

void ADC_Init(void)
{
	DDRA = 0x00;	        /* Make ADC port as input */
	ADCSRA = 0x87;          /* Enable ADC, with freq/128  */
	ADMUX = 0x40;           /* Vref: AVcc, ADC channel: 0 */
}

int ADC_Read(char channel) {
	uint16_t sum = 0;
	for (uint8_t i = 0; i < 8; i++) {
		ADMUX = 0x40 | (channel & 0x07);
		ADCSRA |= (1 << ADSC);
		while (!(ADCSRA & (1 << ADIF)));
		ADCSRA |= (1 << ADIF);
		_delay_us(50);
		sum += ADCW;
	}
	return sum / 8;
}
