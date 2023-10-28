// User defined settings

#define BAUD 9600  // Serial port rate at which to talk to PC
#define DEBUG 1    // set to 1 for debug output on serial
// Old type hardware with Transistor switching 12v
//#define OFF12v  HIGH
//#define ON12v LOW

// New version 2.1 hardware with DC-DC convertor
#define OFF12v LOW
#define ON12v HIGH

// Internal definitions
#define DEBUG 0  // set to 1 to produce debug output
#define ASK 0    // Note: it is 0 not '0' for no auto selection
#define HVPP 1
#define TINY2313 2
#define HVSP 3