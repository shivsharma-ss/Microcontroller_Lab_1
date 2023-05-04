/**
	@author Shivansh Sharma
	@author 27594
	@version 0.1
	@file main.c
	@brief Main programm for RTC DS1307
*/

const char MtrNum[] __attribute__((__progmem__)) = "27594";

/**
	@brief The CPU speed in Hz
*/
#define F_CPU 8000000UL

/**
	@brief I2C Address of the DS1307
*/
#define DS1307_I2C_ADR 0xD0 

/******************************************************************************/
/* INCLUDED FILES                                                             */
/******************************************************************************/
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "i2c_master.h" 
#include "lcd.h"
/******************************************************************************/


/******************************************************************************/
/* GLOBAL MEMORY                                                              */
/******************************************************************************/
char* dayofweek[8] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun", "Err"};
char roll[8];
char matrikel[8] = "SE27594";

// Global Time memory
uint8_t second;
uint8_t minute;
uint8_t hour;
uint8_t day;
uint8_t month;
uint8_t year;
uint8_t weekday;

uint8_t ram_adr = 0x08; // Address of the RAM in the RTC

/******************************************************************************/



/******************************************************************************/
/* FUNCTIONS                                                                  */
/******************************************************************************/

/**
	@brief Convert from BCD to Binary
	@param in BCD input (00-99)
	@return Binary output
*/
uint8_t ds1307_decodeBcd(uint8_t in){
	return(((in>>4)*10)+ (in&0xF) );
}
/******************************************************************************/

/**
	@brief Convert from Binary to BCD
	@param in Binary input (0-99)
	@return BCD output
*/
uint8_t ds1307_encodeBcd(uint8_t in){
	return ((in / 10) << 4 ) | (in % 10); 
}
/******************************************************************************/


/**
	@brief Show time/date with the LCD
*/
void display_showtime(void){
	char str[16];
	
	// Time and Year
	snprintf(str, 16, "%02d:%02d:%02d  20%02d", hour, minute,
			second, year);
	
	lcd_clear();
	lcd_string(str);
	
	
	// Date and Weekday
	if (weekday < 9 && weekday > 0 ){
		snprintf(str, 16, "%02d.%02d %s", day, month,
			dayofweek[weekday - 1 ]);
	} else {
		snprintf(str, 16, "Error");
	}
	
	lcd_setcursor(0,2);
	lcd_string(str);

	return;
}
/******************************************************************************/

/**
	@brief Write a row byte to the DS1307
	@param adr address to write
	@param data byte to write
*/
void ds1307_write(uint8_t adr, uint8_t data){
	
	if (i2c_master_open_write(DS1307_I2C_ADR))
		return;
	
	i2c_master_write(adr);
	i2c_master_write(data);
	
	i2c_master_close();
}
/******************************************************************************/

/**
	@brief Read a row byte from the DS1307
	@param adr address to read
	@return the received byte
*/
uint8_t ds1307_read(uint8_t adr){
	uint8_t ret;

	if (i2c_master_open_write(DS1307_I2C_ADR))
		return 0;
	
	i2c_master_write(adr);
	i2c_master_open_read(DS1307_I2C_ADR);
	ret = i2c_master_read_last();
	
	i2c_master_close();

	return ret;

}
/******************************************************************************/

/**
	@brief Start or freeze the clock of the DS1307
	@param run zero for stop, all other for run
*/
void ds1307_rtc(uint8_t run){
	
	uint8_t readout;
	
	// Read current value
	readout = ds1307_read(0x00);
	
	// Set CH bit
	if (run)
		readout &= ~(0x80);
	else
		readout |= 0x80;
	
	// Write value back
	ds1307_write(0x00, readout);
}
/******************************************************************************/

/**
	@brief Write the current time to the DS1307
	@return zero for no error, one for communication error
*/
uint8_t ds1307_setTime(void){
	uint8_t chbit = ds1307_read(0x00) & 0x80;
	
	// Open device for write
	if (i2c_master_open_write(DS1307_I2C_ADR))
		return 1;

	i2c_master_write(0x00);
	if (chbit)
		i2c_master_write(ds1307_encodeBcd(second) | 0x80);
	else
		i2c_master_write(ds1307_encodeBcd(second) & 0x7F);		
	
	i2c_master_write(ds1307_encodeBcd(minute));
	i2c_master_write(ds1307_encodeBcd(hour));
	
	i2c_master_write(weekday);		
	
	i2c_master_write(ds1307_encodeBcd(day));
	i2c_master_write(ds1307_encodeBcd(month));
	i2c_master_write(ds1307_encodeBcd(year));

	
	// Close I2C bus
	i2c_master_close();
	
	return 0;
}
/******************************************************************************/

