;
#include "config.h"
#include "chip.h"
#include "pinout.h"

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
void setup() { // run once, when the sketch starts

    // Initialize most important hardware, the rest waits til after target type is known
    pinMode(VCC, OUTPUT);  // Target Supply
    pinMode(RST, OUTPUT);  // Control of DC-DC converter that generates +12V !RESET
    digitalWrite(RST, OFF12v);  // Turn off 12V, Reset=0

    digitalWrite(VCC, LOW); // DJT: OK if no chip inserted
    //digitalWrite(VCC, HIGH); // DJT: Alternative if chip inserted already

    pinMode(BUTTON, INPUT); // User button to start programming
    digitalWrite(BUTTON, HIGH);  // turn on internal pullup resistor
    Serial.begin(BAUD);  // Open serial port, this works on the Mega also because we are using serial port 0
    delay(200);
    Serial.println("\n*** AVR HV Rescue More v1.0");
    target_exit_program_mode();     // be on the safe side....
}


//*** let us Loop ****************************************************************
//********************************************************************************
void loop() {  // run over and over again

		int device,dev_hvsp,dev_hvpp,g,key, searchon=0;
    byte function_result;


// try to read signature
// first using HVSP and -if not successful - using HVPP
// if valid AVR signature found (starting with 0x1E) try to find a match in the internal table
// if no AVR signature found give appropriate error message
// leave while loop only if valid AVR signature found

  do {								// loop until valid AVR signature found

		Serial.print("\n-->Insert Device and press button");
		Serial.print("\n   will try to find valid AVR device...\n");
    Serial.end();

    PAGEL = A5;  // ATtiny2313: PAGEL = BS1
      BS2 = 9;     // ATtiny2313: BS2 = XA1

    // wait for button press, debounce
    while(1) {
    	while (digitalRead(BUTTON) == HIGH);  // wait here until button is pressed
      	delay(100);                            // simple debounce routine
        if (digitalRead(BUTTON) == LOW)       // if the button is still pressed, continue
            break;  // valid press was detected, continue on with rest of program
		}

    dev_hvsp=0;								// set device type to zero
    dev_hvpp=0;								// set device type to zero
		// Try to read signature with HVSP first
		// Put target device into program mode
		mode =HVSP;
  	target_enter_program_mode(mode);
		// read and print Signature and Fuses, also establishes device type (variable device)
		dev_hvsp=read_Chip_data();    // returns device index (in internal table) --> opens Serial !

		device=dev_hvsp;							// returns device type

 		if (dev_hvsp==255) {        // no valid AVR signature found using HVSP
                                // now try HVPP
				Serial.end();
		 		mode=HVPP;
	  		target_enter_program_mode(mode);

				// read and print Signature and Fuses, also establishes device type (variable device)
				dev_hvpp=read_Chip_data();             //

				if (dev_hvpp==255) {    // no valid AVR signature found using HVPP
																// no valid device found, we gave our best and tried HVSP and HVPP
						target_exit_program_mode(); // Place after serial io, so Vcc is still on and prevents corruption
	//					Serial.println("\n\n*** no device found, nothing to do");
				}
				else device=dev_hvpp;
			}

    // Open serial port again to print existing values
      Serial.begin(BAUD);
    if ((dev_hvsp==254) | (dev_hvpp==254)) {            // check if one of them resultet in device 254
			Serial.print("\n*** AVR device found but is not in table");
			Serial.print("\n*** full functionality only if device is defined");
      Serial.print("\n*** in internal device table, please extend table");
      Serial.print("\n");
		}

  } while (device==255);    // go back to square one... no device found as yet


// we have found a valid AVR device, program-mode is still set to method used in finding the signature

	delay (50);


  #if (DEBUG == 1)

// prepare defaultfuses, variable device holds device table index
// get default fuses for device
    lfuse = pgm_read_byte(&targetCpu[device].fuseLowBits);
    hfuse = pgm_read_byte(&targetCpu[device].fuseHighBits);
    efuse = pgm_read_byte(&targetCpu[device].fuseExtendedBits);
		Serial.println("Data");
  	Serial.println(lfuse,HEX);
   	Serial.println(hfuse,HEX);
    Serial.println(efuse,HEX);
  	Serial.println(dev_hvsp);
 		Serial.println(dev_hvpp);
 		Serial.println("Dataend");

 #endif

//  Loop until user selects X on the menu  (safely remove device)
//  otherwise multiple functions can be run on the recognized device

  do              // yeah, just do it baby...
  {
    // display function menu and get function required
		//			- display full menu if device recognized
		//			- display short menu if device not found in array (no default fuses available)
		//      - 254 means: valid AVR signature but no match in table
    //      - can offer only limited set of functions for set fuses !

		print_Chip_data();								// output chip data to serial

		if (device==254) {
			cmd=*get_choice(menu_fs,0);			// display short menu
		}
		else {
			cmd=*get_choice(menu_ff,0);			// display full menu
		}

		if (cmd=='X' ) {
      target_exit_program_mode(); // Place after serial io, so Vcc is still on and prevents corruption

      Serial.println(" Remove device before continuing");
      break;
		}


	// get additional data based on selected function
		get_add_data();


    // This business with TXC0 is required because Arduino doesn't give us a means to tell if a serial
    // transmission is complete before we move on and do other things.  If we don't wait for TXC0 to be reset,
    // I found that sometimes the 1st fuse burn would fail.  It turns out that DATA1 (which doubles as Arduino serial
    // TX) was still toggling by the time the 1st XTAL strobe latches the fuse program command.  Bad news.
    //UCSR0A |= _BV(TXC0);  // Reset serial transmit complete flag (need to do this manually because TX interrupts aren't used by Arduino)
    //while(!(UCSR0A & _BV(TXC0)));  // Wait for serial transmission to complete before burning fuses!
    // The above will no longer work with Arduino 1.0 which buffers transmit data
    Serial.flush(); // In Arduino 1.0 this will wait for transmission to finish (prior to that will empty receive buffer so we reply on the delay after serial end to ensure TX finished)
    Serial.end();    // We're done with serial comms (for now) so disable UART
    delay(200);

		target_enter_program_mode(config[PGMODE]);
		function_result = run_function();             // do the function

    Serial.begin(BAUD);  // open serial port
    // Print result
    Serial.write('\n');
    if (function_result==DONEDEFAULT)
        Serial.println(" Done"); // Non commital result where functions doesn't check (default)
    if (function_result==SUCCESS)
        Serial.println(" OK"); // Success
    if (function_result==FAILURE)
        Serial.println(" Fail"); // Failure

    Serial.flush(); // In Arduino 1.0 this will wait for transmission to finish (prior to that will empty receive buffer so we reply on the delay after serial end to ensure TX finished)
  	Serial.end();    // We're done with serial comms (for now) so disable UART
    delay(200);

		target_exit_program_mode(); // Place after serial io, so Vcc is still on and prevents corruption
  	delay(50);
		target_enter_program_mode(config[PGMODE]);

// read and print Signature and Fuses again , also establishes device type (variable device)
    device=read_Chip_data();            // opens serial again
  Serial.begin(BAUD);  // Open serial port, this works on the Mega also because we are using serial port 0

  }  while (1);					// loop until user selects X  (remove device)

 	Serial.begin(BAUD);  // Open serial port, this works on the Mega also because we are using serial port 0

}
// End of loop *******************************************************
//********************************************************************************




