#include "buttons.h"
#include "bool.h"
#include <avr/io.h>

#define IDLE 0 //Definitions of states used by button state machine
#define DEBOUNCE 1
#define CLICKED 2
#define REPEAT 3

#define DEBOUNCE_DELAY 50 //*2ms

volatile uint16_t button_timer; //Program timer used to create delays

void buttons_init(void) //Initializes pins used to connect buttons
{
	BUTTON_DDR &= ~(HR_DAY_BUTTON | MIN_MONTH_BUTTON | MODE_BUTTON); //Config pins as inputs
	BUTTON_PORT |= (HR_DAY_BUTTON | MIN_MONTH_BUTTON | MODE_BUTTON); //Turn on internal pullups
}
void button_check(button_t* button) //Checks state of buttons and do proper action
{
	static uint8_t last_button, button_state; //Local variables used by button state machine
	BOOL button_pressed;

	if(last_button && last_button != button->button_mask) //Solves problems that may arise because of pressing more than one button at a time
		return;

	button_pressed = !(*button->button_pin & button->button_mask); //Is the button pressed?

	if(button_pressed) //If it is
	{
		if(button_state == IDLE) //And has so far been idle
		{
			button_state = DEBOUNCE; //Start debouncing action
			button_timer = DEBOUNCE_DELAY; //For a defined value of time
		}
		else if(button_state != IDLE && !button_timer) //If it wasn't in idle state and timer has been decremented to zero
		{
			if(button_state == DEBOUNCE) //If it was being debounced
			{
				if(button->click_func) //It's time to perform an action, check whether the function to be called has been specified
					button->click_func(); //If yes, call it
				button_state = CLICKED; //Change state of the button to clicked
				//Set timer for specified delay after which another action has to be performed if the button is not released earlier
				button_timer = (button->hold_delay_ms)/2; //(divided by 2 because of the hardware timer 2ms interval)
				last_button = button->button_mask; //Remember what button is being handled now
			}
			else if(button_state == CLICKED) //If it has already been clicked
				button_state = REPEAT; //Change state of the button to repeat
			else if(button_state == REPEAT) //If it is in repeat state
			{
				button_timer = (button->repeat_delay_ms)/2; //Repeat the action in a specified interval of time (divided by 2 because of the same reason)
				if(button->hold_func) //Check whether the function to be called has been specified
					button->hold_func(); //If yes, call it
			}
		}
	}
	else if(button_state == CLICKED || button_state == REPEAT) //If it isn't pressed and is in clicked or repeat state
	{
		button_state = IDLE; //Change its state to idle
		last_button = 0; //And forget what button was being handled
	}
}
