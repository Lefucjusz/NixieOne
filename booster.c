#include "booster.h"

#include <avr/io.h>

#define MOSFET_PIN (1<<PB3)
#define MOSFET_PORT PORTB
#define MOSFET_DDR DDRB
#define FEEDBACK_PIN (1<<PC0)
#define FEEDBACK_PORT PORTC
#define FEEDBACK_DDR DDRC

#define VOLTAGE_EPS 2 //Maximal voltage error

void booster_start(void) //Starts booster at low power
{
	//PWM
	MOSFET_DDR |= MOSFET_PIN; //PWM output as output
	MOSFET_PORT |= MOSFET_PIN; //Turn off MOSFET (inverted because of inverting driver)
	TCCR2 |= (1<<WGM21) | (1<<WGM20) | (1<<COM21) | (1<<COM20) | (1<<CS20);
	//Timer 2, Fast PWM, Set OC2 on compare (because of inverting driver), no prescaler -> 15.625kHz PWM
	OCR2 = 30; //Start at some low value

	//ADC
	FEEDBACK_DDR &= ~FEEDBACK_PIN; //Feedback pin as input
	ADMUX |= (1<<REFS0) | (1<<ADLAR); //Reference from AVCC, left adjust result
	ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0); //Turn on ADC, single measurement mode, prescaler 32
}
void booster_stop(void) //Stops booster
{
	OCR2 = 0; //Minimum PWM width
	MOSFET_PORT |= MOSFET_PIN; //Turn off MOSFET not to make short circuit
}
uint16_t booster_get_voltage(void) //Measures and returns output booster voltage
{
	ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0)); //Set ADC multiplexer to input 0
	ADCSRA |= (1<<ADSC); //Start measurement
	while(ADCSRA & (1<<ADSC)); //Wait for it to end
	//Equation for calculating voltage, simplified so that it doesn't use floats or create big values
	//It should be (((R1+R2)/R2)*VREF/256)*ADCH
	//R1=680k; R2=10k; VREF=5V; so it's 690*5/2560=~1.35, 4/3=~1.33 is good approximation
	return ((4*(uint16_t)ADCH)/3);
}
BOOL booster_control(uint16_t desired_v) //Controls booster work, THIS FUNCTION HAS TO BE CALLED EVERY BOOSTER_CHECK_RATE*2ms!!!
{
	register uint16_t current_v; //Current output voltage value
	current_v = booster_get_voltage(); //Get that value
	if(current_v < desired_v - VOLTAGE_EPS) //If it's lower than should be
	{
		OCR2++; //Increase PWM
		if(OCR2 > 240) //If over 94% - something has failed
		{
			booster_stop(); //Stop booster
			return BOOSTER_FAIL; //Report failure
		}
	}
	else if(current_v > desired_v + VOLTAGE_EPS) //If it's higher than it should be
	{
		OCR2--; //Decrease PWM
		if(OCR2 == 0) //If OCR had to be decreased to zero - something has failed
		{
			booster_stop(); //Stop booster
			return BOOSTER_FAIL; //Report failure
		}
	}
	return BOOSTER_OK; //If everything was ok - report proper work
}
