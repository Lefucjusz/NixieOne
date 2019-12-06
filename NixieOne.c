//ATmega8, fuses LO = 0xA3, HI = 0xD9
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "bool.h"
#include "booster.h"
#include "nixie.h"
#include "i2c.h"
#include "ds1307.h"
#include "ds18b20.h"
#include "buttons.h"
#include "photoresistor.h"

#define NULL ((void*)0)

#define INT_PIN (1<<PD2)
#define INT_DDR DDRD
#define INT_PORT PORTD

#define L_BRIGHTNESS_V 220
#define M_BRIGHTNESS_V 200
#define S_BRIGHTNESS_V 180
#define XS_BRIGHTNESS_V 160

#define TCNT0_LOAD_VALUE 131

#define CONTENT_HOUR 0
#define CONTENT_DATE 1
#define CONTENT_TEMP 2
#define CONTENT_DEPOISON 3
#define CONTENT_ERROR 4
#define CONTENT_CLEAR 5

#define MODE_HOUR_DATE_TEMP 1
#define MODE_HOUR_DATE 2
#define MODE_HOUR_TEMP 3
#define MODE_HOUR 4

#define MODE_CHANGE_EXIT_DELAY 1500 //*2ms
#define BLINK_DELAY 250 //*2ms

#define HOUR_DISPLAY_START 0
#define HOUR_DISPLAY_END 21
#define DATE_DISPLAY_START 22
#define DATE_DISPLAY_END 31
#define TEMP_DISPLAY_START 32
#define TEMP_DISPLAY_END 42

typedef struct {
	uint8_t digit_1;
	BOOL decimal_point_1;
	uint8_t digit_2;
	BOOL decimal_point_2;
	uint8_t digit_3;
	BOOL decimal_point_3;
	uint8_t digit_4;
	BOOL decimal_point_4;
} disp_t;

disp_t display_content;
ds1307_time_t time;
ds18b20_temp_t temperature;

uint8_t selected_mode;

volatile uint8_t main_timer = 0;
volatile uint16_t auxiliary_timer = 0;
uint8_t content_timer = 0;

BOOL rtc_inconsistence = FALSE;
volatile BOOL check_booster = FALSE;
volatile BOOL update_time = FALSE;

const uint8_t mths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void display_content_init(void)
{
	display_content.decimal_point_1 = FALSE;
	display_content.decimal_point_2 = TRUE;
	display_content.decimal_point_3 = FALSE;
	display_content.decimal_point_4 = FALSE;
	display_content.digit_1 = time.hr/10;
	display_content.digit_2 = time.hr%10;
	display_content.digit_3 = time.min/10;
	display_content.digit_4 = time.min%10;
}
void display_content_update(uint8_t content_type)
{
	static uint8_t digit_counter = 0;
	switch(content_type)
	{
		case CONTENT_HOUR:
			display_content.digit_1 = time.hr/10;
			display_content.digit_2 = time.hr%10;
			display_content.digit_3 = time.min/10;
			display_content.digit_4 = time.min%10;
		break;
		case CONTENT_DATE:
			display_content.digit_1 = time.day/10;
			display_content.digit_2 = time.day%10;
			display_content.digit_3 = time.mth/10;
			display_content.digit_4 = time.mth%10;
		break;
		case CONTENT_TEMP:
			display_content.digit_1 = temperature.integer/10;
			display_content.digit_2 = temperature.integer%10;
			display_content.digit_3 = temperature.fraction/10;
			display_content.digit_4 = NIXIE_OFF;
		break;
		case CONTENT_DEPOISON:
				display_content.digit_1 = digit_counter;
				display_content.digit_2 = digit_counter;
				display_content.digit_3 = digit_counter;
				display_content.digit_4 = digit_counter;
				digit_counter++;
				if(digit_counter > 9)
					digit_counter = 0;
		break;
		case CONTENT_ERROR:
			display_content.digit_1 = 8;
		    display_content.digit_2 = 8;
			display_content.digit_3 = 8;
			display_content.digit_4 = 8;
			display_content.decimal_point_1 = TRUE;
			display_content.decimal_point_2 = TRUE;
			display_content.decimal_point_3 = TRUE;
			display_content.decimal_point_4 = TRUE;
		break;
		case CONTENT_CLEAR:
			display_content.digit_1 = NIXIE_OFF;
			display_content.digit_2 = NIXIE_OFF;
			display_content.digit_3 = NIXIE_OFF;
			display_content.digit_4 = NIXIE_OFF;
			display_content.decimal_point_1 = FALSE;
			display_content.decimal_point_2 = FALSE;
			display_content.decimal_point_3 = FALSE;
			display_content.decimal_point_4 = FALSE;
		break;
	}
}
void display_multiplex(void)
{
	static uint8_t nixie_counter = 0;
	switch(nixie_counter++)
	{
		case 0:
			nixie_show_value(display_content.digit_1, display_content.decimal_point_1, NIXIE_1);
		break;
		case 1:
			nixie_show_value(display_content.digit_2, display_content.decimal_point_2, NIXIE_2);
		break;
		case 2:
			nixie_show_value(display_content.digit_3, display_content.decimal_point_3, NIXIE_3);
		break;
		case 3:
			nixie_show_value(display_content.digit_4, display_content.decimal_point_4, NIXIE_4);
			nixie_counter = 0;
		break;
	}
}

