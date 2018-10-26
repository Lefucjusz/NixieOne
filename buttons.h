#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>

#define BUTTON_READ PINB
#define BUTTON_PORT PORTB
#define BUTTON_DDR DDRB

#define HR_DAY_BUTTON (1<<PB0)
#define MIN_MONTH_BUTTON  (1<<PB1)
#define MODE_BUTTON  (1<<PB2)

extern volatile uint16_t button_timer; //Program timer used to create delays, declaration in buttons.c

typedef struct //Convenient structure for holding button parameters
{
	volatile uint8_t *button_pin;
	uint8_t button_mask;
	uint16_t repeat_delay_ms;
	uint16_t hold_delay_ms;
	void (*click_func)(void);
	void (*hold_func)(void);
} button_t;

void buttons_init(void); //Initializes pins used to connect buttons
void button_check(button_t* button); //Checks state of buttons and do proper action

#endif /* BUTTONS_H_ */
