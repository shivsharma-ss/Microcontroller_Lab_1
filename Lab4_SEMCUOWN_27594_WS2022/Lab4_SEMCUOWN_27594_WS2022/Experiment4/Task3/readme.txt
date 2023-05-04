myAVR board connections:
	
	LCD module
	Memory module
	
Description:

	The memory module has three jumpers which can be used to change its I2C address in order to use up to 8 memory modules simultaneously. 
	This program automatically finds the I2C address of the memory module if it's present.
	It then displays the address in decimal, hexadecimal and binary form. 
	Additionally, it shows how the jumpers are connected: 
	For example, "RLR" refers the first jumper being set to the "right" position, the second one to the "left" and the third one to the "right", 
	resulting in the sequence "101" as the final three bits of the I2C address.