#ifndef DS18B20_H_
#define DS18B20_H_

#include <stdint.h>

typedef struct //Convenient structure for float-avoiding temperature holding
{
	uint8_t integer;
	uint8_t fraction;
} ds18b20_temp_t;

void ds18b20_convert_temp(void); //Sends CONVERT_T command
ds18b20_temp_t ds18b20_get_temp(void); //Retrieves scratchpad content and converts it to temperature (clever float-avoiding equations...)

#endif /* DS18B20_H_ */
