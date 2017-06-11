

/**
 * Von hier: https://playground.arduino.cc/Code/EEPROMWriteAnything
 */
// -------------------------------

#include <EEPROM.h>
#include <Arduino.h>  // for type definitions
#include "bm_estorage.h"
#include "bm_global.h"

// ------------ EEPROM_writeAnything / EEPROM_readAnything-------------

/**
 * You provide the first EEPROM address to be written, 
 * and the functions return how many bytes were transferred. 
 *
 */
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}



// ------------------------
EStorage:: EStorage()
{

}
/*
void EStorage:: storeRast(byte rastNum, brewType bt){
}
void EStorage:: readRast (byte rastNum, brewType& bt) {
}
*/


void EStorage::storeGlobalRasten()
{
	byte z = START_RAST;
	//byte d = 0;
	for (byte i=0; i < MAX_RASTEN1; i++)
	{
		EEPROM_writeAnything(z++, rasten[i].temp);
		EEPROM_writeAnything(z++, rasten[i].time);
		EEPROM_writeAnything(z++, rasten[i].raise);
		EEPROM_writeAnything(z++, rasten[i].wait4User);
	}
	EEPROM_writeAnything(z++, SIMULATION);
	
	Serial.print (F("EEPROM written to byte position: "));
	Serial.println (z);
	
}


void EStorage::readGlobalRasten ()
{
	byte z = START_RAST;
	for (byte i=0; i < MAX_RASTEN1; i++)
	{
		EEPROM_readAnything(z++, rasten[i].temp);
		EEPROM_readAnything(z++, rasten[i].time);
		EEPROM_readAnything(z++, rasten[i].raise);
		EEPROM_readAnything(z++, rasten[i].wait4User);
	}
	EEPROM_readAnything(z++, SIMULATION);
}



