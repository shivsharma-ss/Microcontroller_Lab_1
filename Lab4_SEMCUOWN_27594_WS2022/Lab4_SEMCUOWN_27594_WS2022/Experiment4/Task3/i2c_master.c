#include <avr/io.h>							// Include AVR I/O library
#include <util/twi.h>						// Include TWI definitions
#include "i2c_master.h"						// Include header file "i2c_master.h"
void i2c_master_init(uint8_t pullups, uint8_t prescaler){	// I2C Initialisation
	DDRC &= ~(1<<PC5); 						// TWI pin PC5/SCL configured as input pin
	DDRC &= ~(1<<PC4);						// TWI pin PC4/SDA configured as input pin
	if (pullups){ 							// In case of "uint8_t pullups" is set:
		PORTC |= (1<<PC5);					// Activation of regular
		PORTC |= (1<<PC4);					// internal pullups of MCU pins
	} else {								// If the value of the "pullups" is zero,
		PORTC &= ~(1<<PC5);					// external pullups may be used,
		PORTC &= ~(1<<PC4);					// they should be activated by jumpers
	}
	if (prescaler < 11)						// According to the manufacturer, bitrate
		prescaler = 11;						// prescaler must be greater than 10
	TWBR = prescaler;						// SCL = F_CPU/(16+2(TWBR)*4^TWPS)
	TWSR = (1<<TWPS0);						// Additional TWI bit rate prescaler of 4
}
uint8_t i2c_master_open_raw(uint8_t adr){	// INTERNAL function to connect I2C slave adr
	uint16_t timeout = 500;					// Timeout variable
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);	// Send START condition
	while( !(TWCR & (1<<TWINT)) ){			// wait until this is done
		timeout--;							// Decrement timeout
		if (!timeout){						// If TWI hardware doesn't respond,
			TWCR = 0;						// reset TWI and 
			return 3;						// exit function -> error code 3
		}									// Code lines below: TW_START = 0x08
	}										// Code lines below: TW_REP_START = 0x10
	if((TWSR&0xF8)!=TW_START && (TWSR&0xF8)!=TW_REP_START)	// Check status of TWSR
		return 1;							// exit function with return code 3
	timeout = 500;							// Re-start the timeout variable
	TWDR = adr;								// set I2C slave address
	TWCR = (1<<TWINT)|(1<<TWEN);			// send slave address to the I2C bus and 
	while( !( TWCR & (1<<TWINT)) ){			// wait until this is done
		timeout--;							// Decrement timeout
		if(!timeout) {						// If TWI hardware doesn't respond,
			TWCR = 0;						// reset TWI and 
			return 3;						// exit function -> error code 3
		}									// Code lines below: MT = master transmitter
	}										// Code lines below: MR = master receiver
	if ((TWSR&0xF8) == TW_MT_SLA_ACK) 		// Check status of register TWSR for ACK
		return 0;							// Exit function -> success
	if ((TWSR&0xF8) == TW_MR_SLA_ACK) 		// Binary op. &0xF8 masks out prescaler bits
		return 0;							// If ACK -> success, code 0
	return 2;								// If no ACK: exit function -> error code 2
}
uint8_t i2c_master_open_write(uint8_t adr){ // Ensure the writing access by 
	return i2c_master_open_raw(adr & 0xFE);	// masking LSB out (last bit is zero for write)
}
uint8_t i2c_master_open_read(uint8_t adr){	// Ensure the reading access by 
	return i2c_master_open_raw(adr | 0x01);	// setting LSB on (last bit is one for read)
}
uint8_t i2c_master_write(uint8_t towrite){	// Send data to the already open device
	TWDR = towrite;							// Put data to the TW data register
	TWCR = (1<<TWINT)|(1<<TWEN);			// Start transmission over the I2C bus and 
	while( !( TWCR & (1<<TWINT) ) );		// wait until transmission is complete
	if((TW_STATUS&0xF8) != TW_MT_DATA_ACK )	// Check value of TWI Status Register for ACK bit
		return 1; 							// If no ack: exit function -> error code 1
	return 0;								// If no error, exit function -> success
}
uint8_t i2c_master_read_next(void){ 		// Start receiving from the already open device
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);	// Start transmission over the I2C bus and 
	while( !( TWCR & (1<<TWINT) ) );		// wait until transmission is complete
	return TWDR;							// Return received byte (from TW data register)
}
uint8_t i2c_master_read_last(void){			// Start receiving without ACK bit (= last byte)
	TWCR = (1<<TWINT)|(1<<TWEN);			// Start transmission over the I2C bus and 
	while( !( TWCR & (1<<TWINT) ) );		// wait until transmission is complete
	return TWDR;							// Return received byte (from TW data register)
}
void i2c_master_close(){					// Closes the transmission
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);	// Send stop condition and
	while( TWCR & (1<<TWSTO) );				// wait until this is done (bit TWSTO is zero)
}