#ifndef DS1307_H_
#define DS1307_H_

#include <stdint.h>

#define DS1307_SECS_REG 0x00
#define DS1307_MINS_REG 0x01
#define DS1307_HRS_REG 0x02
#define DS1307_WDAY_REG 0x03
#define DS1307_DAY_REG 0x04
#define DS1307_MTH_REG 0x05
#define DS1307_YR_REG 0x06
#define DS1307_CTRL_REG 0x07
#define DS1307_DATA_OK_REG 0x08 //General RAM byte
#define DS1307_MODE_REG 0x09 //General RAM byte

#define DS1307_DATA_OK 0xAA //Data consistency value, 0b10101010

typedef struct //Convenient structure for holding RTC information
{
	uint8_t hr;
	uint8_t min;
	uint8_t sec;
	uint8_t wday;
	uint8_t day;
	uint8_t mth;
	uint8_t yr;
} ds1307_time_t;

uint8_t ds1307_read_reg(uint8_t reg); //Reads chosen DS1307 register
void ds1307_write_reg(uint8_t value, uint8_t reg); //Writes chosen DS1307 register

void ds1307_set_hour(ds1307_time_t time); //Sets hour in DS1307
void ds1307_set_date(ds1307_time_t time); //Sets date in DS1307
ds1307_time_t ds1307_get_time(void); //Retrieves time from DS1307

#endif /* DS1307_H_ */