void timer_start(void)
{
	TCNT0 = TCNT0_LOAD_VALUE;
	TCCR0 = (1<<CS01) | (1<<CS00);
	TIMSK |= (1<<TOIE0);
}
void timer_stop(void)
{
	TCCR0 = 0;
}

void rtc_interrupt_init(void)
{
	INT_DDR &= ~INT_PIN; //Interrupt pin as input
	INT_PORT |= INT_PIN; //Turn on internal pullup
	MCUCR |= (1<<ISC00); //Both edges because of the colon purpose - only rising or falling would be sufficient otherwise
	GICR |= (1<<INT0); //Enable INT0 interrupt
}
BOOL rtc_data_check_read(void)
{
	if(!(ds1307_read_reg(DS1307_DATA_OK_REG) == DS1307_DATA_OK)) //Data are not consistent, probably battery discharged
	{
		//Write proper value to consistency check register, so that the error doesn't appear next time if the battery has been replaced
		ds1307_write_reg(DS1307_DATA_OK, DS1307_DATA_OK_REG);
		//Turn on generating interrupts every second, as without it clock won't work
		ds1307_write_reg(0x10, DS1307_CTRL_REG);
		selected_mode = MODE_HOUR_DATE_TEMP; //Set default mode
		ds1307_write_reg(MODE_HOUR_DATE_TEMP, DS1307_MODE_REG); //And save it in proper register
		time.hr = 0;
		time.min = 0;
		time.sec = 0;
		time.day = 1;
		time.mth = 1;
		time.yr = 18;
		ds1307_set_hour(time); //Set default correct time and start oscillator (0 written to sec register starts it)
		ds1307_set_date(time);
		return TRUE; //Report failure
	}
	else //If data seem to be consistent
	{
		selected_mode = ds1307_read_reg(DS1307_MODE_REG); //Retrieve them from RTC
		time = ds1307_get_time();
		return FALSE; //Report success
	}
}

uint8_t ambientlight2voltage(uint8_t light)
{
	if(light >= 120)
		return L_BRIGHTNESS_V;
	if(light < 120 && light >= 60)
		return M_BRIGHTNESS_V;
	if(light < 60 && light >= 20)
		return S_BRIGHTNESS_V;
	return XS_BRIGHTNESS_V;
}

void hr_day_increment(void)
{
	if(rtc_inconsistence)
	{
		rtc_inconsistence = FALSE;
		display_content_update(CONTENT_CLEAR);
	}
	if(content_timer <= HOUR_DISPLAY_END)
	{
		content_timer = HOUR_DISPLAY_START;
		time.hr++;
		if(time.hr > 23)
			time.hr = 0;
		time.sec = 0;
		display_content_update(CONTENT_HOUR);
		ds1307_set_hour(time);
	}
	else if(content_timer > HOUR_DISPLAY_END && content_timer < DATE_DISPLAY_END)
	{
		content_timer = DATE_DISPLAY_START;
		time.day++;
		if(time.day > mths[time.mth - 1])
			time.day = 1;
		display_content_update(CONTENT_DATE);
	}
	ds1307_set_date(time);
}
void min_month_increment(void)
{
	if(rtc_inconsistence)
	{
			rtc_inconsistence = FALSE;
			display_content_update(CONTENT_CLEAR);
	}
	if(content_timer <= HOUR_DISPLAY_END)
	{
		content_timer = HOUR_DISPLAY_START;
		time.min++;
		if(time.min > 59)
			time.min = 0;
		time.sec = 0;
		display_content_update(CONTENT_HOUR);
		ds1307_set_hour(time);
	}
	else if(content_timer >= DATE_DISPLAY_START && content_timer < DATE_DISPLAY_END)
	{
		content_timer = DATE_DISPLAY_START;
		time.mth++;
		if(time.mth > 12)
			time.mth = 1;
		if(time.day > mths[time.mth - 1])
			time.day = 1;
		display_content_update(CONTENT_DATE);
	}
	ds1307_set_date(time);
}
void mode_change(void)
{
	if(rtc_inconsistence)
		return;
	content_timer = 0;
	selected_mode++;
	if(selected_mode > MODE_HOUR)
		selected_mode = MODE_HOUR_DATE_TEMP;
	display_content.decimal_point_2 = FALSE;
	display_content.digit_1 = NIXIE_OFF;
	display_content.digit_2 = NIXIE_OFF;
	display_content.digit_3 = NIXIE_OFF;
	display_content.digit_4 = selected_mode;
	ds1307_write_reg(selected_mode, DS1307_MODE_REG);
	auxiliary_timer = MODE_CHANGE_EXIT_DELAY;
}

