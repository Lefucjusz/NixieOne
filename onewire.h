#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include <stdint.h>
#include "bool.h"

BOOL ow_reset(void); //Resets 1wire bus
void ow_write_byte(uint8_t value); //Writes byte to 1wire device
uint8_t ow_read_byte(void); //Reads byte from 1wire device

#endif /* 1WIRE_H_ */
