/*
  Data line assignments
 Fuse and command data for HVPP mode are sent using Arduino digital lines 0-7

 Arduino Uno and other original-style form factor boards:
 Digital Line 0-7 outputs = PORTD
 Inputs = PIND
 Data direction register = DDRD

 Arduino Mega - much more complicated because digital lines 0-7 don't map directly to an AVR hardware port:
 Digital Line  AVR Signal Name
 0  PE0  (PORTE)
 1  PE1  (PORTE)
 2  PE4  (PORTE)
 3  PE5  (PORTE)
 4  PG5  (PORTG)
 5  PE3  (PORTE)
 6  PH3  (PORTH)
 7  PH4  (PORTH)
 */

// Pin Assignments (you shouldn't need to change these)
#define  VCC     12
#define  RDY     13     // RDY/!BSY signal from target
#define  OE      11
#define  WR      10
#define  BS1     A2
#define  XA0     8
#define  XA1     A4
#define  RST     A0    // 12V Step up converter enable (12V_EN)
#define  XTAL1   A3
#define  BUTTON  A1    // Run button

// Pin assignments for HVSP mode
#define  SCI    BS1
#define  SDO    RDY
#define  SII    XA0
#define  SDI    XA1


// Commands for HV prog mode, used for both PP and SP, comment shows usage
#define HV_CMD_CHIP_ERASE     B10000000 // CMD *WR
#define HV_CMD_WRITE_FUSE     B01000000 // CMD DATAL *WR
#define HV_CMD_WRITE_LOCK     B00100000 // CMD DATAL *WR
#define HV_CMD_WRITE_FLASH    B00010000 //  CMD ({ADDRL DATAL DATAH *PG} ADDRH *WR) NOP
#define HV_CMD_WRITE_EEPROM   B00010001 //  CMD ADDRH {ADDRL DATAL *PG} *WR // HVSP shows {ADDRL ADDRH but other seems ok
#define HV_CMD_READ_SIG       B00001000 // CMD ADDRL *RD
#define HV_CMD_READ_FUSE_LOCK B00000100 // CMD *RD
#define HV_CMD_READ_FLASH     B00000010 // CMD ADDRH ADDRL *RD // HVSP shows L first but H seems only
#define HV_CMD_READ_EEPROM    B00000011 // CMD ADDRH ADDRL *RD // HVSP shows L first but H seems only

// Load type bitmap for HV prog: (0 0 0 0 0) XA1 XA0 BS1
//  Used as is for HVSP, HVPP decodes bits to setup ports
#define LOAD_ADDRESS   B00000000
#define LOAD_ADDRESS_H B00000001
#define LOAD_DATA      B00000010
#define LOAD_DATA_H    B00000011
#define LOAD_COMMAND   B00000100
#define POST_OP        B00000110 // Only used for HVSP

// Post-op bitmap for HV Prog: (0 0 0) BS1 WE OE BS2 PAGEL
//  Used as is for HVSP, HVPP decodes bits to setup ports
#define LFUSE_SEL_R     B00001000
#define LFUSE_SEL_W     B00000100
#define HFUSE_SEL_R     B00011010
#define HFUSE_SEL_W     B00010100
#define EFUSE_SEL_R     B00001010
#define EFUSE_SEL_W     B00000110
#define LOCK_SEL_R      B00010000
#define SIG_SEL_R       B00001000
#define LSB_R           B00001000
#define MSB_R           B00011000
#define SEL_WR          B00000100
#define EE_PAGE_LATCH   B00001101
#define FL_PAGE_LATCH   B00011101
#define MASK_WE_OE      B00001100 // Make Write and Output enable strobes inactive
#define MASK_BS1        B00010000
#define MASK_BS2        B00000010
#define MASK_WE         B00001000
#define MASK_OE         B00000100
#define MASK_PAGEL      B00000001