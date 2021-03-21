# I2C-Rotary-Encoder
A quick program to put a rotary encoder on the I2C bus.

I created this so that I could noodle around with making a 
user interface for the [https://github.com/akuker/RASCSI](RaSCSI)
project, so that the OLED display it has could do more than just 
show the current values.

I'm using a standard rotary encoder board that I had leftover from
my Pi1541 and Gotek drive modifications.

# Connecting

Currently, this is only setup out of the box to work with a
standard Arduino Uno (ATmega 328P) micro.  Other devices will
require different pin configurations and wirings.

	Encoder Board   	Arduino Pin
	 CLK 				 D13
	 DT 				 D12
	 SW 				 D11
	 +					 +5V
	 GND				 Ground

	RasPi 		Uno	
	 SDA 		 A4
	 SCL 		 A5
	 5V 		 +5v
	 GND 		 GND


# Setup

If you already have done the setup for the RaSCSI 'oled_monitor', 
then you are good to go.  In short you need to enable the I2C 
interface, and get the i2C tools and such.

If you need to do this, I would recommend the software install
procedure
[https://github.com/akuker/RASCSI/wiki/OLED-Status-Display-(Optional)#Software_Install](over at the RaSCSI project).


# Testing

If you connect this up to your Raspberry Pi, it will appear on I2C
address 0x42.  This is changeable in the arduino source code.

To verify that it is connected:
  i2cdetect -y 1

You should see your knob at 0x42

If you do, you can hit the 6 registers to read the values...

Remember that the high bit will be set on the data byte returned,
so if you read the address out of register 0, it will respond with
0xC2, even though the address is 0x42. For sanity, the rest of this 
doc will just ignore that 0x80 bit.

# Registers

The device has 6 registers that can be read using standard I2C 
procedures:

## Register 0 - I2C Address

Register 0 always returns the I2C address, which by default
is 0x42.  Any register reads greater than 5 (invalid register
address) will also return this value. 

    i2cget -y 1 0x42 0


## Register 1,2 - Version Sentinels

Registers 1 and 2 will always respond with an 'S'(0x53) and an 
'L' (0x4C) respectively for the initial software version.

    i2cget -y 1 0x42 1
    i2cget -y 1 0x42 2


## Register 3 - Rotary Encoder Delta Accumulator

This returns the number of clicks rotated, as well as direction 
since the last time it was called.  That is to say that it
automatically clears the value when you read it, giving just 
the number of clicks since the previous call.

Clockwise movements will be positive values, and anticlockwise
movements will be negative values.

They are returned as a signed 7-bit value, as the top bit is 
always set.  You should read it as an unsigned byte, copy 
bit 7 to bit 8, then cast it to a signed byte to get the 
correctly signed value.

    i2cget -y 1 0x42 3


## Register 4 - Button Is Being Pressed

A value of 0x01 indicates that the encoder button is currently
being pressed.  A value of 0x00 indicates it is not currently 
being pressed.

    i2cget -y 1 0x42 4


## Register 5 - Button Press Accumulator

This will return the number of times that the button was pressed, 
and then released since the last time it was called. That is to 
say that it
automatically clears the value when you read it, giving just 
the number of press-cycles since the previous call.

    i2cget -y 1 0x42 5