//**************************************************************
void target_dump(const char* title, byte command, int address, int len){
    int i=0,n;
    n=address;
    do{
        if (command==HV_CMD_READ_FLASH)
            tbuffer[i++]=target_io(command, n, 0, MSB_R); //Byte buffer order MSB-LSB MSB-LSB .....
        tbuffer[i++]=target_io(command, n++, 0, LSB_R);
    }
    while (i<len);
    delay(10);
    Serial.begin(BAUD);  // open serial port
    delay(10);

    Serial.write('\n');
    Serial.print(title);
    print_fixed_hex(highByte(address));
    print_fixed_hex(lowByte(address));
    i=0;
    do {
        Serial.write(' ');
        if (command==HV_CMD_READ_FLASH)
            print_fixed_hex(tbuffer[i++]); // MSB
        print_fixed_hex(tbuffer[i++]);
    }
    while (i<len);
    Serial.write('\n');
    delay(10);
    Serial.end();
    delay(10);
}


//**************************************************************
void target_enter_program_mode(int mod){

    // Initialize pins to enter programming mode
    digitalWrite(RST, OFF12v);   // Reset must be LOW
    digitalWrite(VCC, LOW); // VCC must be off

    if(mod == HVSP) {
        // Set necessary pin values to enter programming mode
        pinMode(SDI, OUTPUT);
        digitalWrite(SDI, LOW);
        pinMode(SII, OUTPUT);
        digitalWrite(SII, LOW);
        pinMode(SDO, OUTPUT);   // normally an output but
        digitalWrite(SDO, LOW);  // needs to be low to enter programming mode
        // Setup other pins needed during prog
        pinMode(SCI, OUTPUT);
        digitalWrite(SCI, LOW);  // set clock low
    }
    else{
        // Set necessary pin values to enter programming mode
        pinMode(PAGEL, OUTPUT);
        digitalWrite(PAGEL, LOW);
        pinMode(XA1, OUTPUT);
        digitalWrite(XA1, LOW);
        pinMode(XA0, OUTPUT);
        digitalWrite(XA0, LOW);
        pinMode(BS1, OUTPUT);
        digitalWrite(BS1, LOW);
        pinMode(WR, OUTPUT);
        digitalWrite(WR, LOW);  // ATtiny2313 needs this to be low to enter programming mode, ATmega doesn't care

        // Setup other pins needed during prog
        data_as_input();
        pinMode(RDY, INPUT);
        pinMode(XTAL1, OUTPUT);
        digitalWrite(XTAL1, LOW);
        pinMode(BS2, OUTPUT);
        digitalWrite(BS2, LOW);
        pinMode(OE, OUTPUT);
        digitalWrite(OE, LOW);
    }

    // Enter programming mode
    digitalWrite(VCC, HIGH);  // Apply VCC to start programming process
    delayMicroseconds(80);
    digitalWrite(RST, ON12v);   // Apply 12V to !RESET thru level shifter

    if(mod == HVSP) {
        // reset SDO after short delay, longer leads to logic contention because target sets SDO high after entering programming mode
        delayMicroseconds(1);   // datasheet says 10us, 1us is needed to avoid drive contention on SDO
        pinMode(SDO, INPUT);    // set to input to avoid logic contention
    }
    else{
        delayMicroseconds(10);  // Give lots of time for part to enter programming mode
        digitalWrite(OE, HIGH);
        digitalWrite(WR, HIGH);   // Now that we're in programming mode we can disable !WR
    }
    delay(1);
}

