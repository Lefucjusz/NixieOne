#ifndef BOOSTER_H_
#define BOOSTER_H_

#include <stdint.h>
#include "bool.h"

#define PI_CONTROLLER //Define control strategy, comment out to use old relay+integrator controller

#ifndef PI_CONTROLLER
#define BOOSTER_CHECK_RATE 20 //*2ms (for relay+integrator)
#else
#define BOOSTER_CHECK_RATE 2 //*2ms (for PI)
#endif

#define BOOSTER_FAIL TRUE
#define BOOSTER_OK FALSE

void booster_start(void); //Starts booster at low power
void booster_stop(void); //Stops booster
uint16_t booster_get_voltage(void); //Measures and returns output booster voltage
BOOL booster_control(uint16_t desired_v); //Controls booster work, THIS FUNCTION HAS TO BE CALLED EVERY BOOSTER_CHECK_RATE*2ms!!!

#endif /* BOOSTER_H_ */
