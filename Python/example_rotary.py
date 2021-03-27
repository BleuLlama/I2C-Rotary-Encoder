#!/usr/bin/python3


# sudo apt-get install python3-smbus
# sudo pip3 install smbus

import smbus as smbus
import time


IRC_REG_ADDR    = 0
IRC_REG_VS      = 1
IRC_REG_VL      = 2
IRC_REG_BDOWN   = 3
IRC_REG_BCOUNT  = 4
IRC_REG_ROTACW  = 5
IRC_REG_ROTCW   = 6

i2cbus = smbus.SMBus(1)
address = 0x42

print( "Starting up..." )


def i2c_get_signed( addr, reg ):
    value = i2cbus.read_byte_data( addr, reg ) & 0x7f
    # extend the bit over
    value |= (value & 0x40) << 1
    # magic to convert it to signed
    value = value & 0xff
    return (value ^ 0x80) - 0x80

def i2c_get( addr, reg ):
    value = i2cbus.read_byte_data( addr, reg ) & 0x7f
    return value

while True:
    print( "R:{}  B:{}  BP:{}".format( 
        i2c_get( address, IRC_REG_ROTCW ) - i2c_get( address, IRC_REG_ROTACW ),
        i2c_get( address, IRC_REG_BCOUNT ),
        i2c_get( address, IRC_REG_BDOWN ),
        ) )
    time.sleep( 0.25 )