//**************************************************************
void target_exit_program_mode(void){
    // Exit programming mode
    digitalWrite(RST, OFF12v);
    delay(1);

    // Turn off pins driving chip
    if(mode == HVSP) {
        pinMode(SDI, INPUT);
        digitalWrite(SDI, LOW);   // with no pullup
        pinMode(SII, INPUT);
        digitalWrite(SII, LOW);
        pinMode(SCI, INPUT);
        digitalWrite(SII, LOW);
        //pinMode(SDO, INPUT);   // is an input anyway
    }
    else{
        data_as_input(); // Should be input anyway
        digitalWrite(OE, LOW);
        digitalWrite(WR, LOW);
        digitalWrite(PAGEL, LOW);
        digitalWrite(XA1, LOW);
        digitalWrite(XA0, LOW);
        digitalWrite(BS1, LOW);
        digitalWrite(BS2, LOW);
    }
    digitalWrite(VCC, LOW); // Turn off power
}

//**************************************************************
void load(byte type, byte data){ // Load Commands and data into target

    if (mode != HVSP) { // HVPP programming only
        // Set controls for requested transaction type
        digitalWrite(BS2, LOW);  // Not relevant for load but should be low, must set first as shared on tinyX313
        digitalWrite(XA1, b2hilo(type & 0x04));
        digitalWrite(XA0, b2hilo(type & 0x02));
        digitalWrite(BS1, b2hilo(type & 0x01));

        // Output data
        delay(1);
        data_as_output(data);
        strobe_xtal();  // latch DATA
        data_as_input(); // reset DATA to input to avoid bus contentions
    }
    else
    {
        HVSP_sio(data, ((type<<4) +12));
    }
}