void buttons_assign(button_t* hr_day, button_t* min_month, button_t* mode)
{
	hr_day->button_pin = &BUTTON_READ;
	hr_day->button_mask = HR_DAY_BUTTON;
	hr_day->click_func = hr_day_increment;
	hr_day->hold_func = hr_day_increment;
	hr_day->repeat_delay_ms = 200;
	hr_day->hold_delay_ms = 1000;
	min_month->button_pin = &BUTTON_READ;
	min_month->button_mask = MIN_MONTH_BUTTON;
	min_month->click_func = min_month_increment;
	min_month->hold_func = min_month_increment;
	min_month->repeat_delay_ms = 200;
	min_month->hold_delay_ms = 1000;
	mode->button_pin = &BUTTON_READ;
	mode->button_mask = MODE_BUTTON;
	mode->click_func = NULL;
	mode->hold_func = mode_change;
	mode->repeat_delay_ms = 500;
	mode->hold_delay_ms = 1000;
}

void main(void)
{
	uint8_t ambient_light, desired_voltage;
	BOOL booster_error;
	BOOL display_on = FALSE;
	button_t hr_day_button, min_month_button, mode_button;
	wdt_enable(WDTO_250MS); //enable watchdog for 250ms
	booster_start();
	nixie_init();
	photoresistor_init();
	i2c_init();
	rtc_interrupt_init();
	buttons_init();
	buttons_assign(&hr_day_button, &min_month_button, &mode_button);
	rtc_inconsistence = rtc_data_check_read();
	display_content_update(CONTENT_HOUR);
	sei();
	timer_start();
	while(1)
	{
		if(check_booster)
		{
			wdt_reset(); //normally this should be called every 4ms or 40ms, if it wasn't for 250ms - CPU has halted and watchdog will restart it
			check_booster = FALSE;
			booster_error = booster_control(desired_voltage);
			if(booster_error)
			{
				wdt_disable();
				timer_stop();
			}
		}
		if(rtc_inconsistence)
		{
			if(!auxiliary_timer) //Blink display
			{
				if(display_on)
				{
					display_content_update(CONTENT_ERROR);
					display_on = FALSE;
				}
				else
				{
					display_content_update(CONTENT_CLEAR);
					display_on = TRUE;
				}
				auxiliary_timer = BLINK_DELAY;
			}
		}
		button_check(&hr_day_button);
		button_check(&min_month_button);
		button_check(&mode_button);
		if(update_time && !auxiliary_timer && !rtc_inconsistence)
		{
			update_time = FALSE;
			time = ds1307_get_time();
			if(content_timer <= HOUR_DISPLAY_END)
			{
				if(content_timer == HOUR_DISPLAY_START && selected_mode != MODE_HOUR)
				{
					display_content.decimal_point_2 = FALSE;
					ds18b20_convert_temp();
				}
				display_content.decimal_point_2 = display_content.decimal_point_2 ? FALSE:TRUE;
				display_content_update(CONTENT_HOUR);
			}
			else if(content_timer == DATE_DISPLAY_START)
			{
				display_content.decimal_point_2 = TRUE;
				display_content_update(CONTENT_DATE);
			}
			else if(content_timer == TEMP_DISPLAY_START)
			{
				temperature = ds18b20_get_temp();
				display_content.decimal_point_2 = TRUE;
				display_content_update(CONTENT_TEMP);
			}

			if(selected_mode != MODE_HOUR)
			{
				content_timer++;
				if(content_timer >= TEMP_DISPLAY_END)
					content_timer = HOUR_DISPLAY_START;
			}
			if(selected_mode == MODE_HOUR_DATE)
			{
				if(content_timer == TEMP_DISPLAY_START)
					content_timer = HOUR_DISPLAY_START;
			}
			else if(selected_mode == MODE_HOUR_TEMP)
			{
				if(content_timer == DATE_DISPLAY_START)
					content_timer = TEMP_DISPLAY_START;
			}
			ambient_light = photoresistor_get_value(); //Check the value of ambient light...
			desired_voltage = ambientlight2voltage(ambient_light); //...and update display brightness
		}
		else if(time.min == 59 && time.sec == 59)
		{
			content_timer = 0;
			display_content_update(CONTENT_DEPOISON);
		}
	}
}

ISR(TIMER0_OVF_vect) //should be called every 2ms
{
	main_timer++;
	if(main_timer & 1) //If value is odd
		display_multiplex(); //not very proud of it...
	if(main_timer == BOOSTER_CHECK_RATE)
	{
		check_booster = TRUE;
		main_timer = 0;
	}
	if(button_timer)
		button_timer--;
	if(auxiliary_timer)
		auxiliary_timer--;
	TCNT0 = TCNT0_LOAD_VALUE;
}

ISR(INT0_vect) //interrupt from DS1307, should be called two times per second
{
	update_time = TRUE;
}
