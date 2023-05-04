/**
	@author Shivansh Sharma
	@author 27594
	@version 0.1
	@file main.c
	@brief Lab 5 - Main programm for temperature data logger
*/

const char MtrNum[] __attribute__((__progmem__)) = "27594";

/**
	@brief The CPU speed in Hz
*/
#define F_CPU 8000000UL


#define TEMPSENSOR_OFFSET 600  // TODO

/**
	@brief I2C Address of the DS1307
*/
#define DS1307_I2C_ADR 0xD0 //TODO: Enter the Address of the DS1307 

/******************************************************************************/
/* INCLUDED FILES                                                             */
/******************************************************************************/
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "i2c_master.h"
#include "init.h"
#include "lcd.h"
#include "stdio.h"
/******************************************************************************/


/******************************************************************************/
/* GLOBAL MEMORY                                                              */
/******************************************************************************/
char* dayofweek[8] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun", "Err"};

// Global Time memory
uint8_t second;
uint8_t minute;
uint8_t hour;
uint8_t day;
uint8_t month;
uint8_t year;
uint8_t weekday;

/******************************************************************************/

void init(void){
// Digital input
DDRB &= ~(1 << DDB0);		// PB0 as Input
DDRB &= ~(1 << DDB1);		// PB1 as Input
PORTB |= ( 1 << PB0);		// Pullup PB0
PORTB |= ( 1 << PB1);		// Pullup PB1
}

/******************************************************************************/
/* FUNCTIONS                                                                  */
/******************************************************************************/