//**************************************************************
byte strobe(byte select){ // Do a WE or OE or PAGEL strobe, returns value on OE
    byte value=0;
    if (mode != HVSP) { // HVPP programming only

        // Activate Strobe, Made complicated by fact BS2 & PAGEL shared on Tiny2313
        if (select & MASK_PAGEL){
            digitalWrite(WR, HIGH );  // Setup WR
            digitalWrite(OE, HIGH );  // Setup OE
            digitalWrite(BS1, LOW );  // Setup BS1
            digitalWrite(BS2, LOW );  // Setup BS2
            digitalWrite(PAGEL, HIGH);  // Setup PAGEL
        }
        else{ // WR or OE use BS1 & BS2
            digitalWrite(PAGEL, LOW);  // PAGEL inactive
            digitalWrite(BS1, b2hilo(select & MASK_BS1) );  // Setup BS1
            digitalWrite(BS2, b2hilo(select & MASK_BS2) );  // Setup BS2
            delay(1);
            // Now make required strobe active
            digitalWrite(WR, b2hilo(select & MASK_WE) );  // Setup WE, note active low
            digitalWrite(OE, b2hilo(select & MASK_OE) );  // Setup OE, note active low
        }
        delay(1);

        // Capture data on reads
        if ((select & MASK_OE)==0)
            value=data_read();  // read value

        // Now make all strobes inactive
        digitalWrite(WR,HIGH);
        digitalWrite(OE,HIGH);
        digitalWrite(PAGEL,LOW);

        // Wait for writes to complete
        if ((select & MASK_WE)==0){
            while(digitalRead(RDY) == LOW);  // when RDY goes high, burn is done
        }
    }
    else
    { // HVSP programming only, uses same serial protocol as load
        HVSP_sio(0, ((POST_OP<<4)|select)); // select will contain appropriate WE or OE or PAGEL setting
        value=HVSP_sio(0, ((POST_OP<<4) | select | MASK_WE_OE) & (~MASK_PAGEL)); // Get result, ensure strobes turned off
        // Wait for writes to complete
        if ((select & MASK_WE)==0){
            while(digitalRead(SDO) == LOW); // Wait for operation to complete
        }
    }
    return value;
}

// Carry out a transaction with the target, all except Flash & EEPROM write because they are paged
//**************************************************************
byte target_io(byte command, int address, byte data, byte select) {
    byte value;

    // Load the command and parameters into target
    load(LOAD_COMMAND,command);  // Send command
    if (command & (HV_CMD_READ_FLASH|HV_CMD_READ_EEPROM))
        load(LOAD_ADDRESS_H,highByte(address)); // Load MSB address if required into target
    if (command & (HV_CMD_READ_SIG|HV_CMD_READ_FLASH|HV_CMD_READ_EEPROM))
        load(LOAD_ADDRESS,lowByte(address)); // Load LSB address if required into target
    if (command & (HV_CMD_WRITE_FUSE|HV_CMD_WRITE_LOCK|HV_CMD_WRITE_FLASH|HV_CMD_WRITE_EEPROM))
        load(LOAD_DATA,lowByte(data)); // Load LSB value into target

    // Now do strobe to carry out operation
    value=strobe(select);
    return value;
}

// Carry out a page transaction with the target chip.
//   address MUST be the start of a page block
//**************************************************************
void target_page_write(byte command, int address, byte* data, byte count) {
    byte i;

    // Load the command and parameters into target
    load(LOAD_COMMAND,command);  // Send command
    if (command==HV_CMD_WRITE_EEPROM){
        load(LOAD_ADDRESS_H,highByte(address)); // Load MSB address
        for (i=0;i<count;i++){
            load(LOAD_ADDRESS,lowByte(address)); // Load LSB address
            load(LOAD_DATA,lowByte(*data++)); // Load LSB value into target
            strobe(EE_PAGE_LATCH);
            address++;
        }
    }
    else{
        load(LOAD_ADDRESS_H,highByte(address)); // Load MSB address
        for (i=0;i<count;i++){
            load(LOAD_ADDRESS,lowByte(address)); // Load LSB address
            load(LOAD_DATA,lowByte(*(data+1))); // Load LSB (Byte buffer order MSB-LSB ... MSB-LSB)
            load(LOAD_DATA_H,lowByte(*data)); // Load MSB value into target
            data+=2; // Increment to next word
            strobe(EE_PAGE_LATCH | 0x10);
            address++;
        }
    }
    // Now do WR strobe to carry out operation
    strobe(SEL_WR);
    load(LOAD_COMMAND,0);  // Send NOP command as per some datasheets
}

// Notes on Page writing:
//    HVSP EEPROM seems ok to write AddrH first and only once per page
//    Tiny2313 doesn't need PAGE_LATCH pulse but is ok if it is used
//    OK to write FLASH AddrH before loading page
//    HVSP PAGEL needs BS1 (?BS2) to be set
//**************************************************************
byte HVSP_sio(byte datab, byte instrb) { // Do a Data & Instruction transfer and capture a byte using the HVSP protocol
    byte response = 0x00; // the response from target
    int data,instr; //       0 XA1 XA0 BS1   WE OE BS2 (PAGE)

    data= (int)datab<<7; // Pad required bits with leading 0 and 2 trailing 0 as per prog spec
    instr= (int)instrb<<7;

    // We capture a response on every read even though only certain responses contain
    // valid data.  For fuses, the valid response is captured on the 3rd instruction write.
    // It is up to the program calling this function to figure out which response is valid.

    // The MSB of the response byte comes "early", that is,
    // before the 1st non-zero-padded byte of the 3rd instruction is sent to the target.
    // For more information, see the ATtiny25/45/85 datasheet, Table 20-16 (pg. 164).

    // Send each bit of padded data and instruction byte serially, MSB first
    for (int i=0; i<11; i++) {  // i is bit number
        digitalWrite(SDI, int2hilo(data & 0x8000)); // Set SDI to represent next bit in data
        data<<=1;
        digitalWrite(SII, int2hilo(instr & 0x8000));  // Set SII to represent next bit in instruction
        instr<<=1;
        sclk();

        if (i < 8) {  // remaining 7 bits of response are read here (one at a time)
            if(digitalRead(SDO) == HIGH)  // if we get a logic 1 from target,
                response |= (0x80 >> i);    // set corresponding bit of response to 1
        }
    }
    return response;
}