/**
	@brief Get the current time from the DS1307
	@return zero for no error, one for communication error
*/
uint8_t ds1307_getTime(void){
	
	// Open device for write
	if (i2c_master_open_write(DS1307_I2C_ADR))
		return 1;
	
	// select reading position (0x00)
	i2c_master_write(0x00);
	
	// (Re-)Open device for read
	i2c_master_open_read(DS1307_I2C_ADR);
	
	// Read value
	second = ds1307_decodeBcd(i2c_master_read_next() & 0x7F);
	minute = ds1307_decodeBcd(i2c_master_read_next());
    hour = ds1307_decodeBcd(i2c_master_read_next() & 0x3F);
    weekday = ds1307_decodeBcd(i2c_master_read_next());
    day = ds1307_decodeBcd(i2c_master_read_next());
    month = ds1307_decodeBcd(i2c_master_read_next());
    year = ds1307_decodeBcd(i2c_master_read_next());

	
	// Close I2C bus
	i2c_master_close();
	
	return 0;
}
/******************************************************************************/

/**
	@brief Set the time to the "default" time
*/
void ds1307_SetToDefaultTime(void){

	// Defined start time
	second = 55;
	minute = 59;
	hour = 23;
	day = 31;
	month = 12;
	year = 21;
	weekday = 5;
	ds1307_setTime();
}
/******************************************************************************/

uint8_t get_roll(){

	uint8_t i;
	i2c_master_open_write(DS1307_I2C_ADR);
	i2c_master_write(ram_adr);
	i2c_master_open_read(DS1307_I2C_ADR);
	for (i = 0; i < 8; i++){
		roll[i] = i2c_master_read_next();
	}

	i2c_master_close();

	if (roll[0] == 0x00 && roll[1] == 0x00 && roll[2] == 0x00 && roll[3] == 0x00 && roll[4] == 0x00 && roll[5] == 0x00 && roll[6] == 0x00 && roll[7] == 0x00){
		return 0;
	} else {
		return 1;
	}

}


/**
	@brief Main function
	@return only a dummy to avoid a compiler warning, not used
*/
int main(void){
	uint8_t i, running = 1; // Assumes that RTC should always run after the program start
	lcd_init(); // Function to initialise LCD display
	i2c_master_init(1, 10); // Init TWI
	running = ds1307_read(0x00); // last state of RTC
	init();

	uint8_t start = 0;	//state of rtc

	// Loop forever
	while (1){
		
		//Check the state of rtc------------------------------------------------
		if(!start){
			start = 1;
		}
		else{
			running = ds1307_read(0x27);	//address where the play/pause state is stored
		}
        
		// Short delay----------------------------------------------------------
		_delay_ms(100);
		
		// Load current time/date from DS1307-----------------------------------
		ds1307_getTime();
		
		// Show current time/date-----------------------------------------------
		display_showtime();

		if (get_roll() == 0){

			i2c_master_open_write(DS1307_I2C_ADR);
			i2c_master_write(ram_adr);
			for (i = 0; i < 8; i++){
				i2c_master_write(matrikel[i]);
			}

			char str[8];
			snprintf(str, 8, "%s", matrikel);
			lcd_setcursor(9, 2);
			lcd_string(str);

		}
		else {
			
            char str[8];
			snprintf(str, 8, "%s", roll);
			lcd_setcursor(9, 2);
			lcd_string(str);

		}
		
        
		// PB0 to toggle run----------------------------------------------------
		if ( ~PINB & (1 << PB0) ){

			if (running)
				running = 0;
			else
				running = 1;
		
			ds1307_write(0x27, running);

			ds1307_rtc(running);		
		
			// Debouncing
			_delay_ms(50);
			while ( ~PINB & (1 << PB0) )
				;
			_delay_ms(50);
		}	
		
		// PB1 set to default time----------------------------------------------
		if ( ~PINB & (1 << PB1) ){
			ds1307_SetToDefaultTime();
			
			// Debouncing
			_delay_ms(50);
			while ( ~PINB & (1 << PB0) )
				;
			_delay_ms(50);			
			
		}
	}

	return 0;
}
/******************************************************************************/
