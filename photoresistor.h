#ifndef PHOTORESISTOR_H_
#define PHOTORESISTOR_H_

#include <stdint.h>

void photoresistor_init(void); //Initializes ADC for ambient light measurements
uint8_t photoresistor_get_value(void); //Performs measurement and returns measured value

#endif /* PHOTORESISTOR_H_ */
