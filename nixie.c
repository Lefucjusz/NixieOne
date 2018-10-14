#include "nixie.h"

#include <avr/io.h>

#define ANODE_PORT PORTB
#define ANODE_DDR DDRB
#define ANODE_1 (1<<PB4)
#define ANODE_2 (1<<PB5)
#define ANODE_3 (1<<PB6)
#define ANODE_4 (1<<PB7)

#define CATHODE_PORT PORTD
#define CATHODE_DDR DDRD
#define CATHODE_BCD_A (1<<PD4)
#define CATHODE_BCD_B (1<<PD5)
#define CATHODE_BCD_C (1<<PD6)
#define CATHODE_BCD_D (1<<PD7)

#define COLON_PORT PORTD
#define COLON_DDR DDRD
#define COLON_PIN (1<<PD3)

void nixie_init(void) //Initializes ports used by NIXIE
{
	ANODE_PORT &= ~(ANODE_1 | ANODE_2 | ANODE_3 | ANODE_4); //Turn off all anodes
	CATHODE_PORT &= ~(CATHODE_BCD_A | CATHODE_BCD_B | CATHODE_BCD_C | CATHODE_BCD_D); //Turn off all cathodes
	COLON_PORT &= ~COLON_PIN; //Turn off colon
	ANODE_DDR |= ANODE_1 | ANODE_2 | ANODE_3 | ANODE_4; //Set anode pins as output
	CATHODE_DDR |= CATHODE_BCD_A | CATHODE_BCD_B | CATHODE_BCD_C | CATHODE_BCD_D; //Set cathode pins as output
	COLON_DDR |= COLON_PIN; //Set colon pin as output
}

void nixie_show_value(uint8_t digit, BOOL decimal_point, uint8_t tube) //Displays digit on chosen NIXIE
{
	ANODE_PORT &= ~(ANODE_1 | ANODE_2 | ANODE_3 | ANODE_4); //Turn off all anodes
	if(digit > 9) //If digit > 9...it's not digit anymore, leave
	{
		return;
	}
	CATHODE_PORT = ((CATHODE_PORT & 0x07) | (digit<<4) | (decimal_point<<PD3)); //Mask 3 LSB's shift values to be displayed to proper places
	switch(tube) //Turn on chosen tube's anode
	{
		case 1:
			ANODE_PORT |= ANODE_1;
		break;
		case 2:
			ANODE_PORT |= ANODE_2;
		break;
		case 3:
			ANODE_PORT |= ANODE_3;
		break;
		case 4:
			ANODE_PORT |= ANODE_4;
		break;
		default:
		break;//If anode out of range do nothing, leave display turned off by the first command
	}
}