#ifdef ARDUINO_AVR_MEGA  // functions specifically for the Arduino Mega

//**************************************************************
void mega_data_write(byte data) { // Write a byte to digital lines 0-7
    // This is really ugly, thanks to the way that digital lines 0-7 are implemented on the Mega.
    PORTE &= ~(_BV(PE0) | _BV(PE1) | _BV(PE4) | _BV(PE5) | _BV(PE3));  // clear bits associated with digital pins 0-1, 2-3, 5
    PORTE |= (data & 0x03);  // set lower 2 bits corresponding to digital pins 0-1
    PORTE |= (data & 0x0C) << 2;  // set PORTE bits 4-5, corresponding to digital pins 2-3
    PORTE |= (data & 0x20) >> 2;  // set PORTE bit 5, corresponding to digital pin 5
    DDRE |= (_BV(PE0) | _BV(PE1) | _BV(PE4) | _BV(PE5) | _BV(PE3));  // set bits we are actually using to outputs

    PORTG &= ~(_BV(PG5));  // clear bits associated with digital pins 4-5
    PORTG |= (data & 0x10) << 1;  // set PORTG bit 5, corresponding to digital pin 4
    DDRG |= (_BV(PG5));  // set to output

    PORTH &= ~(_BV(PH3) | _BV(PH4));  // clear bites associated with digital pins 6-7
    PORTH |= (data & 0xC0) >> 3;  // set PORTH bits 3-4, corresponding with digital pins 6-7
    DDRH |= (_BV(PH3) | _BV(PH4));  // set bits to outputs
}

//**************************************************************
byte mega_data_read(void) { // Read a byte from digital lines 0-7
    byte data = 0x00;  // initialize to zero
    data |= (PINE & 0x03);  // set lower 2 bits
    data |= (PINE & 0x30) >> 2;  // set bits 3-4 from PINE bits 4-5
    data |= (PINE & 0x08) << 2;  // set bit 5 from PINE bit 3
    data |= (PING & 0x20) >> 1;  // set bit 4 from PING bit 5
    data |= (PINH & 0x18) << 3;  // set bits 6-7 from PINH bits 3-4
    return data;
}

//**************************************************************
void mega_data_input(void) { // Set digital lines 0-7 to inputs and turn off pullups
    PORTE &= ~(_BV(PE0) | _BV(PE1) | _BV(PE4) | _BV(PE5) | _BV(PE3));  // Mega digital pins 0-3, 5
    DDRE &= ~(_BV(PE0) | _BV(PE1) | _BV(PE4) | _BV(PE5) | _BV(PE3));  // Set to input
    PORTG &= ~(_BV(PG5));  // Mega digital pin 4
    DDRG &= ~(_BV(PG5));  // Set to input
    PORTH &= ~(_BV(PH3) | _BV(PH4));  // Mega digital pins 6-7
    DDRH &= ~(_BV(PH3) | _BV(PH4));  // Set to input
}
#endif

//**************************************************************
void sclk(void) {  // send serial clock pulse, used by HVSP commands
    // These delays are much  longer than the minimum requirements, but we don't really care about speed.
    delayMicroseconds(100);
    digitalWrite(SCI, HIGH);
    delayMicroseconds(100);
    digitalWrite(SCI, LOW);
}

//**************************************************************
void strobe_xtal(void) {  // strobe xtal (usually to latch data on the bus)
    delay(1);
    digitalWrite(XTAL1, HIGH);  // pulse XTAL to send command to target
    delay(1);
    digitalWrite(XTAL1, LOW);
}

//**************************************************************
void data_as_input(void) { // Set the data port as an input
#ifndef ARDUINO_AVR_MEGA  // Set up data lines on original Arduino
    PORTD = 0x00;  // clear digital pins 0-7
    DDRD = 0x00;  // set digital pins 0-7 as inputs for now
#else  // Mega uses different ports for same pins, much more complicated setup.  yuck!
    mega_data_input();
#endif
}

