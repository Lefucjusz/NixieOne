#include "photoresistor.h"

#include <avr/io.h>

#define PHOTORESISTOR_PIN (1<<PC0)
#define PHOTORESISTOR_DDR DDRC

void photoresistor_init(void) //Initializes ADC for ambient light measurements
{
	PHOTORESISTOR_DDR &= ~PHOTORESISTOR_PIN; //Photoresistor pin as input
	ADMUX |= (1<<REFS0) | (1<<ADLAR); //Reference from AVCC, left adjust result
	ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0); //Turn on ADC, single measurement mode, prescaler 32
}

uint8_t photoresistor_get_value(void) //Performs measurement and returns measured value
{
	ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0)); //Set ADC multiplexer to input 0
	ADCSRA |= (1<<ADSC); //Start measurement
	while(ADCSRA & (1<<ADSC)); //Wait for it to end
	return ADCH; //Return read value
}
