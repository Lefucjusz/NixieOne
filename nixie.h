#ifndef NIXIE_H_
#define NIXIE_H_

#include <stdint.h>
#include "bool.h"

#define NIXIE_1 1
#define NIXIE_2 2
#define NIXIE_3 3
#define NIXIE_4 4

#define NIXIE_OFF 10

void nixie_init(void); //Initializes ports used by NIXIE
void nixie_show_value(uint8_t digit, BOOL colon, uint8_t tube); //Displays digit on chosen NIXIE

#endif /* NIXIE_H_ */