//**************************************************************
void data_as_output(byte data){ // Set the data port as an output with data
#ifndef ARDUINO_AVR_MEGA
    PORTD = data;
    DDRD = 0xFF;  // Set all DATA lines to outputs
#else
    mega_data_write(data);
#endif
}

//**************************************************************
byte data_read(void) { // Capture value on data port
    byte value;
#ifndef ARDUINO_AVR_MEGA
    value = PIND;
#else
    value = mega_data_read();
#endif
    return value;
}

//**************************************************************
byte int2hilo(int n) // Converts an int to HIGH or LOW
{
    if (n)
        return HIGH;
    return LOW;
}

//**************************************************************
byte b2hilo(byte n) // Converts a byte to HIGH or LOW
{
    if (n)
        return HIGH;
    return LOW;
}

//**************************************************************
int hex2dec(byte c) { // converts one HEX character into a number
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'f') { // Added to allow lower case hex
        return c - 'a' + 10;
    }
    return -1;
}

// Simple printf substitute
//**************************************************************
void printf(const char* format, byte* param){ // Format string usings % to print 2 digit hex from param array
    char c;
    while ((bool)(c=*format++)){
        if (c=='%'){
            if (*param<0x10)
                Serial.print("0");
            Serial.print(*param++, HEX);
        }
        else
            Serial.write(c);
    }
}

//**************************************************************
void print_fixed_hex(int value){ // Prints a hex value as two fixed digits
    //Serial.print(" ");
    if (value<0x10)
        Serial.print("0");
    Serial.print(value, HEX);
}

// Get hex value from the user, handles upper and lower case & errors. Optional 0x prefix ignored
//**************************************************************
int get_value(const char* prompt, int def ) {  // def parameter not yet used, could provide default input
    int incomingByte;
    int n,m,v=0;
    byte err;

    // Prompt
    Serial.print(prompt);

    do {
       delay(100);
       while (Serial.available())   // Clear buffer of any input ie flush
            Serial.read();
        // Serial.flush(); // Not supported in Arduino 1.0
        Serial.print(" ?");

        // Wait for input
        do
            n=Serial.available();
        while (n==0);

        // Wait for input to finish (timeout) . Allow variable length input without terminator
        do {
            delay(100);
            m=n;
            n=Serial.available();
        }
        while (m!=n);

        // Process characters recieved
        err=0; //Assume no errors in input
        while (!err && ((incomingByte = Serial.read()) != -1)){
            if  (incomingByte=='\n' )
                break;
            if  (incomingByte=='\r' )
                break;
            if  ((v==0) && (incomingByte=='0') && (Serial.peek()=='x'))
                incomingByte = Serial.read();    // Ignore 0x prefix at start only
            else if  (hex2dec(incomingByte)==-1)
                err=1;
            else
                v=v*16 + hex2dec(incomingByte);
        }
    }
    while(err); // Repeat until no errors

    Serial.println(v, HEX);  // echo value back to the user
    return v;
}

// Print a simply menu and get user selection of option
//    options = Array of strings, [0] is the intro, others prompt with selection char first
//**************************************************************
const char* get_choice(const char* options[], char auto_cmd){
    int i=0,j,k;
    char c;

    Serial.println(options[0]); // First item is Menu Title
    while(options[++i]!=0){
        Serial.write(' '); // Indent
        Serial.println(options[i]); // Print each option
    }

    k=0;  // Will be set to index of option selected
    do {
        while (Serial.available())   // Clear buffer of any imput ie flush
            Serial.read();
        // Serial.flush(); // Not supported in Arduino 1.0

        if (k==0)
            Serial.write('?'); // Prompt the user if no choice made yet

        if (auto_cmd){
            c=auto_cmd; // Use the auto input
            auto_cmd=0; //  but only first time
        }
        else {
            while (Serial.available() == 0);   // wait for a character to come in
            c=toUpperCase(Serial.read());
        }

        // Search to see if entry valid
        for (j=1;j<i;j++)
            if (c==*options[j])
                k=j;
    }
    while (k==0);
    Serial.write(c); // echo user input
    Serial.write('\n');
    Serial.write(options[k]+2); // Print chosen option
    Serial.println(" selected");
    return (options[k]); // Return pointer to selected option
}


// These functions added or existing code put into functions Sept. 2016 ---------------------------------