/**
	@brief Convert from BCD to Binary
	@param in BCD input (00-99)
	@return Binary output
*/
uint8_t ds1307_decodeBcd(uint8_t in){
	//TODO
	return (((in>>4)*10) + (in&0xF));
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
void display_standby(uint16_t t){
	char str[16];
	
	// Time and Year
	snprintf(str, 16, "%02d:%02d:%02d  20%02d", hour, minute, second, year);
	
	lcd_clear();
	lcd_string(str);
	
	
	// Date and Temperature
	snprintf(str, 16, "%02d.%02d  %d.%d C", day, month, t/10, t%10);
	
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
	// TODO minute, hour, ...
	minute = ds1307_decodeBcd(i2c_master_read_next());
	hour = ds1307_decodeBcd(i2c_master_read_next() & 0x3F);
	weekday = ds1307_decodeBcd(i2c_master_read_next());
	day = ds1307_decodeBcd(i2c_master_read_next());
	month = ds1307_decodeBcd(i2c_master_read_next());
	year = ds1307_decodeBcd(i2c_master_read_last());
	
	// Close I2C bus
	i2c_master_close();
	
	return 0;
}
/******************************************************************************/


void nexttime(void){
	uint8_t days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	// Every 5 seconds
	if (second>=55)
	{
		second = second%5;
		minute++;

	} else {
		second = second + 5;
	}
	if (minute >= 60)	// Goto next minute
	{
		minute = 0;
		hour++;

	}else {
		return;
	}
	
	// Check hour
	if (hour >= 24){
		hour = 0;
		day++;
	} else {
		return;
	}
	
	// Check for gap year
	// TODO
	if (((year%4 == 0) && (year%100 != 0))||(year%400==0)){

		days[1] = 29;
	}
	
	// Check day
	if (day > days[month]){
		day= 1;
		month++;
	}
	
	// Check month
	if (month >= 12){
		month = 1;
		year++;
	}
}
/******************************************************************************/


/**
	@brief Load 8 bit value from the EEPROM
	@return loaded value
*/

uint8_t load_value8bit(uint8_t pos){
	uint8_t value;

	/* TODO */
	uint8_t EEPROM_adr = 0, i;									// Loop variables
	for (i = 0b10100000; i <= 0b10101110; i = i+2) {	// Find EEPROM I2C address
		if (!i2c_master_open_write(i)) {				// If slave responds to address
			EEPROM_adr = i;									// Store address in variable adr
			i2c_master_close();							// Close I2C connection
		}
	}
	i2c_master_open_write(EEPROM_adr);
	i2c_master_write(pos);
	i2c_master_open_read(EEPROM_adr);
	value = i2c_master_read_last();
	i2c_master_close();
	
	return value;
}
/******************************************************************************/


/**
	@brief Load a 16 bit value from the EEPROM
	@return loaded value
*/

uint16_t load_value16bit(uint8_t pos){
	uint8_t highbyte, lowbyte;

	/* TODO */
	uint8_t EEPROM_adr = 0, i;									// Loop variables
	for (i = 0b10100000; i <= 0b10101110; i = i+2) {	// Find EEPROM I2C address
		if (!i2c_master_open_write(i)) {				// If slave responds to address
			EEPROM_adr = i;									// Store address in variable adr
			i2c_master_close();							// Close I2C connection
		}
	}
	i2c_master_open_write(EEPROM_adr);
	i2c_master_write(pos);
	i2c_master_open_read(EEPROM_adr);
	highbyte = i2c_master_read_next();
	lowbyte = i2c_master_read_last();
	i2c_master_close();
	
	return highbyte * 256 + lowbyte;
}
/******************************************************************************/

/**
	@brief Save a 8 bit value to the EEPROM
	@param tosave value to save
*/

void save_value8bit(uint8_t tosave, uint8_t pos){

	/* TODO */
	uint8_t EEPROM_adr = 0, i;									// Loop variables
	for (i = 0b10100000; i <= 0b10101110; i = i+2) {	// Find EEPROM I2C address
		if (!i2c_master_open_write(i)) {				// If slave responds to address
			EEPROM_adr = i;									// Store address in variable adr
			i2c_master_close();							// Close I2C connection
		}
	}
	i2c_master_open_write(EEPROM_adr);
	i2c_master_write(pos);
	i2c_master_write(tosave);

	i2c_master_close();
	_delay_ms(10); // wait 10ms to make sure that data is written
}
/******************************************************************************/


/**
	@brief Save a 16 bit value to the EEPROM
	@param tosave value to save
*/
void save_value16bit(uint16_t tosave, uint8_t pos){
	uint8_t highbyte, lowbyte;
	highbyte = tosave/256;
	lowbyte = tosave%256;
	
	uint8_t EEPROM_adr = 0, i;									// Loop variables
	for (i = 0b10100000; i <= 0b10101110; i = i+2) {	// Find EEPROM I2C address
		if (!i2c_master_open_write(i)) {				// If slave responds to address
			EEPROM_adr = i;									// Store address in variable adr
			i2c_master_close();							// Close I2C connection
		}
	}
	
	i2c_master_open_write(EEPROM_adr);
	i2c_master_write(pos);
	i2c_master_write(highbyte);	
	i2c_master_write(lowbyte);

	i2c_master_close();
	_delay_ms(10); // wait 10ms to make sure that data is written	
}
/******************************************************************************/


/**
	@brief Read the temperature with the internal analog sensor
	@return temperature in 1/10 deg. Celsius
*/
uint16_t adc_temperature_oversample(uint16_t temp_offset){
	uint8_t i;
	uint32_t sum = 0;

	ADMUX = (1 << REFS1)|(1 << REFS0)|(1 << MUX3);	//temp sensor activation
	
	for (i = 0; i < 128; i++){
		ADCSRA |= (1 << ADSC)| (1 << ADEN); // Start ADC
	
		while( ADCSRA & (1 << ADSC) ) // wait for ADC complete
			;
	
		sum += ADCW;
	}
	
//	sum /= 128;		//Average of the values
	sum /= 32;		//because of the prescaler

	// substract offset
	sum = sum - temp_offset;
	
	// 0.27 deg. Celsius per step
	sum *= 27;
	sum /= 100;	//93 around is better
	
	return sum;
}
/******************************************************************************/


//Check if EEPROM is Empty

uint8_t EEPROM_isEmpty() {
	uint8_t currentbyte;
	uint8_t k = 1;
	int i;
	
	for (i = 0; i < 256; i++)
	{
		currentbyte = load_value8bit(i);
		if (currentbyte != 0)
		{
			k = 0;
			break;
		}
	}
	
	return k;	
}
/******************************************************************************/

// Delete EEPROM contents

void EEPROM_clear() {
	int i;
	
	for (i = 0; i < 256; i++)
	{
		save_value8bit(0, i);
	}
	
	lcd_clear();
	lcd_string("Memory Cleared");
	_delay_ms(1000);
}

/******************************************************************************/


void log_data(uint16_t temp_offset){
	// TODO
	display_standby(adc_temperature_oversample(temp_offset));
	if (!EEPROM_isEmpty()) 
	{
		lcd_clear();
		lcd_string("Memory not empty");
		_delay_ms(500);
		return;
	}

	uint8_t n;
	uint16_t temp;
	uint8_t prevmin;
	uint8_t prevsec;
	char str[16];

	//SAVE VALUES
	save_value8bit(year, 1);
	save_value8bit(month, 2);
	save_value8bit(day, 3);
	save_value8bit(hour, 4);
	save_value8bit(minute, 5);
	save_value8bit(second, 6);
	
	

	_delay_ms(1000); 
	for (n = 1; n <= 124; n++)
	{
		prevmin = minute;
		prevsec = second;
		temp = adc_temperature_oversample(temp_offset); 
		
		while(((second<(prevsec+5)) && (minute==prevmin))||((minute!=(prevmin)) && (((60-prevsec) + second) < 5)))		//Checking 5 seconds
		{
			_delay_ms(100);
			
			temp = adc_temperature_oversample(temp_offset);
			save_value16bit(temp, 2*(n-1)+7);
			save_value8bit(n, 0); 
			lcd_clear();
			snprintf(str, 16, "Recording: %03d", n);
			lcd_string(str);
			lcd_setcursor(0,2);
			snprintf(str, 16, "%02d:%02d:%02d-%02d.%01d C", hour, minute, second, temp/10, temp%10);
			lcd_string(str);			
			
			//Stop Recording
			if ((~PINB & (1 << PB0)) && (~PINB & (1 << PB1)))	
			{
				lcd_clear();
				lcd_string("Recording Stop");
				_delay_ms(500);
				return;
			}
			ds1307_getTime();
			
		}

		if (n<=124){ 		// for checking purpose
			lcd_clear();
			lcd_string("SAVED!");
			_delay_ms(500);
		}
		
	} 
	
	//If recording not stopped before 124th value
	lcd_clear();
	_delay_ms(50); 
	lcd_string("Memory is full!");
	_delay_ms(1000);
	
	return;
	
}
/******************************************************************************/


void show_data(uint16_t temp_offset){
	// TODO 

	while (~PINB & (1 << PB0)) //Wait for releasing PB0
		;
	_delay_ms(50);
	
	if (EEPROM_isEmpty())
	{
		lcd_clear();
		lcd_string("No Data");
		_delay_ms(1000);
		return;
	}

	char str[16];
	
	int i = 1;
	uint8_t n = load_value8bit(0);
	uint16_t temp;
	uint8_t k = 0;

	while (1)
	{
		_delay_ms(100);

		lcd_clear();								 
		snprintf(str, 16, "Recording: %03d ", n); //Number of temperatures saved
		lcd_string(str);
		lcd_setcursor(0,2);
		ds1307_getTime();
		snprintf(str, 16, "%02d:%02d:%02d-%02d.%01d C", hour, minute, second, adc_temperature_oversample(temp_offset)/10, adc_temperature_oversample(temp_offset)%10); //Current (running) time and temperature
		lcd_string(str);
		
		if (~PINB & (1 << PB0) || ~PINB & (1 << PB1))	//check if either of the buttons are pressed
		{
			_delay_ms(50);
			if (~PINB & (1 << PB0)) //If button 1 (read) is pressed
			{
				_delay_ms(50);
				k = 1;
			}
			while (~PINB & (1 << PB0) || ~PINB & (1 << PB1)) //Check if again either of the buttons were pressed
			{
				_delay_ms(50);
				if (~PINB & (1 << PB0) && ~PINB & (1 << PB1)) //If both buttons pressed, clear data and return
				{
					_delay_ms(50);
					EEPROM_clear();
					while (~PINB & (1 << PB0) && ~PINB & (1 << PB1))
						;
					_delay_ms(50);
					
					return;
				}
			}
			
			if (k == 1) //If both buttons not pressed, and just button 1 is pressed, finally break out of the loop
			{
				_delay_ms(50);	//debouncing
				k = 0;
				break;	
			}				
			
		}
	
	}

	//Retrieve values from memory
	year = load_value8bit(1);
	month = load_value8bit(2);
	day = load_value8bit(3);
	hour = load_value8bit(4);
	minute = load_value8bit(5);
	second = load_value8bit(6);

	while(1)
	{
		_delay_ms(100);
		
		temp = load_value16bit(2*(i-1)+7);
		
		lcd_clear();
		snprintf(str, 16, "Recording:%03d", i); //Number of temperature shown
		lcd_string(str);
		lcd_setcursor(0,2);
		snprintf(str, 16, "%02d:%02d:%02d-%02d.%01dC", hour, minute, second, temp/10, temp%10); //Current time and temperature
		lcd_string(str);
		
		
		if (~PINB & (1 << PB0) || ~PINB & (1 << PB1))
		{
			_delay_ms(50);
			
			if (~PINB & (1 << PB0)) //If button 1 (read) is pressed, next value
			{
				_delay_ms(50);
				k = 1;
			}
			
			if (~PINB & (1 << PB0) && ~PINB & (1 << PB1)) //If both buttons pressed, clear data and return
			{
				_delay_ms(50);
				EEPROM_clear();
				while (~PINB & (1 << PB0) && ~PINB & (1 << PB1))
					;
				_delay_ms(50);
				return;
			}

			if (~PINB & (1 << PB1)) //If button 2 is pressed, do nothing
			{
				_delay_ms(50);
				k = 0;
			}
			
			if (k == 1)	//if button 1 is pressed again, next value
			{
				_delay_ms(50);
				
				if (i == n)		//if last value return to main screen
					return;
				
				i++;
				nexttime();
			}
		}	
	}

	return;
}
/******************************************************************************/


/**
	@brief Main function
	@return only a dummy to avoid a compiler warning, not used
*/
int main(void){
	uint16_t nowtemp, temp_offset;

	init(); 	// Function to initialise I/Os
	lcd_init(); // Function to initialise LCD display
	i2c_master_init(1, 10); // Init TWI


	// Analog Input
	ADMUX |= (1 << REFS1) | (1 << REFS0)|(1 << MUX3); //TODO // 1.1V as reference
	ADCSRA = (1 << ADPS2)| (1 << ADPS1); // ADC Prescale by 64
	ADCSRA |= (1 << ADSC)| (1 << ADEN); // Start first conversion (dummy read)
	ds1307_rtc(1);	// turn on RTC
	
	PORTB |= (1 << PB0)|(1 << PB1);

	// Loop forever
	for(;;){
		
		// Short delay
		_delay_ms(100);
		
		
		// Mesure temperature
		temp_offset = 600;
		nowtemp = adc_temperature_oversample(temp_offset);
		
		
		// Load current time/date from DS1307
		// TODO
		ds1307_getTime();
		
		// Show current time/date
		display_standby(nowtemp);

		// Show recorded data
		if (~PINB & (1 << PB0)){
			_delay_ms(50);
			show_data(temp_offset);
			_delay_ms(75);				//Debouncing
			while (~PINB & (1 << PB0));
			_delay_ms(75);
		}
		
		// Start Recording
		if (~PINB & (1 << PB1)){
			_delay_ms(50);
			log_data(temp_offset);
			_delay_ms(75);				//Debouncing
			while (~PINB & (1 << PB1));
			_delay_ms(75);
		}

	}

	return 0;
}
/******************************************************************************/
