#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
#include "bool.h"

void i2c_init(void); //Sets SCK speed to 100kHz, equation: SCK = F_CPU/(16 + 2*TWBR*Prescaler)
void i2c_start(void); //Generate start condition on I2C bus
void i2c_stop(void); //Generates stop condition on I2C bus
void i2c_write_byte(uint8_t value); //Writes byte to I2C device
uint8_t i2c_read_byte(BOOL ack); //Reads byte from I2C device

#endif /* I2C_H_ */
