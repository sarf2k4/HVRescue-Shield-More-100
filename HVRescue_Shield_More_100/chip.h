uint8_t device;   // holds found device type (array index)
/*
 * Struct to hold signature and default fuse bits of target to be programmed
 * use Atmel AVR� Fuse Calculator: http://www.engbedded.com/fusecalc/
 * Note: such a structure was first used by Peter Fleury in his ATtiny FuseRestore Project
 * http://homepage.hispeed.ch/peterfleury/avr-hvsp-fuse-restore.html
 *
 * structure expanded and used here.
 */

typedef struct {
	byte		signature[3];
	char 		*mcutype;
	uint8_t 	programming_mode;
	uint8_t		fuseLowBits;
	uint8_t		fuseHighBits;
	uint8_t		fuseExtendedBits;
  uint8_t		HifuseLowBits;
	uint8_t		HifuseHighBits;
	uint8_t		HifuseExtendedBits;
  uint8_t		ExfuseLowBits;
  uint8_t		ExfuseHighBits;
  uint8_t		ExfuseExtendedBits;
}TargetCpuInfo_t;

// DEVICE TABLE
// Array of controller types, held in program memory (let us call it: device table)
// extend if needed, sequence does not matter. simply copy/paste an entry
// end adjust values
// NOTE: Array must have a max of 253 entries or else sketch fails !!
//
// To add more mcu's to the array do this:  <-----------
//  get the datasheet for the mcu
//  calculate the fuses with a fuse calculator
//  find out if mcu has extended fuses
//  get the proper signature
//  get the proper name of the mcu
//  get the progamming mode: HVSP or HVPP
//  copy one existing entry and paste it in.
//  change the values in the new entry.
//  voila, that is it
//  run it
//
static const TargetCpuInfo_t	PROGMEM	targetCpu[] =
{
	{	// ATtiny13
		.signature	      = { 0x1E, 0x90, 0x07 },
		.mcutype					= "ATtiny13",
		.programming_mode = HVSP,
		.fuseLowBits	    = 0x6A,
		.fuseHighBits	    = 0xFF,
		.fuseExtendedBits	= 0x00,
    .HifuseLowBits	    = 0x7A,
		.HifuseHighBits	    = 0xFF,
		.HifuseExtendedBits	= 0x00,				// 0x00 means: no extended fuses written
    .ExfuseLowBits	    = 0x78,
		.ExfuseHighBits	    = 0xFF,
		.ExfuseExtendedBits	= 0x00,
	},
	{	// ATtiny24
		.signature	      = { 0x1E, 0x91, 0x0B },
		.mcutype					= "ATtiny24",
		.programming_mode = HVSP,
		.fuseLowBits	    = 0x62,
		.fuseHighBits	    = 0xDF,
		.fuseExtendedBits	= 0xFF,
    .HifuseLowBits	    = 0xE2,
    .HifuseHighBits	    = 0xDF,
    .HifuseExtendedBits	= 0xFF,
    .ExfuseLowBits	    = 0xFF,
		.ExfuseHighBits	    = 0xDF,
		.ExfuseExtendedBits	= 0xFF,
	},
	{	// ATtiny44
		.signature	      = { 0x1E, 0x92, 0x07 },
		.mcutype					= "ATtiny44",
		.programming_mode = HVSP,
		.fuseLowBits	    = 0x62,
		.fuseHighBits	    = 0xDF,
		.fuseExtendedBits	= 0xFF,
    .HifuseLowBits	    = 0xE2,
    .HifuseHighBits	    = 0xDF,
    .HifuseExtendedBits	= 0xFF,
    .ExfuseLowBits	    = 0xFF,
		.ExfuseHighBits	    = 0xDF,
		.ExfuseExtendedBits	= 0xFF,
	},
	{	// ATtiny84
		.signature	      = { 0x1E, 0x93, 0x0C },
		.mcutype					= "ATtiny84",
		.programming_mode = HVSP,
		.fuseLowBits	    = 0x62,
		.fuseHighBits	    = 0xDF,
		.fuseExtendedBits	= 0xFF,
    .HifuseLowBits	    = 0xE2,
    .HifuseHighBits	    = 0xDF,
    .HifuseExtendedBits	= 0xFF,
    .ExfuseLowBits	    = 0xFF,
		.ExfuseHighBits	    = 0xDF,
		.ExfuseExtendedBits	= 0xFF,
	},
	{	// ATtiny25
		.signature	      = { 0x1E, 0x91, 0x08 },
		.mcutype					= "ATtiny25",
		.programming_mode = HVSP,
		.fuseLowBits	    = 0x62,
		.fuseHighBits	    = 0xDF,
		.fuseExtendedBits	= 0xFF,
    .HifuseLowBits	    = 0xE2,
    .HifuseHighBits	    = 0xDF,
    .HifuseExtendedBits	= 0xFF,
    .ExfuseLowBits	    = 0xFF,
		.ExfuseHighBits	    = 0xDF,
		.ExfuseExtendedBits	= 0xFF,
	},
	{	// ATtiny45
		.signature	      = { 0x1E, 0x92, 0x06 },
		.mcutype					= "ATtiny45",
		.programming_mode = HVSP,
		.fuseLowBits	    = 0x62,
		.fuseHighBits	    = 0xDF,
		.fuseExtendedBits	= 0xFF,
    .HifuseLowBits	    = 0xE2,
		.HifuseHighBits	    = 0xDF,
		.HifuseExtendedBits	= 0xFF,
    .ExfuseLowBits	    = 0xFF,
		.ExfuseHighBits	    = 0xDF,
		.ExfuseExtendedBits	= 0xFF,
	},
	{	// ATtiny85
		.signature	      = { 0x1E, 0x93, 0x0B },
		.mcutype					= "ATtiny85",
		.programming_mode = HVSP,
		.fuseLowBits	    = 0x62,
		.fuseHighBits	    = 0xDF,
		.fuseExtendedBits	= 0xFF,
    .HifuseLowBits	    = 0xE2,
		.HifuseHighBits	    = 0xDF,
		.HifuseExtendedBits	= 0xFF,
    .ExfuseLowBits	    = 0xFF,
		.ExfuseHighBits	    = 0xDF,
		.ExfuseExtendedBits	= 0xFF,
	},
  {  // ATtiny2313A
    .signature        = { 0x1E, 0x91, 0x0A },
    .mcutype          = "ATtiny2313A",
    .programming_mode = HVPP,
    .fuseLowBits      = 0x64,
    .fuseHighBits     = 0xDF,
    .fuseExtendedBits = 0x00,
    .HifuseLowBits      = 0xE4,
    .HifuseHighBits     = 0xDF,
    .HifuseExtendedBits = 0x00,
    .ExfuseLowBits      = 0xFF,
    .ExfuseHighBits     = 0xDF,
    .ExfuseExtendedBits = 0x00,
  },
	{	// ATmega8A
		.signature	      = { 0x1E, 0x93, 0x07 },
		.mcutype					= "ATmega8",
		.programming_mode = HVPP,
		.fuseLowBits	    = 0xE1,
		.fuseHighBits	    = 0xD9,
		.fuseExtendedBits	= 0x00,			// 0x00 means: no extended fuses written
    .HifuseLowBits	    = 0xE4,
		.HifuseHighBits	    = 0xD9,
		.HifuseExtendedBits	= 0x00,		// 0x00 means: no extended fuses written
    .ExfuseLowBits	    = 0xFF,
		.ExfuseHighBits	    = 0xC9,
		.ExfuseExtendedBits	= 0x00,		// 0x00 means: no extended fuses written
	},
  {	// ATmega48A
    .signature	      = { 0x1E, 0x92, 0x05 },
    .mcutype					= "ATmega48",
		.programming_mode = HVPP,
    .fuseLowBits	    = 0x62,
    .fuseHighBits	    = 0xDF,
    .fuseExtendedBits	= 0xFF,
    .HifuseLowBits	    = 0xE2,
    .HifuseHighBits	    = 0xDF,
    .HifuseExtendedBits	= 0xFF,
    .ExfuseLowBits	    = 0xFF,
		.ExfuseHighBits	    = 0xDF,
		.ExfuseExtendedBits	= 0xFF,
  },
  {	// ATmega88
    .signature	      = { 0x1E, 0x93, 0x0A },
    .mcutype					= "ATmega88",
		.programming_mode = HVPP,
    .fuseLowBits	    = 0x62,
    .fuseHighBits	    = 0xDF,
    .fuseExtendedBits	= 0xF9,
    .HifuseLowBits	    = 0xE2,
    .HifuseHighBits	    = 0xDF,
    .HifuseExtendedBits	= 0xF9,
    .ExfuseLowBits	    = 0xF7,
		.ExfuseHighBits	    = 0xDF,
		.ExfuseExtendedBits	= 0xF9,
  },
	{	// ATmega168
		.signature	      = { 0x1E, 0x94, 0x06 },
		.mcutype					= "ATmega168",
		.programming_mode = HVPP,
		.fuseLowBits	    = 0x62,
		.fuseHighBits	    = 0xDF,
		.fuseExtendedBits	= 0xF9,
    .HifuseLowBits	    = 0xE2,
		.HifuseHighBits	    = 0xDF,
		.HifuseExtendedBits	= 0xF9,
    .ExfuseLowBits	    = 0xFF,
		.ExfuseHighBits	    = 0xDF,
		.ExfuseExtendedBits	= 0xF9,
	},
  {	// ATmega328P
    .signature	      = { 0x1E, 0x95, 0x0F },
		.mcutype					= "ATmega328P",
		.programming_mode = HVPP,
    .fuseLowBits	    = 0x62,
    .fuseHighBits	    = 0xD9,
    .fuseExtendedBits	= 0xFF,
    .HifuseLowBits	    = 0xE2,
		.HifuseHighBits	    = 0xD9,
		.HifuseExtendedBits	= 0xFF,
    .ExfuseLowBits	    = 0xFF,
		.ExfuseHighBits	    = 0xD9,
		.ExfuseExtendedBits	= 0xFF,
  },

	  // mark end of list
	{ { 0,0,0 }, 0, 0, 0 },
};