// Find device in array based on signature read
//**************************************************************
int findDevice()
{
// loop through all devices until found (or not)
    	for (device = 0; (int) pgm_read_byte(&targetCpu[device].signature[0])!=0 ; device++)
    	{

    		if (config[SIG1]  == pgm_read_byte(&targetCpu[device].signature[0]) &&
    			  config[SIG2]  == pgm_read_byte(&targetCpu[device].signature[1]) &&
    			  config[SIG3]  == pgm_read_byte(&targetCpu[device].signature[2])	)  {
         return(device);
  			}
			}
        return (254);															// no match in internal device table
}
// end find device



//  read signature and fuse bytes from device
// try to match signature with signatures in internal device table
//**************************************************************
int read_Chip_data(void)
{

    delay(10);
	// Read the Signature
	config[SIG1] = target_io(HV_CMD_READ_SIG,0,0,SIG_SEL_R);
	config[SIG2] = target_io(HV_CMD_READ_SIG,1,0,SIG_SEL_R);
	config[SIG3] = target_io(HV_CMD_READ_SIG,2,0,SIG_SEL_R);


     // Hack for ATtiny 2313: Setup AVR interface
     // ATtiny2313: PAGEL = BS1
     // ATtiny2313: BS2 = XA1

    if (config[SIG1]  == 0x1E &&				// check if signature for ATtiny2313
        config[SIG2]  == 0x91 &&
        config[SIG3]  == 0x0A)  {
          PAGEL = BS1; 							// reassign PAGEL and BS2 to their combined counterparts on the '2313
          BS2 = XA1;
    }
    else {
      	PAGEL = A5;  // ATtiny2313: PAGEL = BS1
      	BS2 = 9;     // ATtiny2313: BS2 = XA1
    }

	// Get current fuse settings stored on target device
	config[LFUSE] = target_io(HV_CMD_READ_FUSE_LOCK,0,0,LFUSE_SEL_R);
	config[HFUSE] = target_io(HV_CMD_READ_FUSE_LOCK,0,0,HFUSE_SEL_R);
	config[EFUSE] = target_io(HV_CMD_READ_FUSE_LOCK,0,0,EFUSE_SEL_R);
	config[LOCK] = target_io(HV_CMD_READ_FUSE_LOCK,0,0,LOCK_SEL_R);

	if (config[SIG1]!=0x1E) {
		return (255);						// needs to be 0x1E for AVR MCU
	}

	device=findDevice();							// try to find device in device table
	efuse_present=1;				           // default value

	if (device<254) {									// device<254: found a match in table
		efuse_present=pgm_read_byte(&targetCpu[device].fuseExtendedBits);				// 0: no extended fuses for this chip
		config[PGMODE] = pgm_read_byte(&targetCpu[device].programming_mode);		// get programming mode for this chip
	}
	return(device);
}

//  print chip data
// try to match signature with signatures in internal device table
//**************************************************************
void print_Chip_data(void)
{

	Serial.print("\n-->Current Chip Data");
	printf("\n Signature:      % % %", &config[SIG1]);
	Serial.print("  Device: ");
	if (device<254) 									// AVR device found in table
		Serial.print( (PGM_P)pgm_read_word(&targetCpu[device].mcutype));
	else 			Serial.print("Device not defined");

	if (efuse_present)                                   //
			printf("\n Fuses Lo/Hi/Ex: % % %", &config[LFUSE]);
	else
			printf("\n Fuses Lo/Hi:    % %", &config[LFUSE]);

	printf("\n Lock Bits:      %", &config[LOCK]);
	Serial.print("\n");

}

