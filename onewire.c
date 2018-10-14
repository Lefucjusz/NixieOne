#include "onewire.h"

#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>


#define OW_PIN (1<<PD1)
#define OW_PORT PORTD
#define OW_DDR DDRD
#define OW_READ PIND

#define OW_LOW OW_PORT &= ~OW_PIN
#define OW_IN OW_DDR &= ~OW_PIN
#define OW_OUT OW_DDR |= OW_PIN

BOOL ow_reset(void) //Resets 1wire bus
{
	BOOL presence = TRUE;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		OW_LOW;
		OW_OUT;
		_delay_us(480);
		OW_IN;
		_delay_us(70);
		presence = (OW_READ & OW_PIN);
		_delay_us(410);
	}
	return presence;
}

void ow_write_byte(uint8_t value) //Writes byte to 1wire device
{
	uint8_t i;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		for(i = 0; i < 8; i++)
		{
			OW_OUT;
			if(value & 0x01)
			{
				_delay_us(6);
				OW_IN;
				_delay_us(64);
			}
			else
			{
				_delay_us(60);
				OW_IN;
				_delay_us(10);
			}
			value >>= 1;
		}
	}
}

uint8_t ow_read_byte(void) //Reads byte from 1wire device
{
	uint8_t i, value = 0;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		for(i = 0; i < 8; i++)
		{
			value >>= 1;
			OW_OUT;
			_delay_us(6);
			OW_IN;
			_delay_us(9);
			if(OW_READ & OW_PIN)
				value |= 0x80;
			_delay_us(55);
		}
	}
	return value;
}
