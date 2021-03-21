// I2C Rotary Encoder
//
//  Simple tool to read a rotary encoder and send deltas of its values when queried
//
//  v001 - 2021-03-21

#include <Wire.h>


// Arduino pin IO configuration
#define kPin_Btn  (11)    // wired to "SW" pin on encoder module
#define kPin_EDat (12)    // wired to "DT" pin on encoder module
#define kPin_EClk (13)    // wired to "CLK" pin on encoder module

// ATmega direct pin/IO values for Arduino Uno. (will need to be changed for other 
#define kDATA_Btn (PINB & 0x08)
#define kDATA_EClk (PINB & 0x20)
#define kDATA_EDat (PINB & 0x10)

// I2C Address that we will respond to
#define kI2C_Address (0x42)

// registers in the I2C device we can request info from
#define kAddr_Addr      (0)   // always returns 0x42
#define kAddr_S         (1)   // always returns 'S'
#define kAddr_L         (2)   // always returns 'L'
#define kAddr_Delta     (3)   // returns +CW or -CW clicks since last requested
#define kAddr_Pressing  (4)   // returns 1 if the user is pressing the button now
#define kAddr_Pressed   (5)   // returns number of times the user press+released the button since last requested



byte theData[8] = { kI2C_Address, 'S', 'L', 0, 0, 0 };
volatile byte regReq = 0;

void setup() {
  //Serial.begin( 115200 );

  setup_I2C();

  // setup our button
  pinMode( kPin_Btn, INPUT_PULLUP );

  // setup our encoder (it has builtin pullups)
  pinMode( kPin_EClk, INPUT );
  pinMode( kPin_EDat, INPUT );
}

unsigned long msto = 0L;
unsigned char b = 0;

unsigned char ld = 0;


//////////////////////////////////////////////////
// Main loop & input decoding


// this is a nifty table i came up with for another project.
// if the graycode is normalized to be in 0x03 bits, then 
// just by doing a 2-dimensional array access, you can 
// get the delta rotation clicks since the last time you 
// got the data.
//
//  that is to say:
//    delta = grayDecode[ previousGrayCode ][ currentGrayCode ];
// that's it!
#define NA (0)
char grayDecode[4][4] = {
                /*  00  01  10  11  */
    /* 00 to */ {    0, +1, -1, NA },
    /* 01 to */ {   -1,  0, NA, +1 },
    /* 10 to */ {   +1, NA,  0, -1 },
    /* 11 to */ {   NA, -1, +1,  0 }
};


unsigned char prevGray = 0;

// since the encoder i'm using cycles through a loop from 0x03 -> 0x03 for each detent,
// we need to account for this so the values seem reasonable.  We will only send out
// data when the value returns to 0x03, and use this "minorCount" to keep track of 
// rotation changes and direction.
int minorCount = 0;

void loop() {  

  // read the button and set/clear the "pressing" flag
  if( kDATA_Btn != 0 ) {
    if( theData[ kAddr_Pressing ] == 1 ) {
      theData[ kAddr_Pressed ]++;
    }
    // probably should debounce it but this seems ok for now.
    
    theData[ kAddr_Pressing ] = 0;
  } else {
    theData[ kAddr_Pressing ] = 1;
  }


  // normalize gray code to be in 00 01 10 11
  unsigned char thisGray = (kDATA_EClk?0x01:0x00) | (kDATA_EDat?0x02:0x00);
 
  // decode encoder gray code and apply the delta
  // the encoder i'm using always returns to '3', so only
  // increment the value if the value is 3.
  minorCount += grayDecode[ prevGray ][ thisGray ];
  if( thisGray == 0x03 ) {
    if( minorCount < 0 ) {
      theData[ kAddr_Delta ]--;
    } else if ( minorCount > 0 ) {
      theData[ kAddr_Delta ]++;
    }
    minorCount = 0;
  }
  prevGray = thisGray;
}


//////////////////////////////////////////////////
// I2C stuff...

void setup_I2C() 
{
  Wire.begin(kI2C_Address);     // join i2c bus
  Wire.onReceive(I2C_receiveEvent);
  Wire.onRequest(I2C_requestEvent);
}


// we received a byte (the register number)... store it for later.
void I2C_receiveEvent( int bytecount )
{
  bytecount = bytecount;
  regReq = Wire.read();
}


// a byte was requested to be sent out from us... send it.
void I2C_requestEvent() 
{
  if( regReq > 5 ) {
    // ignore all out-of-range values, just return our address.
    Wire.write( (byte *)&theData[ 0 ], 1 );
    return;
  }
  
  // seems like a reasonable address, send back the byte!
  Wire.write( (byte *)&theData[ regReq ], 1 );

  // clear any accumulator registers
  if(    regReq == kAddr_Delta 
      || regReq == kAddr_Pressed ) 
  {
    theData[ regReq ] = 0;
  }
}

/*

On the Raspberry pi:
  i2cdetect -y 1
    Should show our knob at 0x42

  i2cget -y 1 0x42 0      -> 0xc2   -> 0x80 | 0x42
  i2cget -y 1 0x42 1      -> 0xd3   -> 0x80 | 'S'
  i2cget -y 1 0x42 2      -> 0xcc   -> 0x80 | 'L'
  i2cget -y 1 0x42 3      -> 0xcc   -> 0x80 | (knob delta, CW=+  ACW=-)
  i2cget -y 1 0x42 4      -> 0xcc   -> 0x80 | (button pressed = 1)
  i2cget -y 1 0x42 5      -> 0xcc   -> 0x80 | (# button presses since last time asked)
 */
