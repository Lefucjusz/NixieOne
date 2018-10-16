#include "i2c.h"
#include <avr/io.h>

#define TIMEOUT_VALUE 250

uint8_t timeout_timer; //variable used to prevent stucking in the infinite loop if the transmission has failed

void i2c_init(void) //Sets SCK speed to 100kHz, equation: SCK = F_CPU/(16 + 2*TWBR*Prescaler)
{
	TWBR = 12; //Write value to bitrate register
	TWSR &= ~((1<<TWPS1) | (1<<TWPS0)); //Set prescaler to 1
}
void i2c_start(void) //Generates start condition on I2C bus
{
     TWCR = (1<<TWINT) | (1<<TWEN) |(1<<TWSTA); //Generate start condition
     timeout_timer = TIMEOUT_VALUE; //Init timer
     while (!(TWCR & (1<<TWINT)) && (timeout_timer--)); //Wait for TWI interface to finish (or the time to end)
}
void i2c_stop(void) //Generates stop condition on I2C bus
{
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO); //Generate stop condition
    timeout_timer = TIMEOUT_VALUE; //Init timer
    while (!(TWCR & (1<<TWSTO)) && (timeout_timer--)); //Wait for TWI interface to finish (or the time to end)
}
void i2c_write_byte(uint8_t value) //Writes byte to I2C device
{
     TWDR = value; //Write byte to be sent to TWDR register
     TWCR = (1<<TWINT) | (1<<TWEN); //Start transmission
     timeout_timer = TIMEOUT_VALUE; //Init timer
     while (!(TWCR & (1<<TWINT)) && (timeout_timer--)); //Wait for TWI interface to finish (or the time to end)
}
uint8_t i2c_read_byte(BOOL ack) //Reads byte from I2C device
{
    TWCR = (1<<TWINT) | (ack<<TWEA) | (1<<TWEN); //Read byte from I2C device
    timeout_timer = TIMEOUT_VALUE; //Init timer
    while (!(TWCR & (1<<TWINT)) && (timeout_timer--)); //Wait for TWI interface to finish (or the time to end)
    return TWDR; //Return read byte
}
