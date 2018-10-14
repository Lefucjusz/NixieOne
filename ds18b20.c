#include "ds18b20.h"
#include "onewire.h"

#define CONVERT_T 0x44
#define READ_SCRATCHPAD 0xBE
#define SKIP_ROM 0xCC

void ds18b20_convert_temp(void) //Sends CONVERT_T command
{
	ow_reset();
	ow_write_byte(SKIP_ROM);
	ow_write_byte(CONVERT_T);
}

ds18b20_temp_t ds18b20_get_temp(void) //Retrieves scratchpad content and converts it to temperature (clever float-avoiding equations...)
{
	uint8_t lsb, msb;
	ds18b20_temp_t temp;
	ow_reset();
	ow_write_byte(SKIP_ROM);
	ow_write_byte(READ_SCRATCHPAD);
	lsb = ow_read_byte();
	msb = ow_read_byte();
	temp.integer = (uint8_t)( ((msb << 4) | (lsb >> 4)) & 0x7F);
	temp.fraction = (uint8_t)( ((lsb & 0x0F) * 625) / 100);
	return temp;
}
