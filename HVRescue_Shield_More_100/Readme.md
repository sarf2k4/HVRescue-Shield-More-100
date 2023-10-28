
/*

 Title:        HVRescue_Shield_More  (renamed from HVRescue_Shield_Plus)

 Description:  Arduino sketch for use with the HV Rescue Shield

 Based on:     HVRescue_Shield_Plus by Dennis Tricker

 Author:       Originally Jeff Keyzer, Modified and extended by Dennis Tricker and More version by Peter Boxler (Sept. 2016)

 Company:      MightyOhm Engineering

 Website:      http://mightyohm.com

 Contact:      http://mightyohm.com/blog/contact/

 This sketch assumes that the Arduino is equipped with an AVR HV Rescue Shield.
 Schematics and other details are available at http://mightyohm.com/hvrescue2

 The sketch uses High Voltage Programming Mode to set fuses on many Atmel AVR family 8-bit microcontrollers.
 Version 2.0 adds support for High Voltage Serial Programming (HVSP) mode and 8-pin ATtiny devices, but remains
 backwards compatible with the 1.x series hardware.

 The HVPP routines are based on those described in the ATmega48/88/168 datasheet rev.
 2545M-AVR-09/07, pg. 290-297 and the ATtiny2313 datasheet rev. 2543I-AVR-04/06 pg. 165-176.

 The HVSP routines are based on the ATtiny25/45/85 and 13A datasheets (ATtiny25/45/85 2586M–AVR–07/10 pg. 159-165,
 ATtiny13A 8126E-AVR-7/10 pg. 109-116).

 These routines are compatible with many other members of the AVR family that are not listed here.
 For a complete list of tested microcontrollers, see http://mightyohm.com/wiki/products:hvrescue:compatibility

Changelog:
**********
16/15/09  More 1.00 (based on Plus version) by Peter Boxler, Switzerland
- Tested with HVRescue Shield 2 on an Ardunio UNO R3 with Arduino IDE 1.6.9 (Mac) <---
- Added table (array) of AVR device definitions (incomplete, can easily be extended) and added more functions.
- Manual selection of device type removed, sketch tries to establish device type:
- finds either:
- invalid AVR signature found (not 0x1E) eg. no device present
- valid AVR signature found but no match to internal table
- valid AVR signature found and matches entry in internal table
- Added print AVR device type (after reading signature)
- Loop function streamlined for better readability
- Programming mode (HVSP/HVPP) is taken from internal device table
- Additionl selections in function menu:
- Function D write default fuses according to device found
- Function H write fuses for internal 8 Mhz clock (no division by 8)
- Function Q write fuses for external crystal 16 Mhz
- Note: these functions are available only if device was recognized based on the signature.
- removed non-interactive feature


 12/1/11 Plus 1.00 Rewritten to simplify device access and extend functionality, by Dennis J Tricker
- Added unified HVPP & SP low level code: target_xxxxxx functions & associated support routines (Load & Strobe)
- Introduced choice menu function and get_value with error checking and automation
- Fuse write now checked and indicated with OK or Error
- Extended chip data output to include SIG and LOCK bits
- Extended commands to allow Erase, FLASH & EEPROM Read
- Added FLASH & EEPROM test page write: writes and checks pattern to ensure chip operational
- Note: Address must be a start of page address ! See datasheet
- EEPROM test writes four bytes 0xaa,0x55,0xa5,0x5a
- FLASH Test writes "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPp" = 32 bytes= 16 words. Upper case is MSB.
- EEPROM&FLASH Test may not write a whole page due to limits on Arduino RAM memory, only check bytes written
- All output in hex, all input in hex: 0x prefix optional
- IMPORTANT: The Erase and Test functions will clear / overwrite existing device data - You have been warned !
- Tested with Arduino 22 & 1.0, ATtiny85/ATtiny2313/ATmega48

 3/15/11 2.12
 - New digital pin 0-7 (command&data) read/write routines for the Arduino Mega, since these lines are implemented
 differently on the Mega than on the original Arduino.

 3/8/11 2.11
 - Analog inputs (used here as digital outputs) are now called by their new Arduino A0-A5 names.

 2/2/11 2.1
 - adjusted RESET and VCC edge timing to work with new board design and avoid signal contention on SDO
 - fixed bug that prevented program from compiling in non-interactive mode
 - modified non-interactive mode so that read/verify serial comms still occur, but fuse values aren't prompted

 12/17/10 2.01
 - added missing braces to if(mode == HVSP) that sets SDO pinmode to INPUT
 - removed misleading comment about removing AVR when entering fuse values
 - default mode changed back to ATMEGA

 12/13/10 v2.0
 - Added support for 8-pin parts that use HV Serial Programming (HVSP)
 - New mode selection at startup determines which type of part is to be programmed
 - Got rid of endSerial function, since Arduino now includes Serial.end (finally!)
 - Added a wait for serial transmit to complete before burning fuses.  Without this HFUSE burn would fail occasionally.
 - Numerous other minor tweaks, removal of unnecessary delays, better commenting

 9/24/10 v1.2a
 - ATtiny2313 mode was being set by default.  Changed default mode back to ATmega (see #define ATtiny).

 8/16/10 v1.2
 - Existing fuse settings are now shown before asking the user for new values
 - Added OE strobe after entering programming mode to get ATtiny2313 to read first fuse correctly.
 - Cleaned up code a bit
 - Some minor tweaks to data direction register settings during setup, etc.

 11/02/09 v1.1
 - Removed endSerial call after reading back fuse bytes, was spewing garbage into
 serial monitor
 - Still occsionally get garbage when opening serial monitor, not sure what is causing this.

 03/01/09 v1.0
 - ATtiny2313 support, enable with ATtiny option
 - 12V Step up converter enable is non-inverting, unlike previous level shifter circuit
 - added interactive mode, asks for fuse values to burn, option to turn off
 - added EFUSE support and option to enable
 - button now has very simple debounce routine

 09/24/08
 - original release of sketch "HVFuse" to support first implementation on perfboard
 - Details: http://mightyohm.com/blog/2008/09/arduino-based-avr-high-voltage-programmer/

 Original Copyright 2008, 2009, 2010 Jeff Keyzer
 Additions Copyright 2011 Dennis Tricker

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