// run function according to function selected by user
//**************************************************************
int run_function() {
int i;
byte result=DONEDEFAULT; // Default response to 'done';
	// Now run selected function
	switch(cmd){
	case 'E': // Chip Erase
			target_io(HV_CMD_CHIP_ERASE,0,0,SEL_WR); // Perform a chip erase
			break;
	case 'F': // Write fuses with user values
	case 'D': // Write default fuses
	case 'H': // Write 8 Mhz fuses
	case 'Q': // Write fuses ext. crystal
			target_io(HV_CMD_WRITE_FUSE,0,lfuse,LFUSE_SEL_W);
			target_io(HV_CMD_WRITE_FUSE,0,hfuse,HFUSE_SEL_W);
			if (efuse_present)
					target_io(HV_CMD_WRITE_FUSE, 0, efuse, EFUSE_SEL_W);

			// Read back fuse contents to verify burn worked
			config[LFUSE] = target_io(HV_CMD_READ_FUSE_LOCK,0,0,LFUSE_SEL_R);
			config[HFUSE] = target_io(HV_CMD_READ_FUSE_LOCK,0,0,HFUSE_SEL_R);
			if (efuse_present)
					config[EFUSE]  = target_io(HV_CMD_READ_FUSE_LOCK,0,0,EFUSE_SEL_R);

			// Check result
			if (config[LFUSE]!=lfuse || config[HFUSE]!=hfuse || ( efuse_present && (config[EFUSE]!=efuse)))
					result=FAILURE;
			else
					result=SUCCESS;
			break;

	case 'R': // Read and print block of FLASH
			for (i=0;i<64;i+=8)
					target_dump("FLASH", HV_CMD_READ_FLASH, start_addr+i, 16); // Read 16 bytes = 8 words
			break;
	case 'P': // Read and print block of EEPROM
			for (i=0;i<128;i+=16)
					target_dump("EEPROM", HV_CMD_READ_EEPROM, start_addr+i, 16);
			break;
	case 'W': // Test write FLASH
			target_page_write(HV_CMD_WRITE_FLASH, start_addr, test_fl, 16); // Write 16 bytes = 8 words, must be equal or less than device page size. Arduino ram limits prevent us writing a whole page for now
			result=SUCCESS; // Assume it worked unless test show otherwise
			for (i=0;i<8;i++){
					if ((test_fl[i*2]*256+test_fl[i*2+1])!=(target_io(HV_CMD_READ_FLASH,  start_addr+i, 0, MSB_R)*256+target_io(HV_CMD_READ_FLASH,  start_addr+i, 0, LSB_R)))
							result=FAILURE;
			}
			break;
	case 'T': // Test write EEPROM
			target_page_write(HV_CMD_WRITE_EEPROM, start_addr, test_ee, 4); // Use EEPROM page size of 4 (Correct for all AVR we are likely to see
			result=SUCCESS; // Assume it worked unless test show otherwise
			for (i=0;i<4;i++){
					if (test_ee[i]!=target_io(HV_CMD_READ_EEPROM,  start_addr+i, 0, LSB_R))
							result=FAILURE;
			}
			break;

	}
	return (result);
}

// get additional data from user, according to function selected
//**************************************************************
void get_add_data(){

	// Get Additional data, according to selected function
	if (cmd=='R' || cmd=='P' || cmd=='T'|| cmd=='W')
			start_addr=get_value("Address",0x00);
	else if (cmd=='F'){
			// Ask the user what fuse values should be burned to the target
			lfuse = get_value("LFUSE value",0x62);
			hfuse = get_value("HFUSE value",0xdf);
			if (efuse_present)
					efuse = get_value("EFUSE value",0xff);
	}
	else if (cmd=='D') {
		    // get default fuses for device
    lfuse = pgm_read_byte(&targetCpu[device].fuseLowBits);
    hfuse = pgm_read_byte(&targetCpu[device].fuseHighBits);
    efuse = pgm_read_byte(&targetCpu[device].fuseExtendedBits);

  #if (DEBUG == 1)
		Serial.print("\n Write Default Fuses: ");
		Serial.print(lfuse,HEX);
		Serial.print(" ");
		Serial.print(hfuse,HEX);
		Serial.print(" ");
		Serial.println(efuse,HEX);
  #endif
	}
	else if (cmd=='H') {

		// get 8 Mhz fuses for device
		lfuse = pgm_read_byte(&targetCpu[device].HifuseLowBits);
		hfuse = pgm_read_byte(&targetCpu[device].HifuseHighBits);
		efuse = pgm_read_byte(&targetCpu[device].HifuseExtendedBits);
  #if (DEBUG == 1)
		Serial.print("\n Write 8MHz Fuses: ");
		Serial.print(lfuse,HEX);
		Serial.print(" ");
		Serial.print(hfuse,HEX);
		Serial.print(" ");
		Serial.println(efuse,HEX);
  #endif
}
	else if (cmd=='Q') {
		// get fuses for ext. crystal for device
		lfuse = pgm_read_byte(&targetCpu[device].ExfuseLowBits);
		hfuse = pgm_read_byte(&targetCpu[device].ExfuseHighBits);
		efuse = pgm_read_byte(&targetCpu[device].ExfuseExtendedBits);
  #if (DEBUG == 1)
		Serial.print("\n Write ext. crystal Fuses: ");
		Serial.print(lfuse,HEX);
		Serial.print(" ");
		Serial.print(hfuse,HEX);
		Serial.print(" ");
		Serial.println(efuse,HEX);
  #endif
	}
}
// ******************************************************************
// End of program
// ******************************************************************
