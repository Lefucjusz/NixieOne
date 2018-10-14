#include "ds1307.h"
#include "i2c.h"
#include "bool.h"

#define DS1307_WR_ADDR 0xD0
#define DS1307_RD_ADDR 0xD1

#define ACK TRUE
#define NACK FALSE

inline static uint8_t dec2bcd (uint8_t dec) //Quite self-explanatory, isn't it? Function used only inside ds1307.c
{
	return ((dec/10*16)+(dec%10));
}
inline static uint8_t bcd2dec (uint8_t bcd) //Quite self-explanatory, isn't it? Function used only inside ds1307.c
{
	return 10*(bcd>>4)+(bcd&0x0F);
}

uint8_t ds1307_read_reg(uint8_t reg) //Reads chosen DS1307 register
{
	uint8_t byte;
	i2c_start();
	i2c_write_byte(DS1307_WR_ADDR);
	i2c_write_byte(reg);
	i2c_start();
	i2c_write_byte(DS1307_RD_ADDR);
	byte = i2c_read_byte(NACK);
	i2c_stop();
	return byte;
}
void ds1307_write_reg(uint8_t value, uint8_t reg) //Writes chosen DS1307 register
{
	i2c_start();
	i2c_write_byte(DS1307_WR_ADDR);
	i2c_write_byte(reg);
	i2c_write_byte(value);
	i2c_stop();
}

void ds1307_set_hour(ds1307_time_t time) //Sets hour in DS1307
{
	i2c_start();
	i2c_write_byte(DS1307_WR_ADDR);
	i2c_write_byte(DS1307_SECS_REG);
	i2c_write_byte(dec2bcd(time.sec));
	i2c_write_byte(dec2bcd(time.min));
	i2c_write_byte(dec2bcd(time.hr));
	i2c_stop();
}

void ds1307_set_date(ds1307_time_t time) //Sets date in DS1307
{
	i2c_start();
	i2c_write_byte(DS1307_WR_ADDR);
	i2c_write_byte(DS1307_WDAY_REG);
	i2c_write_byte(dec2bcd(time.wday));
	i2c_write_byte(dec2bcd(time.day));
	i2c_write_byte(dec2bcd(time.mth));
	i2c_write_byte(dec2bcd(time.yr));
	i2c_stop();
}

ds1307_time_t ds1307_get_time(void) //Retrieves time from DS1307
{
	ds1307_time_t time;
	i2c_start();
	i2c_write_byte(DS1307_WR_ADDR);
	i2c_write_byte(DS1307_SECS_REG);
	i2c_start();
	i2c_write_byte(DS1307_RD_ADDR);
	time.sec = bcd2dec(i2c_read_byte(ACK));
	time.min = bcd2dec(i2c_read_byte(ACK));
	time.hr = bcd2dec(i2c_read_byte(ACK));
	time.wday = bcd2dec(i2c_read_byte(ACK));
	time.day = bcd2dec(i2c_read_byte(ACK));
	time.mth = bcd2dec(i2c_read_byte(ACK));
	time.yr = bcd2dec(i2c_read_byte(NACK));
	i2c_stop();
	return time;
}
