
// Serial instructions for HVSP mode
// Based on the ATtiny85 datasheet Table 20-16 pg. 163-165.
// After analysis serial instructions can be built from the parallel ones

enum result_types {
    DONEDEFAULT, SUCCESS, FAILURE };

// Global variables
byte mode;  // programming mode
byte efuse_present;
byte tbuffer[64];
byte test_ee[]={0xaa,0x55,0xa5,0x5a}; // Four bytes
byte test_fl[]="AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPp"; // 16 bytes, 8 words. Upper case is MSB



// The function menu (full)
const char* menu_ff[]={
    "\n-->Select function:",
    "F: Write Custom Fuses",
    "D: Write Default Fuses",
    "H: Write 8Mhz Fuses",
    "Q: Write ext.Quarz Fuses",
    "E: Erase",
    "R: Read Flash",
    "P: Read EEPROM",
    "W: Test Flash",
    "T: Test EEPROM",
    "X: Remove MCU",

    0}; // end


// The function menu (short)
// used if mcu device could not be found in internal device table
const char* menu_fs[]={
    "\n-->Select function:",
    "F: Write Custom Fuses",
    "E: Erase",
    "R: Read Flash",
    "P: Read EEPROM",
    "W: Test Flash",
    "T: Test EEPROM",
    "X: Remove MCU",

    0}; // end

byte config[17]; // Used to hold data read from target
#define SIG1     0
#define SIG2     1
#define SIG3     2
#define LFUSE    3
#define HFUSE    4
#define EFUSE    5
#define LOCK     6
#define PGMODE   7

// These pin assignments change depending on which chip is being programmed,
// so they can't be set using #define
// There is probably a more elegant way to do this.  Suggestions?
byte PAGEL = A5;  // ATtiny2313: PAGEL = BS1
byte BS2 = 9;     // ATtiny2313: BS2 = XA1

char no_device_present;
byte hfuse=0xff, lfuse=0xff, efuse=0xff;  // desired fuse values from user
char cmd;
int start_addr=0;

//**************************************************************