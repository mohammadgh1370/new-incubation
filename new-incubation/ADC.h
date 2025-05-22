#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <util/delay.h>

void ADC_Init(void);
int ADC_Read(char channel);

#endif
